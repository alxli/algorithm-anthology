/* 4.4.2 - Big Integer Class */

#include <algorithm> /* std::max() */
#include <cmath>     /* sqrt() */
#include <cstdlib>   /* rand() */
#include <iomanip>   /* std::setw(), std::setfill() */
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>   /* std::pair */
#include <vector>

struct bigint {
  //base should be a power of 10 for I/O to work
  //base_digits should equal ceil(log10(base))
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
    while (pos < s.size() && (s[pos] == '-' || s[pos] == '+')) {
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
      for (int i = 0; i < std::max(a.size(), v.a.size()) || carry; i++) {
        if (i == res.a.size()) res.a.push_back(0);
        res.a[i] += carry + (i < a.size() ? a[i] : 0);
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
        for (int i = 0, carry = 0; i < v.a.size() || carry; i++) {
          res.a[i] -= carry + (i < v.a.size() ? v.a[i] : 0);
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
    for (int i = 0, carry = 0; i < a.size() || carry; i++) {
      if (i == a.size()) a.push_back(0);
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
    for (int i = 1; i < p.size(); i++) p[i] = p[i - 1] * 10;
    vint res;
    long long cur = 0;
    for (int i = 0, cur_digits = 0; i < a.size(); i++) {
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

  /* complexity: O(3N^log2(3)) ~ O(3N^1.585) */
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
    for (int i = 0; i < a1b1.size(); i++) r[i] -= a1b1[i];
    for (int i = 0; i < a2b2.size(); i++) r[i] -= a2b2[i];
    for (int i = 0; i < r.size(); i++) res[i + k] += r[i];
    for (int i = 0; i < a1b1.size(); i++) res[i] += a1b1[i];
    for (int i = 0; i < a2b2.size(); i++) res[i + n] += a2b2[i];
    return res;
  }

  bigint operator * (const bigint & v) const {
    vint a6 = convert_base(this->a, base_digits, 6);
    vint b6 = convert_base(v.a, base_digits, 6);
    vll a(a6.begin(), a6.end());
    vll b(b6.begin(), b6.end());
    while (a.size() < b.size()) a.push_back(0);
    while (b.size() < a.size()) b.push_back(0);
    while (a.size() & (a.size() - 1)) {
      a.push_back(0);
      b.push_back(0);
    }
    vll c = karatsuba_multiply(a, b);
    bigint res;
    res.sign = sign * v.sign;
    for (int i = 0, carry = 0; i < c.size(); i++) {
      long long cur = c[i] + carry;
      res.a.push_back((int)(cur % 1000000));
      carry = (int)(cur / 1000000);
    }
    res.a = convert_base(res.a, 6, base_digits);
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

  static bigint rand(int len) {
    std::string s(1, '1' + (::rand() % 9));
    for (int i = 1; i < len; i++) s += '0' + (::rand() % 10);
    return bigint(s);
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
    while ((hi ^ n) < x) hi *= 2;
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
};

template<class T> bool operator >  (const bigint & a, const T & b) { return a >  bigint(b); }
template<class T> bool operator <  (const bigint & a, const T & b) { return a <  bigint(b); }
template<class T> bool operator >= (const bigint & a, const T & b) { return a >= bigint(b); }
template<class T> bool operator <= (const bigint & a, const T & b) { return a <= bigint(b); }
template<class T> bool operator == (const bigint & a, const T & b) { return a == bigint(b); }
template<class T> bool operator != (const bigint & a, const T & b) { return a != bigint(b); }
template<class T> bool operator >  (const T & a, const bigint & b) { return bigint(a) >  b; }
template<class T> bool operator <  (const T & a, const bigint & b) { return bigint(a) <  b; }
template<class T> bool operator >= (const T & a, const bigint & b) { return bigint(a) >= b; }
template<class T> bool operator <= (const T & a, const bigint & b) { return bigint(a) <= b; }
template<class T> bool operator == (const T & a, const bigint & b) { return bigint(a) == b; }
template<class T> bool operator != (const T & a, const bigint & b) { return bigint(a) != b; }
template<class T> bigint operator + (const bigint & a, const T & b) { return a + bigint(b);  }
template<class T> bigint operator - (const bigint & a, const T & b) { return a - bigint(b);  }
template<class T> bigint operator ^ (const bigint & a, const T & b) { return a ^ bigint(b);  }
template<class T> bigint operator + (const T & a, const bigint & b) { return bigint(a) + b;  }
template<class T> bigint operator - (const T & a, const bigint & b) { return bigint(a) - b;  }
template<class T> bigint operator ^ (const T & a, const bigint & b) { return bigint(a) ^ b;  }
template<class T> bigint & operator += (bigint & a, const T & b) { return a += bigint(b); }
template<class T> bigint & operator -= (bigint & a, const T & b) { return a -= bigint(b); }
template<class T> bigint & operator ^= (bigint & a, const T & b) { return a ^= bigint(b); }

/*

Exclude templates for *, /, and % to force a user decision between algorithms:

bigint operator * (bigint a, bigint b) vs. bigint operator * (bigint a, int b)
bigint operator / (bigint a, bigint b) vs. bigint operator / (bigint a, int b)
bigint operator % (bigint a, bigint b) vs. int operator % (bigint a, int b)

*/

//template<class T> bigint operator * (const bigint & a, const T & b) { return a * bigint(b); }
//template<class T> bigint operator / (const bigint & a, const T & b) { return a / bigint(b); }
//template<class T> bigint operator % (const bigint & a, const T & b) { return a % bigint(b); }
//template<class T> bigint operator * (const T & a, const bigint & b) { return bigint(a) * b; }
//template<class T> bigint operator / (const T & a, const bigint & b) { return bigint(a) / b; }
//template<class T> bigint operator % (const T & a, const bigint & b) { return bigint(a) % b; }
//template<class T> bigint & operator *= (bigint & a, const T & b) { return a *= bigint(b); }
//template<class T> bigint & operator /= (bigint & a, const T & b) { return a /= bigint(b); }
//template<class T> bigint & operator %= (bigint & a, const T & b) { return a %= bigint(b); }


/*** Example Usage ***/

#include <cassert>
#include <cstdio>
#include <ctime>
#include <iostream>
using namespace std;

int main() {
  for (int i = 0; i < 1000; i++) {
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
  return 0;
}
