/*

Perform operations on arbitrary precision big integers internally represented as a vector of
base-$10^9$ digits in little-endian order. Typical arithmetic operations involving mixed numeric
primitives and strings are supported through implicit construction and hidden friend operators, as
long as at least one operand is a `BigInt` at any given level of evaluation.

- `BigInt(n)` constructs a big integer from an integer `n` (default: 0).
- `BigInt(s)` constructs a big integer from a C string or an `std::string` `s`.
- `operator=` is defined to copy from another big integer or to assign from a 64-bit integer
  primitive.
- `size()` returns the number of digits in the base-10 representation.
- Operators `>>` and `<<` are defined to support stream-based input and output.
- `v.to_string()`, `v.to_llong()`, `v.to_double()`, and `v.to_ldouble()` return the big integer `v`
  converted to an `std::string`, `int64_t`, `double`, and `long double` respectively. For the
  latter three data types, overflow behavior is based on that of inputting from `std::istream`.
- `v.abs()` returns the absolute value of big integer `v`.
- `a.comp(b)` returns $-1$, $0$, or $1$ depending on whether the big integers `a` and `b` compare
  less, equal, or greater, respectively.
- Operators `<`, `>`, `<=`, `>=`, `==`, `!=`, `+`, `-`, `*`, `/`, `%`, `++`, `--`, `+=`, `-=`, `*=`,
  `/=`, and `%=` are defined analogous to those on integer primitives. Addition, subtraction, and
  comparisons are performed using the standard linear algorithms. Multiplication first converts
  limbs from base $10^9$ to base $10^4$ so coefficient products fit safely, then uses either
  Karatsuba multiplication (if `USE_FFT_MULT` is false) or complex FFT convolution (if
  `USE_FFT_MULT` is true), converting the result back to base $10^9$. Division and modulo are
  computed together by normalized long division: scale the operands so the divisor's leading limb is
  large, estimate each quotient limb from the top one or two limbs of the running remainder, correct
  the estimate by adding the divisor back if necessary, then unscale the remainder.
- `a.div(b)` returns a pair consisting of the quotient and remainder.
- `v.pow(n)` returns `v` raised to the power of $n$.
- `v.sqrt()` returns the integral part of the square root of big integer `v`.
- `v.nth_root(n)` returns the integral part of the $n$-th root of big integer `v`.
- `rand(n)` returns a random, positive big integer with $n$ digits.

`pow(n)` uses binary exponentiation. `sqrt()` uses a digit-by-digit square-root algorithm in the
internal base, while `nth_root(n)` uses binary search over the answer range.

Time Complexity:
- O(n) per call to the constructors, `size()`, `to_string()`, `to_llong()`, `to_double()`,
  `to_ldouble()`, `abs()`, `comp()`, `rand()`, and all comparison and arithmetic operators except
  multiplication, division, and modulo, where $n$ is total number of digits in the arguments and
  result for each operation.
- O(n*log(n)*log(log(n))) or O(n^1.585) per call to multiplication operations, depending on whether
  `USE_FFT_MULT` is set to true or false.
- O(n*m) per call to division and modulo operations, where $n$ and $m$ are the number of digits in
  the dividend and divisor, respectively.
- O(M(d) log n) per call to `pow(n)`, where $d$ is the maximum digit length reached during
  exponentiation and $M(d)$ denotes the cost of one multiplication of two $d$-digit big integers
  using the selected multiplication method above.

Space Complexity:
- O(n) for storage of the big integer.
- O(n) auxiliary heap space for negation, addition, subtraction, multiplication, division, `abs()`,
  `sqrt()`, `pow()`, and `nth_root()`.
- O(1) auxiliary space for all other operations.

*/

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class BigInt {
  static const int BASE = 1000000000, BASE_DIGITS = 9;
  static const bool USE_FFT_MULT = true;

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

  template<class It>
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

  template<class It>
  static vcd fft(It lo, It hi, bool invert = false) {
    int n = std::distance(lo, hi), k = 0, high1 = -1;
    while ((1 << k) < n) {
      k++;
    }
    std::vector<int> rev(n, 0);
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
      roots[i] = std::complex<double>(cos(alpha), sin(alpha));
      res[i] = *(lo + rev[i]);
    }
    for (int len = 1; len < n; len <<= 1) {
      vcd tmp(n);
      int rstep = roots.size() / (len << 1);
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

 public:
  BigInt() : sign(1) {}
  BigInt(int v) { *this = static_cast<int64_t>(v); }
  BigInt(int64_t v) { *this = v; }
  BigInt(const char *s) { read(strlen(s), s); }
  BigInt(const std::string &s) { read(s.size(), s.c_str()); }

  BigInt &operator=(const BigInt &v) {
    sign = v.sign;
    digits = v.digits;
    return *this;
  }

  BigInt &operator=(int64_t v) {
    sign = 1;
    if (v < 0) {
      sign = -1;
      v = -v;
    }
    digits.clear();
    for (; v > 0; v /= BASE) {
      digits.push_back(v % BASE);
    }
    return *this;
  }

  int size() const {
    if (digits.empty()) {
      return 1;
    }
    std::ostringstream oss;
    oss << digits.back();
    return oss.str().length() + BASE_DIGITS * (digits.size() - 1);
  }

  friend std::istream &operator>>(std::istream &in, BigInt &v) {
    std::string s;
    in >> s;
    v.read(s.size(), s.c_str());
    return in;
  }

  friend std::ostream &operator<<(std::ostream &out, const BigInt &v) {
    if (v.sign == -1) {
      out << '-';
    }
    out << (v.digits.empty() ? 0 : v.digits.back());
    for (int i = static_cast<int>(v.digits.size()) - 2; i >= 0; i--) {
      out << std::setw(BASE_DIGITS) << std::setfill('0') << v.digits[i];
    }
    return out;
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

  int64_t to_llong() const {
    int64_t res = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; i--) {
      res = res * BASE + digits[i];
    }
    return res * sign;
  }

  double to_double() const {
    std::stringstream ss(to_string());
    double res;
    ss >> res;
    return res;
  }

  long double to_ldouble() const {
    std::stringstream ss(to_string());
    long double res;
    ss >> res;
    return res;
  }

  int comp(const BigInt &v) const { return comp(digits, v.digits, sign, v.sign); }

  // The comparison and binary arithmetic operators are hidden friends, so a raw integer operand on
  // either side converts through the implicit constructor.
  friend bool operator<(const BigInt &a, const BigInt &b) { return a.comp(b) < 0; }
  friend bool operator>(const BigInt &a, const BigInt &b) { return a.comp(b) > 0; }
  friend bool operator<=(const BigInt &a, const BigInt &b) { return a.comp(b) <= 0; }
  friend bool operator>=(const BigInt &a, const BigInt &b) { return a.comp(b) >= 0; }
  friend bool operator==(const BigInt &a, const BigInt &b) { return a.comp(b) == 0; }
  friend bool operator!=(const BigInt &a, const BigInt &b) { return a.comp(b) != 0; }

  // clang-format off
  BigInt abs() const { BigInt res(*this); res.sign = 1; return res; }
  BigInt operator-() const { BigInt res(*this); res.sign = -sign; return res; }
  // clang-format on

  friend BigInt operator+(const BigInt &a, const BigInt &b) {
    return add(a.digits, b.digits, a.sign, b.sign);
  }

  friend BigInt operator-(const BigInt &a, const BigInt &b) {
    return sub(a.digits, b.digits, a.sign, b.sign);
  }

  void operator*=(int v) {
    if (v < 0) {
      sign = -sign;
      v = -v;
    }
    for (int i = 0, carry = 0; i < static_cast<int>(digits.size()) || carry; i++) {
      if (i == static_cast<int>(digits.size())) {
        digits.push_back(0);
      }
      int64_t curr = digits[i] * static_cast<int64_t>(v) + carry;
      carry = static_cast<int>((curr / BASE));
      digits[i] = static_cast<int>((curr % BASE));
    }
    normalize();
  }

  BigInt operator*(int v) const {
    BigInt res(*this);
    res *= v;
    return res;
  }

  friend BigInt operator*(const BigInt &u, const BigInt &v) {
    static const int TEMP_BASE = 10000, TEMP_BASE_DIGITS = 4;
    vint a = convert_base(u.digits, BASE_DIGITS, TEMP_BASE_DIGITS);
    vint b = convert_base(v.digits, BASE_DIGITS, TEMP_BASE_DIGITS);
    int n = 1;
    while (n < 2 * static_cast<int>(std::max(a.size(), b.size()))) {
      n <<= 1;
    }
    a.resize(n, 0);
    b.resize(n, 0);
    vint64 c;
    if (USE_FFT_MULT) {
      auto at = fft(a.begin(), a.end()), bt = fft(b.begin(), b.end());
      for (int i = 0; i < n; i++) {
        at[i] *= bt[i];
      }
      at = fft(at.begin(), at.end(), true);
      c.resize(n);
      for (int i = 0; i < n; i++) {
        c[i] = at[i].real() + 0.5;
      }
    } else {
      c = karatsuba(a.begin(), a.end(), b.begin(), b.end());
    }
    BigInt res;
    res.sign = u.sign * v.sign;
    for (int i = 0, carry = 0; i < static_cast<int>(c.size()); i++) {
      int64_t d = c[i] + carry;
      res.digits.push_back(d % TEMP_BASE);
      carry = d / TEMP_BASE;
    }
    res.digits = convert_base(res.digits, TEMP_BASE_DIGITS, BASE_DIGITS);
    res.normalize();
    return res;
  }

  BigInt &operator/=(int v) {
    if (v == 0) {
      throw std::runtime_error("Division by zero in BigInt.");
    }
    if (v < 0) {
      sign = -sign;
      v = -v;
    }
    for (int i = static_cast<int>(digits.size()) - 1, rem = 0; i >= 0; i--) {
      int64_t curr = digits[i] + rem * static_cast<int64_t>(BASE);
      digits[i] = static_cast<int>((curr / v));
      rem = static_cast<int>((curr % v));
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
    if (v == 0) {
      throw std::runtime_error("Division by zero in BigInt.");
    }
    if (v < 0) {
      v = -v;
    }
    int m = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; i--) {
      m = (digits[i] + m * static_cast<int64_t>(BASE)) % v;
    }
    return m * sign;
  }

  std::pair<BigInt, BigInt> div(const BigInt &v) const {
    if (v == 0) {
      throw std::runtime_error("Division by zero in BigInt.");
    }
    if (comp(digits, v.digits, 1, 1) < 0) {
      return {BigInt(0), *this};
    }
    int norm = BASE / (v.digits.back() + 1);
    BigInt an = abs() * norm, bn = v.abs() * norm, q, r;
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
    q.sign = sign * v.sign;
    r.sign = sign;
    q.normalize();
    r.normalize();
    return {q, r / norm};
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
    if (n == 0) {
      return BigInt(1);
    }
    if (*this == 0 || n < 0) {
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

  BigInt sqrt() const {
    if (sign == -1) {
      throw std::runtime_error("Cannot take square root of a negative number.");
    }
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
    if (sign == -1 && n % 2 == 0) {
      throw std::runtime_error("Cannot take even root of a negative number.");
    }
    if (*this == 0 || n < 0) {
      return BigInt(0);
    }
    if (n >= size()) {
      int p = 1;
      while (comp(BigInt(p).pow(n)) > 0) {
        p++;
      }
      return comp(BigInt(p).pow(n)) < 0 ? p - 1 : p;
    }
    BigInt lo(BigInt(10).pow(static_cast<int>(ceil(static_cast<double>(size()) / n)) - 1)),
        hi(lo * 10), mid;
    while (lo < hi) {
      mid = (lo + hi) / 2;
      int cmp = comp(digits, mid.pow(n).digits, 1, 1);
      if (lo < mid && cmp > 0) {
        lo = mid;
      } else if (mid < hi && cmp < 0) {
        hi = mid;
      } else {
        return (sign == -1) ? -mid : mid;
      }
    }
    return (sign == -1) ? -(mid + 1) : (mid + 1);
  }

  static BigInt rand(int n) {
    if (n == 0) {
      return BigInt(0);
    }
    std::string s(1, '1' + (::rand() % 9));
    for (int i = 1; i < n; i++) {
      s += '0' + (::rand() % 10);
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

#include <cassert>

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
  for (int i = 0; i < 20; i++) {
    int n = rand() % 100 + 1;
    BigInt a(BigInt::rand(n)), s(a.sqrt()), xx(s * s), yy(s + 1);
    yy *= yy;
    assert(xx <= a && a < yy);
    BigInt b(BigInt::rand(rand() % n + 1) + 1), q(a / b);
    xx = q * b;
    yy = b * (q + 1);
    assert(a >= xx && a < yy);
  }
  BigInt x(-6);
  assert(x.to_string() == "-6");
  assert(x.to_llong() == -6LL);
  assert(x.to_double() == -6.0);
  assert(x.to_ldouble() == -6.0);
  return 0;
}
