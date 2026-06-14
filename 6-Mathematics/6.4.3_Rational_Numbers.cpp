/*

Perform operations on rational numbers internally represented as two integers: a numerator and a
denominator. The template integer type must support streamed input/output, comparisons, and
arithmetic operations. Overflow is not checked for in internal operations: comparisons and
arithmetic cross-multiply numerators and denominators, so instantiate with a wider integer type
(such as `__int128`, or even `BigInt`) if the values may grow large.

- `Rational(n)` constructs a rational number with numerator `n` and denominator 1.
- `Rational(n, d)` constructs a rational number with numerator `n` and denominator `d`.
- `operator>>` inputs a rational number using the next integer from the stream as the numerator and
  1 as the denominator.
- `operator<<` outputs the rational number as a string consisting of possibly a minus sign followed
  by the numerator, followed by a slash, followed by the denominator.
- `v.to_string()`, `v.to_llong()`, `v.to_double()`, and `v.to_ldouble()` return the rational `v`
  converted to an `std::string`, `int64_t`, `double`, and `long double` respectively.
- `v.abs()`, `abs(v)`, `v.floor()`, and `v.ceil()` return the absolute value, floor, and ceiling of
  rational `v`.
- Operators `<`, `>`, `<=`, `>=`, `==`, `!=`, `+`, `-`, `*`, `/`, `%`, `++`, `--`, `+=`, `-=`, `*=`,
  `/=`, and `%=` are defined analogous to those on numerical primitives. The comparisons and binary
  arithmetic are hidden friends, so a raw integer operand works on either side.

Time Complexity:
- O(log(n + d)) per call to constructor `Rational(n, d)`.
- O(1) per call to all other operations, assuming that corresponding operations on the template
  integer type are O(1) as well.

Space Complexity:
- O(1) for storage of the rational number.
- O(1) auxiliary space for all operations.

*/

#include <cstdint>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

template<class Int>
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
    ss << num << "/" << den;
    return ss.str();
  }

  // to_llong/to_double/to_ldouble round-trip through a stringstream so that any Int supporting
  // streamed I/O (e.g. a big-integer type) converts, even without a direct cast to the target type.
  int64_t to_llong() const {
    std::stringstream ss;
    ss << num << " " << den;
    int64_t n, d;
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

  // The comparison and binary arithmetic operators are hidden friends, so a raw integer operand on
  // either side converts through the implicit constructor.
  friend bool operator<(const Rational &a, const Rational &b) {
    return a.num * b.den < b.num * a.den;
  }

  friend bool operator==(const Rational &a, const Rational &b) {
    return a.num == b.num && a.den == b.den;
  }

  // clang-format off
  friend bool operator>(const Rational &a, const Rational &b) { return b < a; }
  friend bool operator<=(const Rational &a, const Rational &b) { return !(b < a); }
  friend bool operator>=(const Rational &a, const Rational &b) { return !(a < b); }
  friend bool operator!=(const Rational &a, const Rational &b) { return !(a == b); }
  // clang-format on

  Rational abs() const { return Rational(num < 0 ? -num : num, den); }
  friend Rational abs(const Rational &r) { return r.abs(); }
  Int floor() const { return num < 0 ? -((-num + den - 1) / den) : num / den; }
  Int ceil() const { return num < 0 ? -(-num / den) : (num + den - 1) / den; }

  friend Rational operator+(const Rational &a, const Rational &b) {
    return Rational(a.num * b.den + b.num * a.den, a.den * b.den);
  }

  friend Rational operator-(const Rational &a, const Rational &b) {
    return Rational(a.num * b.den - b.num * a.den, a.den * b.den);
  }

  friend Rational operator*(const Rational &a, const Rational &b) {
    return Rational(a.num * b.num, a.den * b.den);
  }

  friend Rational operator/(const Rational &a, const Rational &b) {
    return Rational(a.num * b.den, a.den * b.num);
  }

  friend Rational operator%(const Rational &a, const Rational &b) {
    return a - b * Rational(a.num * b.den / (b.num * a.den), 1);
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
  // clang-format on
};

/*** Example Usage ***/

#include <cassert>
#include <cmath>

#define EQ(a, b) (fabs((a) - (b)) <= 1e-9)

int main() {
  using Rational = Rational<int64_t>;
  assert(Rational(-21, 1) % 2 == -1);
  Rational r(Rational(-53, 10) % Rational(-17, 10));
  assert(EQ(r.to_ldouble(), fmod(-5.3, -1.7)));
  assert(r.to_string() == "-1/5");

  // Raw integers work on either side of comparisons and arithmetic.
  assert(2 + Rational(1, 2) == Rational(5, 2));
  assert(1 < Rational(3, 2) && Rational(3, 2) < 2);
  assert(2 % Rational(3, 4) == Rational(1, 2));
  return 0;
}
