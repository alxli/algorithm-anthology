/*

Perform operations on arbitrary precision big integers internally represented as a vector of
base-`BASE` digits in little-endian order. `BASE` defaults to $10^9$ and must be a power of 10 no
larger than $10^9$. Multiplication selects schoolbook, Karatsuba, or FFT from the operand size;
`MUL_CUTOFF` is the padded transform length where FFT takes over and normally needs no adjustment.
Division likewise splits into a quadratic loop for small divisors and a recursive one past
`DIV_CUTOFF` limbs, so it inherits whichever multiplication the size selects. Typical arithmetic
operations involving mixed numeric primitives and strings are supported through implicit
construction and hidden friend operators, as long as at least one operand is a `BigInt` at any given
level of evaluation.

Addition, subtraction, and comparisons are performed using the standard linear algorithms.
Multiplication first converts limbs from base `BASE` to the narrower base `MUL_BASE` so coefficient
products fit safely, then multiplies with schoolbook, Karatsuba, or (if beyond `MUL_CUTOFF`) complex
FFT convolution according to the operand size, converting the result back. Division and modulo are
computed together by normalized long division: scale the operands so the divisor's leading limb is
large, estimate each quotient limb from the top one or two limbs of the running remainder, correct
the estimate by adding the divisor back if necessary, then unscale the remainder. Beyond
`DIV_CUTOFF` limbs, that loop becomes the base case of Burnikel-Ziegler recursion, which halves both
operands and recovers each half of the quotient from a division of the same shape, paying one
multiplication per level rather than one estimate per limb.

- `BigInt()` constructs zero, and `BigInt(n)` constructs a big integer from an integer `n`.
- `BigInt(s)` constructs a big integer from a C string or an `std::string` `s`.
- `operator=` is defined to copy from another big integer or to assign from a 64-bit integer
  primitive.
- `size()` returns the number of digits in the base-10 representation.
- Operators `>>` and `<<` are defined to support stream-based input and output.
- `to_string()`, `to_llong()`, `to_double()`, and `to_ldouble()` return the big integer converted to
  an `std::string`, `int64_t`, `double`, and `long double` respectively. For the latter three data
  types, overflow behavior is based on that of inputting from `std::istream`. Equivalent conversions
  are also available through explicit casts to `int`, `long long`, `double`, and `long double`.
- `abs()` returns the absolute value.
- `comp(v)` returns $-1$, $0$, or $1$ depending on whether the big integer compares less than, equal
  to, or greater than `v`, respectively.
- Operators `<`, `>`, `<=`, `>=`, `==`, `!=`, `+`, `-`, `*`, `/`, `%`, `++`, `--`, `+=`, `-=`, `*=`,
  `/=`, and `%=` are defined analogous to those on integer primitives.
- `div(v)` returns a pair consisting of the quotient and remainder after dividing by `v`.
- `pow(n)` returns the big integer raised to the nonnegative power `n` using binary exponentiation.
- `mul_pow10(n)` returns the big integer multiplied by $10^{`n`}$ for nonnegative `n`, which costs a
  limb shift rather than a multiplication.
- `sqrt()` returns the integral part of the square root using a digit-by-digit algorithm in the
  internal base.
- `nth_root(n)` returns the integral part of the `n`-th root using Newton's method. Negative inputs
  require odd `n`.
- `rand(d)` returns a random, positive big integer with `d` digits.

Time Complexity:
- O(d) per call to the constructors, `size()`, `to_string()`, `to_llong()`, `to_double()`,
  `to_ldouble()`, `abs()`, `comp()`, `rand()`, and all comparison and arithmetic operators except
  multiplication, division, and modulo, where $d$ is the total number of digits in the arguments and
  result for each operation.
- O(d*log(d)*log(log(d))) per call to multiplication operations once operands reach `MUL_CUTOFF`,
  and O(d^1.585) below it.
- O(d*m) per call to division and modulo operations below `DIV_CUTOFF`, and O(M(m) log m) at or
  above it, where $d$ and $m$ are the number of digits in the dividend and divisor respectively, and
  $M(m)$ is the cost of one multiplication of two $m$-digit big integers.
- O(M(d) log n) per call to `pow()`, where $n$ is the exponent and $d$ is the maximum digit length
  reached during exponentiation.
- O(d + n) per call to `mul_pow10()`, where $n$ is the power of ten.
- O(d^2) per call to `sqrt()`.
- O(M(d)*log(d)*log(d*n)) per call to `nth_root()`, where $n$ is the root. Newton's iteration starts
  within a factor of ten of the answer and doubles the correct digits each step, so it converges in
  O(log d) steps, each costing one exponentiation and one division.

Space Complexity:
- O(d) for storage of the big integer.
- O(d) auxiliary for negation, addition, subtraction, multiplication, division, `abs()`, `sqrt()`,
  `pow()`, and `nth_root()`.
- O(1) auxiliary for all other operations.

*/

#include <algorithm>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <istream>
#include <iterator>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class BigInt {
  static const int BASE = 1000000000, BASE_DIGITS = 9;  // BASE must equal 10^BASE_DIGITS.
  // Multiplication rebases to half as many decimal digits per limb, so that coefficient products
  // stay exact both in int64_t and within the FFT's double precision.
  static const int MUL_BASE = 10000, MUL_BASE_DIGITS = 4;  // Keep consistent with BASE_DIGITS.
  static const int MUL_CUTOFF = 256;  // Padded MUL_BASE length past which FFT beats Karatsuba.
  static const int DIV_CUTOFF = 64;   // Divisor limb count past which recursion wins.
  static_assert(
      BASE >= 100 && BASE <= 1000000000,
      "BASE must be between 10^2 and 10^9; below that MUL_BASE_DIGITS would round down to zero"
  );
  static_assert(
      MUL_BASE_DIGITS == BASE_DIGITS / 2 && MUL_BASE <= 10000,
      "MUL_BASE must hold half of BASE's digits and stay exact under the FFT"
  );

  using vint = std::vector<int>;
  using vint64 = std::vector<int64_t>;
  using vcd = std::vector<std::complex<double>>;

  vint digits;
  int sign;

  void normalize() {
    while (!digits.empty() && digits.back() == 0) {
      digits.pop_back();
    }
    if (digits.empty()) {
      sign = 1;
    }
  }

  void read(int n, const char *s) {
    sign = 1;
    digits.clear();
    int pos = 0;
    while (pos < n && (s[pos] == '-' || s[pos] == '+')) {
      if (s[pos] == '-') {
        sign = -sign;
      }
      pos++;
    }
    for (int i = n - 1; i >= pos; i -= BASE_DIGITS) {
      int x = 0;
      for (int j = std::max(pos, i - BASE_DIGITS + 1); j <= i; j++) {
        x = x * 10 + s[j] - '0';
      }
      digits.push_back(x);
    }
    normalize();
  }

  static int comp(const vint &a, const vint &b, int asign, int bsign) {
    if (asign != bsign) {
      return asign < bsign ? -1 : 1;
    }
    if (a.size() != b.size()) {
      return a.size() < b.size() ? -asign : asign;
    }
    for (int i = static_cast<int>(a.size()) - 1; i >= 0; i--) {
      if (a[i] != b[i]) {
        return a[i] < b[i] ? -asign : asign;
      }
    }
    return 0;
  }

  static BigInt add(const vint &a, const vint &b, int asign, int bsign) {
    if (asign != bsign) {
      return (asign == 1) ? sub(a, b, asign, 1) : sub(b, a, bsign, 1);
    }
    BigInt res;
    res.digits = a;
    res.sign = asign;
    int carry = 0, size = static_cast<int>(std::max(a.size(), b.size()));
    for (int i = 0; i < size || carry; i++) {
      if (i == static_cast<int>(res.digits.size())) {
        res.digits.push_back(0);
      }
      res.digits[i] += carry + (i < static_cast<int>(b.size()) ? b[i] : 0);
      carry = (res.digits[i] >= BASE) ? 1 : 0;
      if (carry) {
        res.digits[i] -= BASE;
      }
    }
    return res;
  }

  static BigInt sub(const vint &a, const vint &b, int asign, int bsign) {
    if (asign == -1 || bsign == -1) {
      return add(a, b, asign, -bsign);
    }
    BigInt res;
    if (comp(a, b, asign, bsign) < 0) {
      res = sub(b, a, bsign, asign);
      res.sign = -1;
      return res;
    }
    res.digits = a;
    res.sign = asign;
    for (int i = 0, borrow = 0; i < static_cast<int>(a.size()) || borrow; i++) {
      res.digits[i] -= borrow + (i < static_cast<int>(b.size()) ? b[i] : 0);
      borrow = res.digits[i] < 0;
      if (borrow) {
        res.digits[i] += BASE;
      }
    }
    res.normalize();
    return res;
  }

  static vint convert_base(const vint &digits, int l1, int l2) {
    vint64 p(std::max(l1, l2) + 1);
    p[0] = 1;
    for (int i = 1; i < static_cast<int>(p.size()); i++) {
      p[i] = p[i - 1] * 10;
    }
    vint res;
    int64_t curr = 0;
    for (int i = 0, curr_digits = 0; i < static_cast<int>(digits.size()); i++) {
      curr += digits[i] * p[curr_digits];
      curr_digits += l1;
      while (curr_digits >= l2) {
        res.push_back(static_cast<int>((curr % p[l2])));
        curr /= p[l2];
        curr_digits -= l2;
      }
    }
    res.push_back(static_cast<int>(curr));
    while (!res.empty() && res.back() == 0) {
      res.pop_back();
    }
    return res;
  }

  template<typename It>
  static vint64 karatsuba(It alo, It ahi, It blo, It bhi) {
    int n = std::distance(alo, ahi), k = n / 2;
    vint64 res(n * 2);
    if (n <= 32) {
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          res[i + j] += alo[i] * blo[j];
        }
      }
      return res;
    }
    auto a1b1 = karatsuba(alo, alo + k, blo, blo + k);
    auto a2b2 = karatsuba(alo + k, ahi, blo + k, bhi);
    vint64 a2(alo + k, ahi), b2(blo + k, bhi);
    for (int i = 0; i < k; i++) {
      a2[i] += alo[i];
      b2[i] += blo[i];
    }
    auto r = karatsuba(a2.begin(), a2.end(), b2.begin(), b2.end());
    for (int i = 0; i < static_cast<int>(a1b1.size()); i++) {
      r[i] -= a1b1[i];
      res[i] += a1b1[i];
    }
    for (int i = 0; i < static_cast<int>(a2b2.size()); i++) {
      r[i] -= a2b2[i];
      res[i + n] += a2b2[i];
    }
    for (int i = 0; i < static_cast<int>(r.size()); i++) {
      res[i + k] += r[i];
    }
    return res;
  }

  template<typename It>
  static vcd fft(It lo, It hi, bool invert = false) {
    int n = std::distance(lo, hi), k = 0, high1 = -1;
    while ((1 << k) < n) {
      k++;
    }
    std::vector<int> rev(n);
    for (int i = 1; i < n; i++) {
      if (!(i & (i - 1))) {
        high1++;
      }
      rev[i] = rev[i ^ (1 << high1)];
      rev[i] |= (1 << (k - high1 - 1));
    }
    vcd roots(n), res(n);
    for (int i = 0; i < n; i++) {
      double alpha = 2 * 3.14159265358979323846 * i / n;
      roots[i] = std::complex<double>(std::cos(alpha), std::sin(alpha));
      res[i] = *(lo + rev[i]);
    }
    for (int len = 1; len < n; len <<= 1) {
      vcd tmp(n);
      int rstep = static_cast<int>(roots.size()) / (len << 1);
      for (int pdest = 0; pdest < n; pdest += len) {
        int p = pdest;
        for (int i = 0; i < len; i++) {
          std::complex<double> c = roots[i * rstep] * res[p + len];
          tmp[pdest] = res[p] + c;
          tmp[pdest + len] = res[p] - c;
          pdest++;
          p++;
        }
      }
      res.swap(tmp);
    }
    if (invert) {
      for (int i = 0; i < static_cast<int>(res.size()); i++) {
        res[i] /= n;
      }
      std::reverse(res.begin() + 1, res.end());
    }
    return res;
  }

  // Schoolbook long division of nonnegative values, quadratic in the limb counts.
  std::pair<BigInt, BigInt> div_schoolbook(const BigInt &v) const {
    int norm = BASE / (v.digits.back() + 1);
    BigInt an = *this * norm, bn = v * norm, q, r;
    q.digits.resize(an.digits.size());
    for (int i = static_cast<int>(an.digits.size()) - 1; i >= 0; i--) {
      r *= BASE;
      r += an.digits[i];
      int s1 = (r.digits.size() <= bn.digits.size()) ? 0 : r.digits[bn.digits.size()];
      int s2 = (r.digits.size() <= bn.digits.size() - 1) ? 0 : r.digits[bn.digits.size() - 1];
      int d = (static_cast<int64_t>(s1) * BASE + s2) / bn.digits.back();
      for (r -= bn * d; r < 0; r += bn) {
        d--;
      }
      q.digits[i] = d;
    }
    q.normalize();
    r.normalize();
    return {q, r / norm};
  }

  // Burnikel-Ziegler recursive division of nonnegative values. Balanced halves replace the
  // quadratic inner loop, paying one multiplication per level instead of one estimate per limb, so
  // division inherits whichever multiplication the operand size selects.
  std::pair<BigInt, BigInt> div_recursive(const BigInt &v) const {
    auto shifted = [](const BigInt &x, int k) {  // Multiplies by BASE^k.
      BigInt res(x);
      if (!res.digits.empty()) {
        res.digits.insert(res.digits.begin(), k, 0);
      }
      return res;
    };
    auto slice = [](const BigInt &x, int lo, int hi) {  // Limbs [lo, hi), or 0 if empty range.
      BigInt res;
      lo = std::max(lo, 0);
      hi = std::min(hi, static_cast<int>(x.digits.size()));
      if (lo < hi) {
        res.digits.assign(x.digits.begin() + lo, x.digits.begin() + hi);
      }
      res.normalize();
      return res;
    };
    // Divides at most 2n limbs by exactly n limbs whose leading limb is at least BASE / 2, where
    // the quotient is known to be below BASE^n. Each half of the quotient comes from one
    // three-by-two block division, which itself divides two half-size blocks by one.
    auto rec = [&](auto &&rec, const BigInt &a, const BigInt &b,
                   int n) -> std::pair<BigInt, BigInt> {
      if (n % 2 != 0 || n < DIV_CUTOFF) {
        return a.div_schoolbook(b);
      }
      int k = n / 2;
      BigInt hi = slice(b, k, 2 * k), lo = slice(b, 0, k);
      auto block = [&](const BigInt &x) {  // At most 3k limbs by b, with quotient below BASE^k.
        BigInt q, r;
        if (slice(x, 2 * k, 3 * k).comp(hi) < 0) {
          std::tie(q, r) = rec(rec, slice(x, k, 3 * k), hi, k);
        } else {
          // The estimate saturates: the leading limbs already reach the divisor, so take the
          // largest quotient the block can hold and recover the matching remainder.
          q = shifted(BigInt(1), k) - 1;
          r = slice(x, k, 3 * k) - hi * q;
        }
        r = shifted(r, k) + slice(x, 0, k) - q * lo;
        while (r < 0) {  // At most twice, since the divisor's leading limb is at least BASE / 2.
          r += b;
          q -= 1;
        }
        return std::make_pair(q, r);
      };
      auto top = block(slice(a, k, 4 * k));
      auto bottom = block(shifted(top.second, k) + slice(a, 0, k));
      return {shifted(top.first, k) + bottom.first, bottom.second};
    };
    // Scale so the divisor's leading limb exceeds BASE / 2, then pad it to a multiple of a power
    // of two so that every recursive split stays even until the blocks fall under DIV_CUTOFF, and
    // consume the dividend one block at a time.
    int scale = BASE / (v.digits.back() + 1);
    BigInt a = *this * scale, b = v * scale;
    int limbs = static_cast<int>(b.digits.size()), block = 1;
    while (block * DIV_CUTOFF < limbs) {
      block *= 2;
    }
    int n = (limbs + block - 1) / block * block, sigma = n - limbs;
    a = shifted(a, sigma);
    b = shifted(b, sigma);
    BigInt q, r;
    for (int i = (static_cast<int>(a.digits.size()) + n - 1) / n - 1; i >= 0; i--) {
      auto step = rec(rec, shifted(r, n) + slice(a, i * n, (i + 1) * n), b, n);
      q = shifted(q, n) + step.first;
      r = step.second;
    }
    // Both scalings scale the remainder and leave the quotient alone.
    return {q, slice(r, sigma, static_cast<int>(r.digits.size())) / scale};
  }

 public:
  BigInt() : sign(1) {}
  BigInt(int v) { *this = static_cast<int64_t>(v); }
  BigInt(int64_t v) { *this = v; }
  BigInt(const char *s) { read(strlen(s), s); }
  BigInt(const std::string &s) { read(s.size(), s.c_str()); }

  BigInt &operator=(int64_t v) {
    sign = v < 0 ? -1 : 1;
    digits.clear();
    for (; v != 0; v /= BASE) {
      int limb = static_cast<int>(v % BASE);
      digits.push_back(limb < 0 ? -limb : limb);
    }
    return *this;
  }

  int size() const {
    return digits.empty() ? 1
                          : static_cast<int>(std::to_string(digits.back()).size()) +
                                BASE_DIGITS * (static_cast<int>(digits.size()) - 1);
  }

  friend std::istream &operator>>(std::istream &in, BigInt &v) {
    std::string s;
    in >> s;
    v.read(s.size(), s.c_str());
    return in;
  }

  friend std::ostream &operator<<(std::ostream &out, const BigInt &v) {
    return out << v.to_string();
  }

  std::string to_string() const {
    std::ostringstream oss;
    if (sign == -1) {
      oss << '-';
    }
    oss << (digits.empty() ? 0 : digits.back());
    for (int i = static_cast<int>(digits.size()) - 2; i >= 0; i--) {
      oss << std::setw(BASE_DIGITS) << std::setfill('0') << digits[i];
    }
    return oss.str();
  }

  template<typename T>
  T to_arithmetic() const {
    std::stringstream ss(to_string());
    T res;
    ss >> res;
    return res;
  }

  int64_t to_llong() const { return to_arithmetic<int64_t>(); }
  double to_double() const { return to_arithmetic<double>(); }
  long double to_ldouble() const { return to_arithmetic<long double>(); }

  explicit operator int() const { return static_cast<int>(to_llong()); }
  explicit operator long long() const { return static_cast<long long>(to_llong()); }
  explicit operator double() const { return to_double(); }
  explicit operator long double() const { return to_ldouble(); }

  int comp(const BigInt &v) const { return comp(digits, v.digits, sign, v.sign); }

  // The comparison and binary arithmetic operators are hidden friends, so a raw integer operand on
  // either side converts through the implicit constructor.
  friend bool operator<(const BigInt &a, const BigInt &b) { return a.comp(b) < 0; }
  friend bool operator>(const BigInt &a, const BigInt &b) { return a.comp(b) > 0; }
  friend bool operator<=(const BigInt &a, const BigInt &b) { return a.comp(b) <= 0; }
  friend bool operator>=(const BigInt &a, const BigInt &b) { return a.comp(b) >= 0; }
  friend bool operator==(const BigInt &a, const BigInt &b) { return a.comp(b) == 0; }
  friend bool operator!=(const BigInt &a, const BigInt &b) { return a.comp(b) != 0; }

  BigInt abs() const {
    BigInt res(*this);
    res.sign = 1;
    return res;
  }

  BigInt operator-() const {
    BigInt res(*this);
    if (!digits.empty()) res.sign = -sign;
    return res;
  }

  friend BigInt operator+(const BigInt &a, const BigInt &b) {
    return add(a.digits, b.digits, a.sign, b.sign);
  }

  friend BigInt operator-(const BigInt &a, const BigInt &b) {
    return sub(a.digits, b.digits, a.sign, b.sign);
  }

  BigInt &operator*=(int v) {
    int64_t factor = v;
    if (factor < 0) {
      sign = -sign;
      factor = -factor;
    }
    int64_t carry = 0;
    for (int i = 0; i < static_cast<int>(digits.size()) || carry; i++) {
      if (i == static_cast<int>(digits.size())) {
        digits.push_back(0);
      }
      int64_t curr = digits[i] * factor + carry;
      carry = curr / BASE;
      digits[i] = static_cast<int>((curr % BASE));
    }
    normalize();
    return *this;
  }

  BigInt operator*(int v) const {
    BigInt res(*this);
    res *= v;
    return res;
  }

  friend BigInt operator*(const BigInt &u, const BigInt &v) {
    vint a = convert_base(u.digits, BASE_DIGITS, MUL_BASE_DIGITS);
    vint b = convert_base(v.digits, BASE_DIGITS, MUL_BASE_DIGITS);
    int n = 1;
    while (n < 2 * static_cast<int>(std::max(a.size(), b.size()))) {
      n <<= 1;
    }
    a.resize(n, 0);
    b.resize(n, 0);
    vint64 c;
    if (n < MUL_CUTOFF) {
      c = karatsuba(a.begin(), a.end(), b.begin(), b.end());
    } else {
      auto at = fft(a.begin(), a.end()), bt = fft(b.begin(), b.end());
      for (int i = 0; i < n; i++) {
        at[i] *= bt[i];
      }
      at = fft(at.begin(), at.end(), true);
      c.resize(n);
      for (int i = 0; i < n; i++) {
        c[i] = at[i].real() + 0.5;
      }
    }
    BigInt res;
    res.sign = u.sign * v.sign;
    for (int i = 0, carry = 0; i < static_cast<int>(c.size()); i++) {
      int64_t d = c[i] + carry;
      res.digits.push_back(d % MUL_BASE);
      carry = d / MUL_BASE;
    }
    res.digits = convert_base(res.digits, MUL_BASE_DIGITS, BASE_DIGITS);
    res.normalize();
    return res;
  }

  BigInt &operator/=(int v) {
    assert(v != 0);
    int64_t divisor = v;
    if (divisor < 0) {
      sign = -sign;
      divisor = -divisor;
    }
    int64_t rem = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; i--) {
      int64_t curr = digits[i] + rem * static_cast<int64_t>(BASE);
      digits[i] = static_cast<int>((curr / divisor));
      rem = curr % divisor;
    }
    normalize();
    return *this;
  }

  BigInt operator/(int v) const {
    BigInt res(*this);
    res /= v;
    return res;
  }

  int operator%(int v) const {
    assert(v != 0);
    int64_t divisor = v;
    if (divisor < 0) {
      divisor = -divisor;
    }
    int64_t m = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; i--) {
      m = (digits[i] + m * static_cast<int64_t>(BASE)) % divisor;
    }
    return static_cast<int>(m * sign);
  }

  std::pair<BigInt, BigInt> div(const BigInt &v) const {
    assert(v != 0);
    if (comp(digits, v.digits, 1, 1) < 0) {
      return {BigInt(0), *this};
    }
    auto res = static_cast<int>(v.digits.size()) < DIV_CUTOFF ? abs().div_schoolbook(v.abs())
                                                              : abs().div_recursive(v.abs());
    res.first.sign = sign * v.sign;
    res.second.sign = sign;
    res.first.normalize();
    res.second.normalize();
    return res;
  }

  // clang-format off
  friend BigInt operator/(const BigInt &a, const BigInt &b) { return a.div(b).first; }
  friend BigInt operator%(const BigInt &a, const BigInt &b) { return a.div(b).second; }
  BigInt operator++(int){ BigInt t(*this); operator++(); return t; }
  BigInt operator--(int){ BigInt t(*this); operator--(); return t; }
  BigInt &operator++() { *this = *this + BigInt(1); return *this; }
  BigInt &operator--() { *this = *this - BigInt(1); return *this; }
  BigInt &operator+=(const BigInt &v) { *this = *this + v; return *this; }
  BigInt &operator-=(const BigInt &v) { *this = *this - v; return *this; }
  BigInt &operator*=(const BigInt &v) { *this = *this * v; return *this; }
  BigInt &operator/=(const BigInt &v) { *this = *this / v; return *this; }
  BigInt &operator%=(const BigInt &v) { *this = *this % v; return *this; }
  // clang-format on

  BigInt pow(int n) const {
    assert(n >= 0);
    if (n == 0) {
      return BigInt(1);
    }
    if (*this == 0) {
      return BigInt(0);
    }
    BigInt x(*this), res(1);
    for (; n != 0; n >>= 1) {
      if (n & 1) {
        res *= x;
      }
      x *= x;
    }
    return res;
  }

  // Multiplying by a power of ten is a limb shift, since BASE is a power of ten by construction.
  BigInt mul_pow10(int n) const {
    assert(n >= 0);
    static const int POW10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000};
    BigInt res(*this);
    res.digits.insert(res.digits.begin(), n / BASE_DIGITS, 0);
    res *= POW10[n % BASE_DIGITS];  // Also normalizes, which is what clears a shifted zero.
    return res;
  }

  BigInt sqrt() const {
    assert(sign != -1);
    BigInt v(*this);
    while (v.digits.empty() || v.digits.size() % 2 == 1) {
      v.digits.push_back(0);
    }
    int n = static_cast<int>(v.digits.size());
    int ldig =
        static_cast<int>(::sqrt(static_cast<double>(v.digits[n - 1]) * BASE + v.digits[n - 2]));
    int norm = BASE / (ldig + 1);
    v *= norm;
    v *= norm;
    while (v.digits.empty() || v.digits.size() % 2 == 1) {
      v.digits.push_back(0);
    }
    BigInt r(static_cast<int64_t>(v.digits[n - 1]) * BASE + v.digits[n - 2]);
    int q = ldig =
        static_cast<int>(::sqrt(static_cast<double>(v.digits[n - 1]) * BASE + v.digits[n - 2]));
    BigInt res;
    for (int j = n / 2 - 1; j >= 0; j--) {
      for (;; q--) {
        BigInt r1 =
            (r - (res * 2 * BASE + q) * q) * BASE * BASE +
            (j > 0 ? static_cast<int64_t>(v.digits[2 * j - 1]) * BASE + v.digits[2 * j - 2] : 0);
        if (r1 >= 0) {
          r = r1;
          break;
        }
      }
      res = res * BASE + q;
      if (j > 0) {
        int sz1 = static_cast<int>(res.digits.size());
        int sz2 = static_cast<int>(r.digits.size());
        int d1 = (sz1 + 2 < sz2) ? r.digits[sz1 + 2] : 0;
        int d2 = (sz1 + 1 < sz2) ? r.digits[sz1 + 1] : 0;
        int d3 = (sz1 < sz2) ? r.digits[sz1] : 0;
        q = (static_cast<int64_t>(d1) * BASE * BASE + static_cast<int64_t>(d2) * BASE + d3) /
            (ldig * 2);
      }
    }
    res.normalize();
    return res / norm;
  }

  BigInt nth_root(int n) const {
    assert(n > 0 && (sign != -1 || n % 2 == 1));
    if (*this == 0) {
      return BigInt(0);
    }
    // Newton's iteration, started just above the answer and decreasing to it.
    BigInt magnitude = abs();
    BigInt x = BigInt(10).pow(static_cast<int>(std::ceil(static_cast<double>(size()) / n))), y;
    while (x.comp(y = (x * (n - 1) + magnitude / x.pow(n - 1)) / n) > 0) {
      x = y;
    }
    return sign == -1 ? -x : x;
  }

  static BigInt rand(int d) {
    if (d == 0) {
      return BigInt(0);
    }
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> first_digit(1, 9), digit(0, 9);
    std::string s(1, static_cast<char>('0' + first_digit(rng)));
    for (int i = 1; i < d; i++) {
      s += static_cast<char>('0' + digit(rng));
    }
    return BigInt(s);
  }

  friend int comp(const BigInt &a, const BigInt &b) { return a.comp(b); }
  friend BigInt abs(const BigInt &v) { return v.abs(); }
  friend BigInt pow(const BigInt &v, int n) { return v.pow(n); }
  friend BigInt sqrt(const BigInt &v) { return v.sqrt(); }
  friend BigInt nth_root(const BigInt &v, int n) { return v.nth_root(n); }
};

/*** Example Usage ***/

using namespace std;

int main() {
  BigInt a("-9899819294989142124"), b("12398124981294214");
  assert(a + b == "-9887421170007847910");
  assert(a - b == "-9912217419970436338");
  assert(a * b == "-122739196911503356525379735104870536");
  assert(a / b == "-798");

  // Raw integers and strings work on either side of comparisons and arithmetic.
  assert("12398124981294214" == b);
  assert(5 + BigInt(7) == 12);
  assert(-100 < b && 5 % BigInt(3) == 2);

  assert(BigInt(20).pow(12345).size() == 16062);
  assert(BigInt("9812985918924981892491829").nth_root(4) == 1769906);
  assert(BigInt(-8).nth_root(3) == -2);
  for (int i = -100; i <= 100; i++) {
    if (i >= 0) {
      assert(BigInt(i).sqrt() == static_cast<int>(sqrt(i)));
    }
    for (int j = -100; j <= 100; j++) {
      assert(BigInt(i) + BigInt(j) == i + j);
      assert(BigInt(i) - BigInt(j) == i - j);
      assert(BigInt(i) * BigInt(j) == i * j);
      if (j != 0) {
        assert(BigInt(i) / BigInt(j) == i / j);
      }
      if (0 < i && i <= 10 && 0 < j && j <= 10) {
        assert(BigInt(i).nth_root(j) == static_cast<int64_t>((pow(i, 1.0 / j) + 1E-5)));
        int64_t p = 1;
        for (int k = 0; k < j; k++) {
          p *= i;
        }
        assert(BigInt(i).pow(j) == p);
      }
    }
  }
  mt19937 rng(1234567);  // Fixed seed for reproducibility.
  uniform_int_distribution<int> length_dist(1, 100);
  for (int i = 0; i < 20; i++) {
    int d = length_dist(rng);
    BigInt value(BigInt::rand(d)), root(value.sqrt()), lower(root * root), upper(root + 1);
    upper *= upper;
    assert(lower <= value && value < upper);
    uniform_int_distribution<int> divisor_length_dist(1, d);
    BigInt divisor(BigInt::rand(divisor_length_dist(rng)) + 1), quotient(value / divisor);
    lower = quotient * divisor;
    upper = divisor * (quotient + 1);
    assert(value >= lower && value < upper);
  }
  BigInt x(-6);
  assert(x.to_string() == "-6");
  assert(x.to_llong() == -6LL);
  assert(x.to_double() == -6.0);
  assert(x.to_ldouble() == -6.0);
  assert(static_cast<int>(x) == -6);
  assert(static_cast<long long>(x) == -6LL);
  assert(static_cast<double>(x) == -6.0);
  assert(BigInt(INT64_MIN).to_string() == "-9223372036854775808");
  assert(BigInt(INT64_MIN).to_llong() == INT64_MIN);
  assert(-BigInt(0) == 0);
  assert(BigInt(3) * INT_MIN == "-6442450944");
  assert(BigInt("6442450944") / INT_MIN == -3);
  assert(BigInt("6442450945") % INT_MIN == 1);
  return 0;
}
