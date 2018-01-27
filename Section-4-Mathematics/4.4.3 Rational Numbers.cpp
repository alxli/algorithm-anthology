/*

Perform operations on rational numbers internally represented as two integers, a
numerator and a denominator. The template integer type must support streamed
input/output, comparisons, and arithmetic operations. Overflow is not checked
for in internal operations.

- rational(n) constructs a rational with numerator n and denominator 1.
- rational(n, d) constructs a rational with numerator n and denominator d.
- operator >> inputs a rational using the next integer from the stream as the
  numerator and 1 as the denominator.
- operator << outputs a rational as a string consisting of possibly a minus sign
  followed by the numerator, followed by a slash, followed by the denominator.
- v.to_string(), v.to_llong(), v.to_double(), and v.to_ldouble() return the big
  integer v converted to an std::string, long long, double, and long double
  respectively.
- operators <, >, <=, >=, ==, !=, +, -, *, /, %, ++, --, +=, -=, *=, /=, and %=
  are defined analogous to those on numerical primitives.

Time Complexity:
- O(log(n + d)) per call to constructor rational(n, d).
- O(1) per call to all other operations, assuming that corresponding operations
  on the template integer type are O(1) as well.

Space Complexity:
- O(1) for storage of the rational.
- O(1) auxiliary space for all operations.

*/

#include <istream>
#include <ostream>
#include <sstream>
#include <string>

template<class Int = long long>
class rational {
  Int num, den;

 public:
  rational(): num(0), den(1) {}
  rational(const Int &n) : num(n), den(1) {}

  template<class T1, class T2>
  rational(const T1 &n, const T2 &d): num(n), den(d) {
    if (den == 0) {
      throw std::runtime_error("Division by zero in rational.");
    }
    if (den < 0) {
      num = -num;
      den = -den;
    }
    Int a(num < 0 ? -num : num), b(den), tmp;
    while (a != 0 && b != 0) {
      tmp = a % b;
      a = b;
      b = tmp;
    }
    Int gcd = (b == 0) ? a : b;
    num /= gcd;
    den /= gcd;
  }

  friend std::istream& operator>>(std::istream &in, rational &r) {
    std::string s;
    in >> r.num;
    r.den = 1;
    return in;
  }

  friend std::ostream& operator<<(std::ostream &out, const rational &r) {
    out << r.num << "/" << r.den;
    return out;
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << num << " " << den;
    std::string n, d;
    ss >> n >> d;
    return n + "/" + d;
  }

  long long to_llong() const {
    std::stringstream ss;
    ss << num << " " << den;
    long long n, d;
    ss >> n >> d;
    return n/d;
  }

  double to_double() const {
    std::stringstream ss;
    ss << num << " " << den;
    double n, d;
    ss >> n >> d;
    return n/d;
  }

  long double to_ldouble() const {
    long double n, d;
    std::stringstream ss;
    ss << num << " " << den;
    ss >> n >> d;
    return n/d;
  }

  bool operator<(const rational &r) const {
    return num*r.den < r.num*den;
  }

  bool operator>(const rational &r) const {
    return r.num*den < num*r.den;
  }

  bool operator<=(const rational &r) const {
    return !(r < *this);
  }

  bool operator>=(const rational &r) const {
    return !(*this < r);
  }

  bool operator==(const rational &r) const {
    return num == r.num && den == r.den;
  }

  bool operator!=(const rational &r) const {
    return num != r.num || den != r.den;
  }

  template<class T>
  friend bool operator<(const T &a, const rational &b) {
    return rational(a) < b;
  }

  template<class T>
  friend bool operator>(const T &a, const rational &b) {
    return rational(a) > b;
  }

  template<class T>
  friend bool operator<=(const T &a, const rational &b) {
    return rational(a) <= b;
  }

  template<class T>
  friend bool operator>=(const T &a, const rational &b) {
    return rational(a) >= b;
  }

  template<class T>
  friend bool operator==(const T &a, const rational &b) {
    return rational(a) == b;
  }

  template<class T>
  friend bool operator!=(const T &a, const rational &b) {
    return rational(a) != b;
  }

  rational abs() const {
    return rational(num < 0 ? -num : num, den);
  }

  friend rational abs(const rational &r) { return r.abs(); }

  rational operator+(const rational &r) const {
    return rational(num*r.den + r.num*den, den*r.den);
  }

  rational operator-(const rational &r) const {
    return rational(num*r.den - r.num*den, r.den*den);
  }

  rational operator*(const rational &r) const {
    return rational(num*r.num, r.den*den);
  }

  rational operator/(const rational &r) const {
    return rational(num*r.den, den*r.num);
  }

  rational operator%(const rational &r) const {
    return *this - r*rational(num*r.den/(r.num*den), 1);
  }

  template<class T>
  friend rational operator+(const T &a, const rational &b) {
    return rational(a) + b;
  }

  template<class T>
  friend rational operator-(const T &a, const rational &b) {
    return rational(a) - b;
  }

  template<class T>
  friend rational operator*(const T &a, const rational &b) {
    return rational(a) * b;
  }

  template<class T>
  friend rational operator/(const T &a, const rational &b) {
    return rational(a) / b;
  }

  template<class T>
  friend rational operator%(const T &a, const rational &b) {
    return rational(a) % b;
  }

  rational operator-() const { return rational(-num, den); }
  rational operator++(int) { rational t(*this); operator++(); return t; }
  rational operator--(int) { rational t(*this); operator--(); return t; }
  rational& operator++() { *this = *this + 1; return *this; }
  rational& operator--() { *this = *this - 1; return *this; }
  rational& operator+=(const rational &r) { *this = *this + r; return *this; }
  rational& operator-=(const rational &r) { *this = *this - r; return *this; }
  rational& operator*=(const rational &r) { *this = *this * r; return *this; }
  rational& operator/=(const rational &r) { *this = *this / r; return *this; }
  rational& operator%=(const rational &r) { *this = *this % r; return *this; }
};

/*** Example Usage ***/

#include <cassert>
#include <cmath>

int main() {
  #define EQ(a, b) (fabs((a) - (b)) <= 1E-9)
  typedef rational<long long> rational;

  assert(rational(-21, 1) % 2 == -1);
  rational r(rational(-53, 10) % rational(-17, 10));
  assert(EQ(r.to_ldouble(), fmod(-5.3, -1.7)));
  assert(r.to_string() == "-1/5");
  return 0;
}
