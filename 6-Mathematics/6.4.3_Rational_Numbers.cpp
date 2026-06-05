/*

Perform operations on Rational numbers internally represented as two integers, a numerator and a
denominator. The template integer type must support streamed input/output, comparisons, and
arithmetic operations. Overflow is not checked for in internal operations.

- `Rational(n)` constructs a Rational with numerator `n` and denominator 1.
- `Rational(n, d)` constructs a Rational with numerator `n` and denominator `d`.
- `operator >>` inputs a Rational using the next integer from the stream as the numerator and 1 as
  the denominator.
- `operator <<` outputs a Rational as a string consisting of possibly a minus sign followed by the
  numerator, followed by a slash, followed by the denominator.
- `v.to_string()`, `v.to_llong()`, `v.to_double()`, and `v.to_ldouble()` return the big integer `v`
  converted to an `std::string`, `long long`, `double`, and `long double` respectively.
- Operators `<`, `>`, `<=`, `>=`, `==`, `!=`, `+`, `-`, `*`, `/`, `%`, `++`, `--`, `+=`, `-=`, `*=`,
  `/=`, and `%=` are defined analogous to those on numerical primitives.

Time Complexity:
- O(log(n + d)) per call to constructor `Rational(n, d)`.
- O(1) per call to all other operations, assuming that corresponding operations on the template
  integer type are O(1) as well.

Space Complexity:
- O(1) for storage of the Rational.
- O(1) auxiliary space for all operations.

*/

#include <istream>
#include <ostream>
#include <sstream>
#include <string>

template<class Int = long long>
class Rational {
  Int num, den;

 public:
  Rational() : num(0), den(1) {}
  Rational(const Int &n) : num(n), den(1) {}

  template<class T1, class T2>
  Rational(const T1 &n, const T2 &d) : num(n), den(d) {
    if (den == 0) {
      throw std::runtime_error("Division by zero in Rational.");
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

  friend std::istream &operator>>(std::istream &in, Rational &r) {
    std::string s;
    in >> r.num;
    r.den = 1;
    return in;
  }

  friend std::ostream &operator<<(std::ostream &out, const Rational &r) {
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
    return n / d;
  }

  double to_double() const {
    std::stringstream ss;
    ss << num << " " << den;
    double n, d;
    ss >> n >> d;
    return n / d;
  }

  long double to_ldouble() const {
    long double n, d;
    std::stringstream ss;
    ss << num << " " << den;
    ss >> n >> d;
    return n / d;
  }

  bool operator<(const Rational &r) const { return num * r.den < r.num * den; }
  bool operator>(const Rational &r) const { return r.num * den < num * r.den; }
  bool operator<=(const Rational &r) const { return !(r < *this); }
  bool operator>=(const Rational &r) const { return !(*this < r); }
  bool operator==(const Rational &r) const { return num == r.num && den == r.den; }
  bool operator!=(const Rational &r) const { return num != r.num || den != r.den; }

  // clang-format off
  template<class T>
  friend bool operator<(const T &a, const Rational &b) { return Rational(a) < b; }

  template<class T>
  friend bool operator>(const T &a, const Rational &b) { return Rational(a) > b; }

  template<class T>
  friend bool operator<=(const T &a, const Rational &b) { return Rational(a) <= b; }

  template<class T>
  friend bool operator>=(const T &a, const Rational &b) { return Rational(a) >= b; }

  template<class T>
  friend bool operator==(const T &a, const Rational &b) { return Rational(a) == b; }

  template<class T>
  friend bool operator!=(const T &a, const Rational &b) { return Rational(a) != b; }
  // clang-format on

  Rational abs() const { return Rational(num < 0 ? -num : num, den); }
  friend Rational abs(const Rational &r) { return r.abs(); }
  Int floor() const { return num < 0 ? -((-num + den - 1) / den) : num / den; }
  Int ceil() const { return num < 0 ? -(-num / den) : (num + den - 1) / den; }

  Rational operator+(const Rational &r) const {
    return Rational(num * r.den + r.num * den, den * r.den);
  }

  Rational operator-(const Rational &r) const {
    return Rational(num * r.den - r.num * den, r.den * den);
  }

  Rational operator*(const Rational &r) const { return Rational(num * r.num, r.den * den); }
  Rational operator/(const Rational &r) const { return Rational(num * r.den, den * r.num); }

  Rational operator%(const Rational &r) const {
    return *this - r * Rational(num * r.den / (r.num * den), 1);
  }

  // clang-format off
  Rational operator-() const { return Rational(-num, den); }
  Rational operator++(int) { Rational t(*this); operator++(); return t; }
  Rational operator--(int) { Rational t(*this); operator--(); return t; }
  Rational &operator++() { *this = *this + 1; return *this; }
  Rational &operator--() { *this = *this - 1; return *this; }
  Rational &operator+=(const Rational &r) { *this = *this + r; return *this; }
  Rational &operator-=(const Rational &r) { *this = *this - r; return *this; }
  Rational &operator*=(const Rational &r) { *this = *this * r; return *this; }
  Rational &operator/=(const Rational &r) { *this = *this / r; return *this; }
  Rational &operator%=(const Rational &r) { *this = *this % r; return *this; }

  template<class T>
  friend Rational operator+(const T &a, const Rational &b) { return Rational(a) + b; }

  template<class T>
  friend Rational operator-(const T &a, const Rational &b) { return Rational(a) - b; }

  template<class T>
  friend Rational operator*(const T &a, const Rational &b) { return Rational(a) * b; }

  template<class T>
  friend Rational operator/(const T &a, const Rational &b) { return Rational(a) / b; }

  template<class T>
  friend Rational operator%(const T &a, const Rational &b) { return Rational(a) % b; }
  // clang-format on
};

/*** Example Usage ***/

#include <cassert>
#include <cmath>

int main() {
#define EQ(a, b) (fabs((a) - (b)) <= 1E-9)
  using Rational = Rational<long long>;

  assert(Rational(-21, 1) % 2 == -1);
  Rational r(Rational(-53, 10) % Rational(-17, 10));
  assert(EQ(r.to_ldouble(), fmod(-5.3, -1.7)));
  assert(r.to_string() == "-1/5");
  return 0;
}
