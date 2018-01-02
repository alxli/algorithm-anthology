/*

Perform operations on arbitrary precision big integers internally represented as
a vector of base-1000000000 digits in little-endian order. Typical arithmetic
operations involving mixed numeric primitives and strings are supported using
templates and operator overloading, as long as at least one operand is a bigint
at any given level of evaluation.

- bigint(n) constructs a big integer from a long long (default = 0).
- bigint(s) constructs a big integer from a C string or an std::string s.
- operator = is defined to copy from another big integer or to assign from an
  64-bit integer primitive.
- size() returns the number of digits in the base-10 representation.
- operators >> and << are defined to support stream-based input and output.
- v.to_string(), v.to_llong(), v.to_double(), and v.to_ldouble() return the big
  integer v converted to an std::string, long long, double, and long double
  respectively. For the latter three data types, overflow behavior is based on
  that of inputting from std::istream.
- v.abs() returns the absolute value of big integer v.
- a.comp(b) returns -1, 0, or 1 depending on whether the big integers a and b
  compare less, equal, or greater, respectively.
- operators <, >, <=, >=, ==, !=, +, -, *, /, %, ++, --, +=, -=, *=, /=, and %=
  are defined analogous to those on integer primitives. Addition, subtraction,
  and comparisons are performed using the standard linear algorithms.
  Multiplication is performed using a combination of the grade school algorithm
  (for smaller inputs) and either the Karatsuba algorithm (if the USE_FFT_MULT
  flag is set to false) or the Schönhage–Strassen algorithm (if USE_FFT_MULT is
  set to true). Division and modulo are computed simultaneously using the grade
  school method.
- a.div(b) returns a pair consisting of the quotient and remainder.
- v.pow(n) returns v raised to the power of n.
- v.sqrt() returns the integral part of the square root of big integer v.
- v.nth_root(n) returns the integral part of the n-th root of big integer v.
- rand(n) returns a random, positive big integer with n digits.

Time Complexity:
- O(n) per call to the constructors, size(), to_string(), to_llong(),
  to_double(), to_ldouble(), abs(), comp(), rand(), and all comparison and
  arithmetic operators except multiplication, division, and modulo, where n is
  total number of digits in the argument(s) and result for each operation.
- O(n*log(n)*log(log(n))) or O(n^1.585) per call to multiplication operations,
  depending on whether USE_FFT_MULT is set to true or false.
- O(n*m) per call to division and modulo operations, where n and m are the
  number of digits in the dividend and divisor, respectively.
- O(M(m) log n) per call to pow(n), where m is the length of the big integer.

Space Complexity:
- O(n) for storage of the big integer.
- O(n) auxiliary heap space for negation, addition, subtraction, multiplication,
  division, abs(), sqrt(), pow(), and nth_root().
- O(1) auxiliary space for all other operations.

*/

#include <algorithm>
#include <cmath>
#include <complex>
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

class bigint {
  static const int BASE = 1000000000, BASE_DIGITS = 9;
  static const bool USE_FFT_MULT = true;

  typedef std::vector<int> vint;
  typedef std::vector<long long> vll;
  typedef std::vector<std::complex<double> > vcd;

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
        x = x*10 + s[j] - '0';
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
    for (int i = (int)a.size() - 1; i >= 0; i--) {
      if (a[i] != b[i]) {
        return a[i] < b[i] ? -asign : asign;
      }
    }
    return 0;
  }

  static bigint add(const vint &a, const vint &b, int asign, int bsign) {
    if (asign != bsign) {
      return (asign == 1) ? sub(a, b, asign, 1) : sub(b, a, bsign, 1);
    }
    bigint res;
    res.digits = a;
    res.sign = asign;
    int carry = 0, size = (int)std::max(a.size(), b.size());
    for (int i = 0; i < size || carry; i++) {
      if (i == (int)res.digits.size()) {
        res.digits.push_back(0);
      }
      res.digits[i] += carry + (i < (int)b.size() ? b[i] : 0);
      carry = (res.digits[i] >= BASE) ? 1 : 0;
      if (carry) {
        res.digits[i] -= BASE;
      }
    }
    return res;
  }

  static bigint sub(const vint &a, const vint &b, int asign, int bsign) {
    if (asign == -1 || bsign == -1) {
      return add(a, b, asign, -bsign);
    }
    bigint res;
    if (comp(a, b, asign, bsign) < 0) {
      res = sub(b, a, bsign, asign);
      res.sign = -1;
      return res;
    }
    res.digits = a;
    res.sign = asign;
    for (int i = 0, borrow = 0; i < (int)a.size() || borrow; i++) {
      res.digits[i] -= borrow + (i < (int)b.size() ? b[i] : 0);
      borrow = res.digits[i] < 0;
      if (borrow) {
        res.digits[i] += BASE;
      }
    }
    res.normalize();
    return res;
  }

  static vint convert_base(const vint &digits, int l1, int l2) {
    vll p(std::max(l1, l2) + 1);
    p[0] = 1;
    for (int i = 1; i < (int)p.size(); i++) {
      p[i] = p[i - 1]*10;
    }
    vint res;
    long long curr = 0;
    for (int i = 0, curr_digits = 0; i < (int)digits.size(); i++) {
      curr += digits[i]*p[curr_digits];
      curr_digits += l1;
      while (curr_digits >= l2) {
        res.push_back((int)(curr % p[l2]));
        curr /= p[l2];
        curr_digits -= l2;
      }
    }
    res.push_back((int)curr);
    while (!res.empty() && res.back() == 0) {
      res.pop_back();
    }
    return res;
  }

  template<class It>
  static vll karatsuba(It alo, It ahi, It blo, It bhi) {
    int n = std::distance(alo, ahi), k = n/2;
    vll res(n*2);
    if (n <= 32) {
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          res[i + j] += alo[i]*blo[j];
        }
      }
      return res;
    }
    vll a1b1 = karatsuba(alo, alo + k, blo, blo + k);
    vll a2b2 = karatsuba(alo + k, ahi, blo + k, bhi);
    vll a2(alo + k, ahi), b2(blo + k, bhi);
    for (int i = 0; i < k; i++) {
      a2[i] += alo[i];
      b2[i] += blo[i];
    }
    vll r = karatsuba(a2.begin(), a2.end(), b2.begin(), b2.end());
    for (int i = 0; i < (int)a1b1.size(); i++) {
      r[i] -= a1b1[i];
      res[i] += a1b1[i];
    }
    for (int i = 0; i < (int)a2b2.size(); i++) {
      r[i] -= a2b2[i];
      res[i + n] += a2b2[i];
    }
    for (int i = 0; i < (int)r.size(); i++) {
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
      double alpha = 2*3.14159265358979323846*i/n;
      roots[i] = std::complex<double>(cos(alpha), sin(alpha));
      res[i] = *(lo + rev[i]);
    }
    for (int len = 1; len < n; len <<= 1) {
      vcd tmp(n);
      int rstep = roots.size()/(len << 1);
      for (int pdest = 0; pdest < n; pdest += len) {
        int p = pdest;
        for (int i = 0; i < len; i++) {
          std::complex<double> c = roots[i*rstep]*res[p + len];
          tmp[pdest] = res[p] + c;
          tmp[pdest + len] = res[p] - c;
          pdest++;
          p++;
        }
      }
      res.swap(tmp);
    }
    if (invert) {
      for (int i = 0; i < (int)res.size(); i++) {
        res[i] /= n;
      }
      std::reverse(res.begin() + 1, res.end());
    }
    return res;
  }

 public:
  bigint() : sign(1) {}
  bigint(int v) { *this = (long long)v; }
  bigint(long long v) { *this = v; }
  bigint(const char *s) { read(strlen(s), s); }
  bigint(const std::string &s) { read(s.size(), s.c_str()); }

  void operator=(const bigint &v) {
    sign = v.sign;
    digits = v.digits;
  }

  void operator=(long long v) {
    sign = 1;
    if (v < 0) {
      sign = -1;
      v = -v;
    }
    digits.clear();
    for (; v > 0; v /= BASE) {
      digits.push_back(v % BASE);
    }
  }

  int size() const {
    if (digits.empty()) {
      return 1;
    }
    std::ostringstream oss;
    oss << digits.back();
    return oss.str().length() + BASE_DIGITS*(digits.size() - 1);
  }

  friend std::istream& operator>>(std::istream &in, bigint &v) {
    std::string s;
    in >> s;
    v.read(s.size(), s.c_str());
    return in;
  }

  friend std::ostream& operator<<(std::ostream &out, const bigint &v) {
    if (v.sign == -1) {
      out << '-';
    }
    out << (v.digits.empty() ? 0 : v.digits.back());
    for (int i = (int)v.digits.size() - 2; i >= 0; i--) {
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
    for (int i = (int)digits.size() - 2; i >= 0; i--) {
      oss << std::setw(BASE_DIGITS) << std::setfill('0') << digits[i];
    }
    return oss.str();
  }

  long long to_llong() const {
    long long res = 0;
    for (int i = (int)digits.size() - 1; i >= 0; i--) {
      res = res*BASE + digits[i];
    }
    return res*sign;
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

  int comp(const bigint &v) const {
    return comp(digits, v.digits, sign, v.sign);
  }

  bool operator<(const bigint &v) const { return comp(v) < 0; }
  bool operator>(const bigint &v) const { return comp(v) > 0; }
  bool operator<=(const bigint &v) const { return comp(v) <= 0; }
  bool operator>=(const bigint &v) const { return comp(v) >= 0; }
  bool operator==(const bigint &v) const { return comp(v) == 0; }
  bool operator!=(const bigint &v) const { return comp(v) != 0; }

  template<class T>
  friend bool operator<(const T &a, const bigint &b) { return bigint(a) < b; }

  template<class T>
  friend bool operator>(const T &a, const bigint &b) { return bigint(a) > b; }

  template<class T>
  friend bool operator<=(const T &a, const bigint &b) { return bigint(a) <= b; }

  template<class T>
  friend bool operator>=(const T &a, const bigint &b) { return bigint(a) >= b; }

  template<class T>
  friend bool operator==(const T &a, const bigint &b) { return bigint(a) == b; }

  template<class T>
  friend bool operator!=(const T &a, const bigint &b) { return bigint(a) != b; }

  bigint abs() const {
    bigint res(*this);
    res.sign = 1;
    return res;
  }

  bigint operator-() const {
    bigint res(*this);
    res.sign = -sign;
    return res;
  }

  bigint operator+(const bigint &v) const {
    return add(digits, v.digits, sign, v.sign);
  }

  bigint operator-(const bigint &v) const {
    return sub(digits, v.digits, sign, v.sign);
  }

  void operator*=(int v) {
    if (v < 0) {
      sign = -sign;
      v = -v;
    }
    for (int i = 0, carry = 0; i < (int)digits.size() || carry; i++) {
      if (i == (int)digits.size()) {
        digits.push_back(0);
      }
      long long curr = digits[i]*(long long)v + carry;
      carry = (int)(curr/BASE);
      digits[i] = (int)(curr % BASE);
    }
    normalize();
  }

  bigint operator*(int v) const {
    bigint res(*this);
    res *= v;
    return res;
  }

  bigint operator*(const bigint &v) const {
    static const int TEMP_BASE = 10000, TEMP_BASE_DIGITS = 4;
    vint a = convert_base(digits, BASE_DIGITS, TEMP_BASE_DIGITS);
    vint b = convert_base(v.digits, BASE_DIGITS, TEMP_BASE_DIGITS);
    int n = 1 << (33 - __builtin_clz(std::max(a.size(), b.size()) - 1));
    a.resize(n, 0);
    b.resize(n, 0);
    vll c;
    if (USE_FFT_MULT) {
      vcd at = fft(a.begin(), a.end()), bt = fft(b.begin(), b.end());
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
    bigint res;
    res.sign = sign*v.sign;
    for (int i = 0, carry = 0; i < (int)c.size(); i++) {
      long long d = c[i] + carry;
      res.digits.push_back(d % TEMP_BASE);
      carry = d/TEMP_BASE;
    }
    res.digits = convert_base(res.digits, TEMP_BASE_DIGITS, BASE_DIGITS);
    res.normalize();
    return res;
  }

  bigint& operator/=(int v) {
    if (v == 0) {
      throw std::runtime_error("Division by zero in bigint.");
    }
    if (v < 0) {
      sign = -sign;
      v = -v;
    }
    for (int i = (int)digits.size() - 1, rem = 0; i >= 0; i--) {
      long long curr = digits[i] + rem*(long long)BASE;
      digits[i] = (int)(curr/v);
      rem = (int)(curr % v);
    }
    normalize();
    return *this;
  }

  bigint operator/(int v) const {
    bigint res(*this);
    res /= v;
    return res;
  }

  int operator%(int v) const {
    if (v == 0) {
      throw std::runtime_error("Division by zero in bigint.");
    }
    if (v < 0) {
      v = -v;
    }
    int m = 0;
    for (int i = (int)digits.size() - 1; i >= 0; i--) {
      m = (digits[i] + m*(long long)BASE) % v;
    }
    return m*sign;
  }

  std::pair<bigint, bigint> div(const bigint &v) const {
    if (v == 0) {
      throw std::runtime_error("Division by zero in bigint.");
    }
    if (comp(digits, v.digits, 1, 1) < 0) {
      return std::make_pair(0, *this);
    }
    int norm = BASE/(v.digits.back() + 1);
    bigint an = abs()*norm, bn = v.abs()*norm, q, r;
    q.digits.resize(an.digits.size());
    for (int i = (int)an.digits.size() - 1; i >= 0; i--) {
      r *= BASE;
      r += an.digits[i];
      int s1 = (r.digits.size() <= bn.digits.size())
                  ? 0 : r.digits[bn.digits.size()];
      int s2 = (r.digits.size() <= bn.digits.size() - 1)
                  ? 0 : r.digits[bn.digits.size() - 1];
      int d = ((long long)s1*BASE + s2)/bn.digits.back();
      for (r -= bn*d; r < 0; r += bn) {
        d--;
      }
      q.digits[i] = d;
    }
    q.sign = sign*v.sign;
    r.sign = sign;
    q.normalize();
    r.normalize();
    return std::make_pair(q, r/norm);
  }

  bigint operator/(const bigint &v) const { return div(v).first; }
  bigint operator%(const bigint &v) const { return div(v).second; }

  bigint operator++(int) { bigint t(*this); operator++(); return t; }
  bigint operator--(int) { bigint t(*this); operator--(); return t; }
  bigint& operator++() { *this = *this + bigint(1); return *this; }
  bigint& operator--() { *this = *this - bigint(1); return *this; }
  bigint& operator+=(const bigint &v) { *this = *this + v; return *this; }
  bigint& operator-=(const bigint &v) { *this = *this - v; return *this; }
  bigint& operator*=(const bigint &v) { *this = *this * v; return *this; }
  bigint& operator/=(const bigint &v) { *this = *this / v; return *this; }
  bigint& operator%=(const bigint &v) { *this = *this % v; return *this; }

  template<class T>
  friend bigint operator+(const T &a, const bigint &b) { return bigint(a) + b; }

  template<class T>
  friend bigint operator-(const T &a, const bigint &b) { return bigint(a) - b; }

  bigint pow(int n) const {
    if (n == 0) {
      return bigint(1);
    }
    if (*this == 0 || n < 0) {
      return bigint(0);
    }
    bigint x(*this), res(1);
    for (; n != 0; n >>= 1) {
      if (n & 1) {
        res *= x;
      }
      x *= x;
    }
    return res;
  }

  bigint sqrt() const {
    if (sign == -1) {
      throw std::runtime_error("Cannot take square root of a negative number.");
    }
    bigint v(*this);
    while (v.digits.empty() || v.digits.size() % 2 == 1) {
      v.digits.push_back(0);
    }
    int n = v.digits.size();
    int ldig = (int)::sqrt((double)v.digits[n - 1]*BASE + v.digits[n - 2]);
    int norm = BASE/(ldig + 1);
    v *= norm;
    v *= norm;
    while (v.digits.empty() || v.digits.size() % 2 == 1) {
      v.digits.push_back(0);
    }
    bigint r((long long)v.digits[n - 1]*BASE + v.digits[n - 2]);
    int q = ldig = (int)::sqrt((double)v.digits[n - 1]*BASE + v.digits[n - 2]);
    bigint res;
    for (int j = n/2 - 1; j >= 0; j--) {
      for (;; q--) {
        bigint r1 = (r - (res*2*BASE + q)*q)*BASE*BASE +
            (j > 0 ? (long long)v.digits[2*j - 1]*BASE + v.digits[2*j - 2] : 0);
        if (r1 >= 0) {
          r = r1;
          break;
        }
      }
      res = res*BASE + q;
      if (j > 0) {
        int sz1 = res.digits.size(), sz2 = r.digits.size();
        int d1 = (sz1 + 2 < sz2) ? r.digits[sz1 + 2] : 0;
        int d2 = (sz1 + 1 < sz2) ? r.digits[sz1 + 1] : 0;
        int d3 = (sz1 < sz2) ? r.digits[sz1] : 0;
        q = ((long long)d1*BASE*BASE + (long long)d2*BASE + d3)/(ldig*2);
      }
    }
    res.normalize();
    return res/norm;
  }

  bigint nth_root(int n) const {
    if (sign == -1 && n % 2 == 0) {
      throw std::runtime_error("Cannot take even root of a negative number.");
    }
    if (*this == 0 || n < 0) {
      return bigint(0);
    }
    if (n >= size()) {
      int p = 1;
      while (comp(bigint(p).pow(n)) > 0) {
        p++;
      }
      return comp(bigint(p).pow(n)) < 0 ? p - 1 : p;
    }
    bigint lo(bigint(10).pow((int)ceil((double)size()/n) - 1)), hi(lo*10), mid;
    while (lo < hi) {
      mid = (lo + hi)/2;
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

  static bigint rand(int n) {
    if (n == 0) {
      return bigint(0);
    }
    std::string s(1, '1' + (::rand() % 9));
    for (int i = 1; i < n; i++) {
      s += '0' + (::rand() % 10);
    }
    return bigint(s);
  }

  friend int comp(const bigint &a, const bigint &b) { return a.comp(b); }
  friend bigint abs(const bigint &v) { return v.abs(); }
  friend bigint pow(const bigint &v, int n) { return v.pow(n); }
  friend bigint sqrt(const bigint &v) { return v.sqrt(); }
  friend bigint nth_root(const bigint &v, int n) { return v.nth_root(n); }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  bigint a("-9899819294989142124"), b("12398124981294214");
  assert(a + b == "-9887421170007847910");
  assert(a - b == "-9912217419970436338");
  assert(a * b == "-122739196911503356525379735104870536");
  assert(a / b == "-798");
  assert(bigint(20).pow(12345).size() == 16062);
  assert(bigint("9812985918924981892491829").nth_root(4) == 1769906);
  for (int i = -100; i <= 100; i++) {
    if (i >= 0) {
      assert(bigint(i).sqrt() == (int)sqrt(i));
    }
    for (int j = -100; j <= 100; j++) {
      assert(bigint(i) + bigint(j) == i + j);
      assert(bigint(i) - bigint(j) == i - j);
      assert(bigint(i) * bigint(j) == i * j);
      if (j != 0) {
        assert(bigint(i) / bigint(j) == i / j);
      }
      if (0 < i && i <= 10 && 0 < j && j <= 10) {
        assert(bigint(i).nth_root(j) == (long long)(pow(i, 1.0/j) + 1E-5));
        long long p = 1;
        for (int k = 0; k < j; k++) {
          p *= i;
        }
        assert(bigint(i).pow(j) == p);
      }
    }
  }
  for (int i = 0; i < 20; i++) {
    int n = rand() % 100 + 1;
    bigint a(bigint::rand(n)), s(a.sqrt()), xx(s*s), yy(s + 1);
    yy *= yy;
    assert(xx <= a && a < yy);
    bigint b(bigint::rand(rand() % n + 1) + 1), q(a/b);
    xx = q*b;
    yy = b*(q + 1);
    assert(a >= xx && a < yy);
  }
  bigint x(-6);
  assert(x.to_string() == "-6");
  assert(x.to_llong() == -6LL);
  assert(x.to_double() == -6.0);
  assert(x.to_ldouble() == -6.0);
  return 0;
}
