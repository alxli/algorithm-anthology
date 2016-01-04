/*

The following bigint class is implemented by storing "chunks"
of the big integer in a large base that is a power of 10 so
it can be efficiently stored, operated on, and printed.

It has extensive features including karatsuba multiplication,
exponentiation by squaring, and n-th root using binary search.
The class is thoroughly templatized, so you can use it as
easily as you do for normal ints. For example, you may use
operators with a bigint and a string (e.g. bigint(1234)+"-567"
and the result will be correctly promoted to a bigint that has
a value of 667). I/O is done using <iostream>. For example:
  bigint a, b; cin >> a >> b; cout << a + b << "\n";
adds two integers together and prints the result, just as you
would expect for a normal int, except with arbitrary precision.
The class also supports other streams such as fstream.

After the bigint class, a class for rational numbers is
implemented, using two bigints to store its numerators and
denominators. It is useful for when exact results of division
operations are needed.

*/

#include <algorithm> /* std::max(), std::swap() */
#include <cmath>     /* sqrt() */
#include <cstdlib>   /* rand() */
#include <iomanip>   /* std::setw(), std::setfill() */
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept> /* std::runtime_error() */
#include <string>
#include <utility>   /* std::pair */
#include <vector>

struct bigint {
  //base should be a power of 10 for I/O to work
  //base and base_digits should be consistent
  static const int base = 1000000000, base_digits = 9;

  typedef std::vector<int> vint;
  typedef std::vector<long long> vll;

  vint a; //a[0] stores right-most (least significant) base-digit
  int sign;

  bigint() : sign(1) {}
  bigint(int v) { *this = (long long)v; }
  bigint(long long v) { *this = v; }
  bigint(const std::string & s) { read(s); }
  bigint(const char * s) { read(std::string(s)); }

  void trim() {
    while (!a.empty() && a.back() == 0) a.pop_back();
    if (a.empty()) sign = 1;
  }

  void read(const std::string & s) {
    sign = 1;
    a.clear();
    int pos = 0;
    while (pos < (int)s.size() && (s[pos] == '-' || s[pos] == '+')) {
      if (s[pos] == '-') sign = -sign;
      pos++;
    }
    for (int i = s.size() - 1; i >= pos; i -= base_digits) {
      int x = 0;
      for (int j = std::max(pos, i - base_digits + 1); j <= i; j++)
        x = x * 10 + s[j] - '0';
      a.push_back(x);
    }
    trim();
  }

  void operator = (const bigint & v) {
    sign = v.sign;
    a = v.a;
  }

  void operator = (long long v) {
    sign = 1;
    if (v < 0) sign = -1, v = -v;
    a.clear();
    for (; v > 0; v /= base) a.push_back(v % base);
  }

  bigint operator + (const bigint & v) const {
    if (sign == v.sign) {
      bigint res = v;
      int carry = 0;
      for (int i = 0; i < (int)std::max(a.size(), v.a.size()) || carry; i++) {
        if (i == (int)res.a.size()) res.a.push_back(0);
        res.a[i] += carry + (i < (int)a.size() ? a[i] : 0);
        carry = res.a[i] >= base;
        if (carry) res.a[i] -= base;
      }
      return res;
    }
    return *this - (-v);
  }

  bigint operator - (const bigint & v) const {
    if (sign == v.sign) {
      if (abs() >= v.abs()) {
        bigint res(*this);
        for (int i = 0, carry = 0; i < (int)v.a.size() || carry; i++) {
          res.a[i] -= carry + (i < (int)v.a.size() ? v.a[i] : 0);
          carry = res.a[i] < 0;
          if (carry) res.a[i] += base;
        }
        res.trim();
        return res;
      }
      return -(v - *this);
    }
    return *this + (-v);
  }

  void operator *= (int v) {
    if (v < 0) sign = -sign, v = -v;
    for (int i = 0, carry = 0; i < (int)a.size() || carry; i++) {
      if (i == (int)a.size()) a.push_back(0);
      long long cur = a[i] * (long long)v + carry;
      carry = (int)(cur / base);
      a[i] = (int)(cur % base);
      //asm("divl %%ecx" : "=a"(carry), "=d"(a[i]) : "A"(cur), "c"(base));
    }
    trim();
  }

  bigint operator * (int v) const {
    bigint res(*this);
    res *= v;
    return res;
  }

  static vint convert_base(const vint & a, int l1, int l2) {
    vll p(std::max(l1, l2) + 1);
    p[0] = 1;
    for (int i = 1; i < (int)p.size(); i++) p[i] = p[i - 1] * 10;
    vint res;
    long long cur = 0;
    for (int i = 0, cur_digits = 0; i < (int)a.size(); i++) {
      cur += a[i] * p[cur_digits];
      cur_digits += l1;
      while (cur_digits >= l2) {
        res.push_back((int)(cur % p[l2]));
        cur /= p[l2];
        cur_digits -= l2;
      }
    }
    res.push_back((int)cur);
    while (!res.empty() && res.back() == 0) res.pop_back();
    return res;
  }

  //complexity: O(3N^log2(3)) ~ O(3N^1.585)
  static vll karatsuba_multiply(const vll & a, const vll & b) {
    int n = a.size();
    vll res(n + n);
    if (n <= 32) {
      for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
          res[i + j] += a[i] * b[j];
      return res;
    }
    int k = n >> 1;
    vll a1(a.begin(), a.begin() + k), a2(a.begin() + k, a.end());
    vll b1(b.begin(), b.begin() + k), b2(b.begin() + k, b.end());
    vll a1b1 = karatsuba_multiply(a1, b1);
    vll a2b2 = karatsuba_multiply(a2, b2);
    for (int i = 0; i < k; i++) a2[i] += a1[i];
    for (int i = 0; i < k; i++) b2[i] += b1[i];
    vll r = karatsuba_multiply(a2, b2);
    for (int i = 0; i < (int)a1b1.size(); i++) r[i] -= a1b1[i];
    for (int i = 0; i < (int)a2b2.size(); i++) r[i] -= a2b2[i];
    for (int i = 0; i < (int)r.size(); i++) res[i + k] += r[i];
    for (int i = 0; i < (int)a1b1.size(); i++) res[i] += a1b1[i];
    for (int i = 0; i < (int)a2b2.size(); i++) res[i + n] += a2b2[i];
    return res;
  }

  bigint operator * (const bigint & v) const {
    //if really big values cause overflow, use smaller _base
    static const int _base = 10000, _base_digits = 4;
    vint _a = convert_base(this->a, base_digits, _base_digits);
    vint _b = convert_base(v.a, base_digits, _base_digits);
    vll a(_a.begin(), _a.end());
    vll b(_b.begin(), _b.end());
    while (a.size() < b.size()) a.push_back(0);
    while (b.size() < a.size()) b.push_back(0);
    while (a.size() & (a.size() - 1)) {
      a.push_back(0);
      b.push_back(0);
    }
    vll c = karatsuba_multiply(a, b);
    bigint res;
    res.sign = sign * v.sign;
    for (int i = 0, carry = 0; i < (int)c.size(); i++) {
      long long cur = c[i] + carry;
      res.a.push_back((int)(cur % _base));
      carry = (int)(cur / _base);
    }
    res.a = convert_base(res.a, _base_digits, base_digits);
    res.trim();
    return res;
  }

  bigint operator ^ (const bigint & v) const {
    if (v.sign == -1) return bigint(0);
    bigint x(*this), n(v), res(1);
    while (!n.is_zero()) {
      if (n.a[0] % 2 == 1) res *= x;
      x *= x;
      n /= 2;
    }
    return res;
  }

  friend std::pair<bigint, bigint> divmod(const bigint & a1, const bigint & b1) {
    int norm = base / (b1.a.back() + 1);
    bigint a = a1.abs() * norm;
    bigint b = b1.abs() * norm;
    bigint q, r;
    q.a.resize(a.a.size());
    for (int i = a.a.size() - 1; i >= 0; i--) {
      r *= base;
      r += a.a[i];
      int s1 = r.a.size() <= b.a.size() ? 0 : r.a[b.a.size()];
      int s2 = r.a.size() <= b.a.size() - 1 ? 0 : r.a[b.a.size() - 1];
      int d = ((long long)base * s1 + s2) / b.a.back();
      for (r -= b * d; r < 0; r += b) d--;
      q.a[i] = d;
    }
    q.sign = a1.sign * b1.sign;
    r.sign = a1.sign;
    q.trim();
    r.trim();
    return std::make_pair(q, r / norm);
  }

  bigint operator / (const bigint & v) const { return divmod(*this, v).first; }
  bigint operator % (const bigint & v) const { return divmod(*this, v).second; }

  bigint & operator /= (int v) {
    if (v < 0) sign = -sign, v = -v;
    for (int i = a.size() - 1, rem = 0; i >= 0; i--) {
      long long cur = a[i] + rem * (long long)base;
      a[i] = (int)(cur / v);
      rem = (int)(cur % v);
    }
    trim();
    return *this;
  }

  bigint operator / (int v) const {
    bigint res(*this);
    res /= v;
    return res;
  }

  int operator % (int v) const {
    if (v < 0) v = -v;
    int m = 0;
    for (int i = a.size() - 1; i >= 0; i--)
      m = (a[i] + m * (long long)base) % v;
    return m * sign;
  }

  bigint operator ++(int) { bigint t(*this); operator++(); return t; }
  bigint operator --(int) { bigint t(*this); operator--(); return t; }
  bigint & operator ++() { *this = *this + bigint(1); return *this; }
  bigint & operator --() { *this = *this - bigint(1); return *this; }
  bigint & operator += (const bigint & v) { *this = *this + v; return *this; }
  bigint & operator -= (const bigint & v) { *this = *this - v; return *this; }
  bigint & operator *= (const bigint & v) { *this = *this * v; return *this; }
  bigint & operator /= (const bigint & v) { *this = *this / v; return *this; }
  bigint & operator %= (const bigint & v) { *this = *this % v; return *this; }
  bigint & operator ^= (const bigint & v) { *this = *this ^ v; return *this; }

  bool operator < (const bigint & v) const {
    if (sign != v.sign) return sign < v.sign;
    if (a.size() != v.a.size())
      return a.size() * sign < v.a.size() * v.sign;
    for (int i = a.size() - 1; i >= 0; i--)
      if (a[i] != v.a[i])
        return a[i] * sign < v.a[i] * sign;
    return false;
  }

  bool operator >  (const bigint & v) const { return v < *this; }
  bool operator <= (const bigint & v) const { return !(v < *this); }
  bool operator >= (const bigint & v) const { return !(*this < v); }
  bool operator == (const bigint & v) const { return !(*this < v) && !(v < *this); }
  bool operator != (const bigint & v) const { return *this < v || v < *this; }

  int size() const {
    if (a.empty()) return 1;
    std::ostringstream oss;
    oss << a.back();
    return oss.str().length() + base_digits*(a.size() - 1);
  }

  bool is_zero() const {
    return a.empty() || (a.size() == 1 && !a[0]);
  }

  bigint operator - () const {
    bigint res(*this);
    res.sign = -sign;
    return res;
  }

  bigint abs() const {
    bigint res(*this);
    res.sign *= res.sign;
    return res;
  }

  friend bigint abs(const bigint & a) {
    return a.abs();
  }

  friend bigint gcd(const bigint & a, const bigint & b) {
    return b.is_zero() ? a : gcd(b, a % b);
  }

  friend bigint lcm(const bigint & a, const bigint & b) {
    return a / gcd(a, b) * b;
  }

  friend bigint sqrt(const bigint & x) {
    bigint a = x;
    while (a.a.empty() || a.a.size() % 2 == 1) a.a.push_back(0);
    int n = a.a.size();
    int firstdig = sqrt((double)a.a[n - 1] * base + a.a[n - 2]);
    int norm = base / (firstdig + 1);
    a *= norm;
    a *= norm;
    while (a.a.empty() || a.a.size() % 2 == 1) a.a.push_back(0);
    bigint r = (long long)a.a[n - 1] * base + a.a[n - 2];
    firstdig = sqrt((double)a.a[n - 1] * base + a.a[n - 2]);
    int q = firstdig;
    bigint res;
    for (int j = n / 2 - 1; j >= 0; j--) {
      for (;; q--) {
        bigint r1 = (r - (res * 2 * base + q) * q) * base * base + (j > 0 ?
                      (long long)a.a[2 * j - 1] * base + a.a[2 * j - 2] : 0);
        if (r1 >= 0) {
          r = r1;
          break;
        }
      }
      res = (res * base) + q;
      if (j > 0) {
        int d1 = res.a.size() + 2 < r.a.size() ? r.a[res.a.size() + 2] : 0;
        int d2 = res.a.size() + 1 < r.a.size() ? r.a[res.a.size() + 1] : 0;
        int d3 = res.a.size() < r.a.size() ? r.a[res.a.size()] : 0;
        q = ((long long)d1*base*base + (long long)d2*base + d3)/(firstdig * 2);
      }
    }
    res.trim();
    return res / norm;
  }

  friend bigint nthroot(const bigint & x, const bigint & n) {
    bigint hi = 1;
    while ((hi ^ n) <= x) hi *= 2;
    bigint lo = hi / 2, mid, midn;
    while (lo < hi) {
      mid = (lo + hi) / 2;
      midn = mid ^ n;
      if (lo < mid && midn < x) {
        lo = mid;
      } else if (mid < hi && x < midn) {
        hi = mid;
      } else {
        return mid;
      }
    }
    return mid + 1;
  }

  friend std::istream & operator >> (std::istream & in, bigint & v) {
    std::string s;
    in >> s;
    v.read(s);
    return in;
  }

  friend std::ostream & operator << (std::ostream & out, const bigint & v) {
    if (v.sign == -1) out << '-';
    out << (v.a.empty() ? 0 : v.a.back());
    for (int i = v.a.size() - 2; i >= 0; i--)
      out << std::setw(base_digits) << std::setfill('0') << v.a[i];
    return out;
  }

  std::string to_string() const {
    std::ostringstream oss;
    if (sign == -1) oss << '-';
    oss << (a.empty() ? 0 : a.back());
    for (int i = a.size() - 2; i >= 0; i--)
      oss << std::setw(base_digits) << std::setfill('0') << a[i];
    return oss.str();
  }

  long long to_llong() const {
    long long res = 0;
    for (int i = a.size() - 1; i >= 0; i--)
      res = res * base + a[i];
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

  static bigint rand(int len) {
    if (len == 0) return bigint(0);
    std::string s(1, '1' + (::rand() % 9));
    for (int i = 1; i < len; i++) s += '0' + (::rand() % 10);
    return bigint(s);
  }
};

template<class T> bool operator > (const T & a, const bigint & b) { return bigint(a) > b; }
template<class T> bool operator < (const T & a, const bigint & b) { return bigint(a) < b; }
template<class T> bool operator >= (const T & a, const bigint & b) { return bigint(a) >= b; }
template<class T> bool operator <= (const T & a, const bigint & b) { return bigint(a) <= b; }
template<class T> bool operator == (const T & a, const bigint & b) { return bigint(a) == b; }
template<class T> bool operator != (const T & a, const bigint & b) { return bigint(a) != b; }
template<class T> bigint operator + (const T & a, const bigint & b) { return bigint(a) + b;  }
template<class T> bigint operator - (const T & a, const bigint & b) { return bigint(a) - b;  }
template<class T> bigint operator ^ (const T & a, const bigint & b) { return bigint(a) ^ b;  }

/*

Exclude *, /, and % to force a user decision between int and bigint algorithms

bigint operator * (bigint a, bigint b) vs. bigint operator * (bigint a, int b)
bigint operator / (bigint a, bigint b) vs. bigint operator / (bigint a, int b)
bigint operator % (bigint a, bigint b) vs. int operator % (bigint a, int b)

*/

struct rational {
  bigint num, den;

  rational(): num(0), den(1) {}
  rational(long long n): num(n), den(1) {}
  rational(const bigint & n) : num(n), den(1) {}

  template<class T1, class T2>
  rational(const T1 & n, const T2 & d): num(n), den(d) {
    if (den == 0)
      throw std::runtime_error("Rational division by zero.");
    if (den < 0) {
      num = -num;
      den = -den;
    }
    bigint a(num < 0 ? -num : num), b(den), tmp;
    while (a != 0 && b != 0) {
      tmp = a % b;
      a = b;
      b = tmp;
    }
    bigint gcd = (b == 0) ? a : b;
    num /= gcd;
    den /= gcd;
  }

  bool operator < (const rational & r) const {
    return num * r.den < r.num * den;
  }

  bool operator > (const rational & r) const {
    return r.num * den < num * r.den;
  }

  bool operator <= (const rational & r) const {
    return !(r < *this);
  }

  bool operator >= (const rational & r) const {
    return !(*this < r);
  }

  bool operator == (const rational & r) const {
    return num == r.num && den == r.den;
  }

  bool operator != (const rational & r) const {
    return num != r.num || den != r.den;
  }

  rational operator + (const rational & r) const {
    return rational(num * r.den + r.num * den, den * r.den);
  }

  rational operator - (const rational & r) const {
    return rational(num * r.den - r.num * den, r.den * den);
  }

  rational operator * (const rational & r) const {
    return rational(num * r.num, r.den * den);
  }

  rational operator / (const rational & r) const {
    return rational(num * r.den, den * r.num);
  }

  rational operator % (const rational & r) const {
    return *this - r * rational(num * r.den / (r.num * den), 1);
  }

  rational operator ^ (const bigint & p) const {
    return rational(num ^ p, den ^ p);
  }

  rational operator ++(int) { rational t(*this); operator++(); return t; }
  rational operator --(int) { rational t(*this); operator--(); return t; }
  rational & operator ++() { *this = *this + 1; return *this; }
  rational & operator --() { *this = *this - 1; return *this; }
  rational & operator += (const rational & r) { *this = *this + r; return *this; }
  rational & operator -= (const rational & r) { *this = *this - r; return *this; }
  rational & operator *= (const rational & r) { *this = *this * r; return *this; }
  rational & operator /= (const rational & r) { *this = *this / r; return *this; }
  rational & operator %= (const rational & r) { *this = *this % r; return *this; }
  rational & operator ^= (const bigint & r) { *this = *this ^ r; return *this; }

  rational operator - () const {
    return rational(-num, den);
  }

  rational abs() const {
    return rational(num.abs(), den);
  }

  long long to_llong() const {
    return num.to_llong() / den.to_llong();
  }

  double to_double() const {
    return num.to_double() / den.to_double();
  }

  friend rational abs(const rational & r) {
    return rational(r.num.abs(), r.den);
  }

  friend std::istream & operator >> (std::istream & in, rational & r) {
    std::string s;
    in >> r.num;
    r.den = 1;
    return in;
  }

  friend std::ostream & operator << (std::ostream & out, const rational & r) {
    out << r.num << "/" << r.den;
    return out;
  }

  //rational in range [0, 1] with precision no greater than prec
  static rational rand(int prec) {
    rational r(bigint::rand(prec), bigint::rand(prec));
    if (r.num > r.den) std::swap(r.num, r.den);
    return r;
  }
};

template<class T> bool operator > (const T & a, const rational & b) { return rational(a) > b; }
template<class T> bool operator < (const T & a, const rational & b) { return rational(a) < b; }
template<class T> bool operator >= (const T & a, const rational & b) { return rational(a) >= b; }
template<class T> bool operator <= (const T & a, const rational & b) { return rational(a) <= b; }
template<class T> bool operator == (const T & a, const rational & b) { return rational(a) == b; }
template<class T> bool operator != (const T & a, const rational & b) { return rational(a) != b; }
template<class T> rational operator + (const T & a, const rational & b) { return rational(a) + b;  }
template<class T> rational operator - (const T & a, const rational & b) { return rational(a) - b;  }
template<class T> rational operator * (const T & a, const rational & b) { return rational(a) * b;  }
template<class T> rational operator / (const T & a, const rational & b) { return rational(a) / b;  }
template<class T> rational operator % (const T & a, const rational & b) { return rational(a) % b;  }
template<class T> rational operator ^ (const T & a, const rational & b) { return rational(a) ^ b;  }

/*** Example Usage ***/

#include <cassert>
#include <cstdio>
#include <ctime>
#include <iostream>
using namespace std;

int main() {
  for (int i = 0; i < 20; i++) {
    int n = rand() % 100 + 1;
    bigint a = bigint::rand(n);
    bigint res = sqrt(a);
    bigint xx(res * res);
    bigint yy(res + 1);
    yy *= yy;
    assert(xx <= a && yy > a);
    int m = rand() % n + 1;
    bigint b = bigint::rand(m) + 1;
    res = a / b;
    xx = res * b;
    yy = b * (res + 1);
    assert(a >= xx && a < yy);
  }

  assert("995291497" ==
    nthroot(bigint("981298591892498189249182998429898124"), 4));

  bigint x(5);
  x = -6;
  assert(x.to_llong() == -6ll);
  assert(x.to_string() == "-6");

  clock_t start;

  start = clock();
  bigint c = bigint::rand(10000) / bigint::rand(2000);
  cout << "Div took " << (float)(clock() - start)/CLOCKS_PER_SEC << "s\n";

  start = clock();
  assert((20^bigint(12345)).size() == 16062);
  cout << "Pow took " << (float)(clock() - start)/CLOCKS_PER_SEC << "s\n";

  int nn = -21, dd = 2;
  rational n(nn, 1), d(dd);
  cout << (nn % dd) << "\n";
  cout << (n % d) << "\n";
  cout << fmod(-5.3, -1.7) << "\n";
  cout << rational(-53, 10) % rational(-17, 10) << "\n";
  cout << rational(-53, 10).abs() << "\n";
  cout << (rational(-53, 10) ^ 20) << "\n";
  cout << rational::rand(20) << "\n";
  return 0;
}
