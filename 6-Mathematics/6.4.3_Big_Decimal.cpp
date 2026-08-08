/*

Perform exact decimal arithmetic using a `BigInt` coefficient and a nonnegative scale. Unlike binary
floating point, every parsed decimal is represented exactly: for example, `BigDecimal("1.20")`
stores coefficient $120$ with scale $2$. Addition and subtraction preserve the larger operand scale,
multiplication adds the scales, and formatting preserves trailing zeros unless `normalized()` is
called. Only finite, well-formed decimal strings are supported; `NaN` and infinities are not
represented.

Division and square root generally have nonterminating decimal expansions, so both operations
require an explicit result scale. The rounding mode defaults to `HalfAway`, which rounds ties away
from zero. The other modes are `TowardZero`, `AwayFromZero`, `Floor`, `Ceil`, and `HalfEven`. This
section imports the full `BigInt` implementation from 6.4.2 rather than duplicating its
arbitrary-precision arithmetic.

- `BigDecimal(n = 0)` constructs a decimal from integer `n`.
- `BigDecimal(s)` parses decimal string `s`, optionally written in scientific notation, and
  preserves trailing zeros in its mantissa.
- `decimal_places()`, `precision()`, and `sign()` return the scale, the number of coefficient
  digits, and the sign as $-1$, $0$, or $1$, respectively.
- `unscaled_value()` returns the underlying `BigInt` coefficient.
- `to_string()` returns ordinary decimal notation with the stored number of decimal places.
- `abs()` and `normalized()` return the absolute value and a value with trailing fractional zeros
  removed.
- `move_point(places)` moves the decimal point right by `places` positions, or left if `places` is
  negative.
- `rescale(new_scale, mode = Rounding::HalfAway)` returns the value with exactly `new_scale` decimal
  places, rounding if the scale is reduced.
- `trunc()`, `floor()`, and `ceil()` return the corresponding integer as a `BigInt`.
- `divide(v, result_scale, mode = Rounding::HalfAway)` divides by nonzero `v` and rounds to exactly
  `result_scale` decimal places.
- `sqrt(result_scale, mode = Rounding::HalfAway)` returns the nonnegative square root rounded to
  exactly `result_scale` decimal places.
- `pow(n)` returns the value raised to nonnegative integer power `n`.
- Stream operators and comparisons are defined, as are `+`, `-`, `*`, `%`, and their compound
  assignments.

Time Complexity:
- O(d) per call to parsing, formatting, normalization, addition, subtraction, comparisons, and
  decimal-point movement, where $d$ is the number of relevant decimal digits.
- O(M(d) log n) per call to `pow(n)`, where $M(d)$ is the multiplication cost for the largest
  intermediate coefficient.
- O(d^2) per call to division, remainder, rescaling to fewer places, and square root, matching the
  corresponding `BigInt` operations used here.

Space Complexity:
- O(d) for stored digits, returned values, and arithmetic results.

*/

#define main bigint_example
#include "6.4.2_Big_Integer.cpp"
#undef main

class BigDecimal {
 public:
  enum class Round { TowardZero, AwayFromZero, Floor, Ceil, HalfAway, HalfEven };

 private:
  BigInt coeff;
  int scale;

  BigDecimal(BigInt coeff, int scale) : coeff(std::move(coeff)), scale(scale) {}
  BigInt scaled(int new_scale) const { return coeff * pow10(new_scale - scale); }
  static BigInt pow10(int n) { return BigInt(10).pow(n); }

  static bool round_up(bool negative, int half_cmp, const BigInt &q, Round mode) {
    return mode == Round::AwayFromZero || (mode == Round::Floor && negative) ||
           (mode == Round::Ceil && !negative) || (mode == Round::HalfAway && half_cmp >= 0) ||
           (mode == Round::HalfEven && (half_cmp > 0 || (half_cmp == 0 && q % 2 != 0)));
  }

  static BigInt rounded_quotient(BigInt num, const BigInt &den, Round mode) {
    assert(den > 0);
    bool negative = num < 0;
    auto [q, r] = num.abs().div(den);
    bool increment = r != 0 && round_up(negative, (2 * r).comp(den), q, mode);
    return negative ? -(q + increment) : q + increment;
  }

 public:
  BigDecimal(int64_t n = 0) : coeff(n), scale(0) {}

  explicit BigDecimal(const std::string &s) : coeff(0), scale(0) {
    size_t e = s.find_first_of("eE");
    std::string mantissa = s.substr(0, e);
    int exponent = e == std::string::npos ? 0 : std::stoi(s.substr(e + 1));
    int pos = 0;
    bool negative = false, point = false;
    if (pos < static_cast<int>(mantissa.size()) && (mantissa[pos] == '+' || mantissa[pos] == '-')) {
      negative = mantissa[pos++] == '-';
    }
    std::string digits;
    for (; pos < static_cast<int>(mantissa.size()); pos++) {
      if (mantissa[pos] == '.') {
        assert(!point);
        point = true;
      } else {
        assert('0' <= mantissa[pos] && mantissa[pos] <= '9');
        digits += mantissa[pos];
        scale += point;
      }
    }
    assert(!digits.empty());
    coeff = negative ? -BigInt(digits) : BigInt(digits);
    scale -= exponent;
    if (scale < 0) {
      coeff *= pow10(-scale);
      scale = 0;
    }
  }

  int decimal_places() const { return scale; }
  int precision() const { return coeff.abs().size(); }
  int sign() const { return (coeff > 0) - (coeff < 0); }
  const BigInt &unscaled_value() const { return coeff; }
  BigDecimal abs() const { return BigDecimal(coeff.abs(), scale); }

  std::string to_string() const {
    std::string s = coeff.abs().to_string();
    if (static_cast<int>(s.size()) <= scale) {
      s.insert(0, scale + 1 - s.size(), '0');
    }
    if (scale > 0) {
      s.insert(s.size() - scale, 1, '.');
    }
    return coeff < 0 ? "-" + s : s;
  }

  BigDecimal normalized() const {
    BigInt c = coeff;
    int s = scale;
    while (s > 0 && c % 10 == 0) {
      c /= 10;
      s--;
    }
    return BigDecimal(c, s);
  }

  BigDecimal move_point(int places) const {
    int new_scale = scale - places;
    return new_scale >= 0 ? BigDecimal(coeff, new_scale) : BigDecimal(coeff * pow10(-new_scale), 0);
  }

  BigDecimal rescale(int new_scale, Round mode = Round::HalfAway) const {
    assert(new_scale >= 0);
    return new_scale >= scale
               ? BigDecimal(scaled(new_scale), new_scale)
               : BigDecimal(rounded_quotient(coeff, pow10(scale - new_scale), mode), new_scale);
  }

  BigDecimal divide(const BigDecimal &v, int result_scale, Round mode = Round::HalfAway) const {
    assert(v.coeff != 0 && result_scale >= 0);
    BigInt num = coeff.abs(), den = v.coeff.abs();
    int shift = v.scale + result_scale - scale;
    (shift >= 0 ? num : den) *= pow10(std::abs(shift));
    if ((coeff < 0) != (v.coeff < 0)) {
      num = -num;
    }
    return BigDecimal(rounded_quotient(num, den, mode), result_scale);
  }

  BigDecimal sqrt(int result_scale, Round mode = Round::HalfAway) const {
    assert(coeff >= 0 && result_scale >= 0);
    BigInt num = coeff, den = 1;
    int shift = 2 * result_scale - scale;
    (shift >= 0 ? num : den) *= pow10(std::abs(shift));
    BigInt q = (num / den).sqrt();
    bool exact = q * q * den == num;
    int half_cmp = (4 * num).comp(den * (4 * q * q + 4 * q + 1));
    return BigDecimal(q + (!exact && round_up(false, half_cmp, q, mode)), result_scale);
  }

  BigInt trunc() const { return rescale(0, Round::TowardZero).coeff; }
  BigInt floor() const { return rescale(0, Round::Floor).coeff; }
  BigInt ceil() const { return rescale(0, Round::Ceil).coeff; }

  friend std::istream &operator>>(std::istream &in, BigDecimal &v) {
    std::string s;
    if (in >> s) {
      v = BigDecimal(s);
    }
    return in;
  }

  friend std::ostream &operator<<(std::ostream &out, const BigDecimal &v) {
    return out << v.to_string();
  }

  friend bool operator<(const BigDecimal &a, const BigDecimal &b) {
    int s = std::max(a.scale, b.scale);
    return a.scaled(s) < b.scaled(s);
  }

  friend bool operator==(const BigDecimal &a, const BigDecimal &b) {
    int s = std::max(a.scale, b.scale);
    return a.scaled(s) == b.scaled(s);
  }

  // clang-format off
  friend bool operator>(const BigDecimal &a, const BigDecimal &b) { return b < a; }
  friend bool operator<=(const BigDecimal &a, const BigDecimal &b) { return !(b < a); }
  friend bool operator>=(const BigDecimal &a, const BigDecimal &b) { return !(a < b); }
  friend bool operator!=(const BigDecimal &a, const BigDecimal &b) { return !(a == b); }
  // clang-format on

  friend BigDecimal operator+(const BigDecimal &a, const BigDecimal &b) {
    int s = std::max(a.scale, b.scale);
    return BigDecimal(a.scaled(s) + b.scaled(s), s);
  }

  friend BigDecimal operator-(const BigDecimal &a, const BigDecimal &b) { return a + -b; }

  friend BigDecimal operator*(const BigDecimal &a, const BigDecimal &b) {
    return BigDecimal(a.coeff * b.coeff, a.scale + b.scale);
  }

  friend BigDecimal operator%(const BigDecimal &a, const BigDecimal &b) {
    assert(b.coeff != 0);
    int s = std::max(a.scale, b.scale);
    return BigDecimal(a.scaled(s) % b.scaled(s), s);
  }

  BigDecimal operator-() const { return BigDecimal(-coeff, scale); }

  BigDecimal pow(int n) const {
    assert(n >= 0);
    return BigDecimal(coeff.pow(n), scale * n);
  }

  BigDecimal &operator+=(const BigDecimal &v) { return *this = *this + v; }
  BigDecimal &operator-=(const BigDecimal &v) { return *this = *this - v; }
  BigDecimal &operator*=(const BigDecimal &v) { return *this = *this * v; }
  BigDecimal &operator%=(const BigDecimal &v) { return *this = *this % v; }
};

/*** Example Usage ***/

int main() {
  using Round = BigDecimal::Round;

  assert(BigDecimal().to_string() == "0");
  assert(BigDecimal(-12).to_string() == "-12");
  assert(BigDecimal("+.50").to_string() == "0.50");
  assert(BigDecimal("1.20e3").to_string() == "1200");
  assert(BigDecimal("1.20e-2").to_string() == "0.0120");

  BigDecimal a("1.20"), b("3.4");
  assert(a.decimal_places() == 2 && a.precision() == 3);
  assert(a.unscaled_value() == 120);
  assert(a.sign() == 1 && BigDecimal(0).sign() == 0 && BigDecimal(-1).sign() == -1);
  assert(BigDecimal("-1.20").abs().to_string() == "1.20");
  assert(a.normalized().to_string() == "1.2");
  assert(a == BigDecimal("1.2") && a != b);
  assert(a < b && b > a && a <= BigDecimal("1.200") && b >= a);

  assert((a + b).to_string() == "4.60");
  assert((b - a).to_string() == "2.20");
  assert((a * b).to_string() == "4.080");
  assert((-a).to_string() == "-1.20");
  assert((BigDecimal("5.50") % 2).to_string() == "1.50");

  BigDecimal x = a;
  assert((x += b).to_string() == "4.60");
  assert((x -= b).to_string() == "1.20");
  assert((x *= b).to_string() == "4.080");
  assert((x %= BigDecimal("1.00")).to_string() == "0.080");

  assert(a.move_point(2).to_string() == "120");
  assert(a.move_point(-1).to_string() == "0.120");
  assert(a.rescale(4).to_string() == "1.2000");
  assert(BigDecimal("1.29").rescale(1, Round::TowardZero).to_string() == "1.2");
  assert(BigDecimal("1.21").rescale(1, Round::AwayFromZero).to_string() == "1.3");
  assert(BigDecimal("-1.21").rescale(1, Round::Floor).to_string() == "-1.3");
  assert(BigDecimal("-1.21").rescale(1, Round::Ceil).to_string() == "-1.2");
  assert(BigDecimal("1.25").rescale(1, Round::HalfAway).to_string() == "1.3");
  assert(BigDecimal("2.685").rescale(2, Round::HalfEven).to_string() == "2.68");

  assert(BigDecimal(1).divide(8, 4).to_string() == "0.1250");
  assert(BigDecimal(2).divide(3, 4).to_string() == "0.6667");
  assert(BigDecimal(-1).divide(8, 2).to_string() == "-0.13");
  assert(BigDecimal(1).divide(8, 2, Round::HalfEven).to_string() == "0.12");

  assert(BigDecimal("-1.2").trunc() == -1);
  assert(BigDecimal("-1.2").floor() == -2);
  assert(BigDecimal("-1.2").ceil() == -1);
  assert(a.pow(3).to_string() == "1.728000");

  assert(BigDecimal(2).sqrt(5).to_string() == "1.41421");
  assert(BigDecimal("2.25").sqrt(2).to_string() == "1.50");
  assert(BigDecimal("2.25").sqrt(0, Round::HalfAway).to_string() == "2");
  assert(BigDecimal("6.25").sqrt(0, Round::HalfEven).to_string() == "2");
  assert(BigDecimal(2).sqrt(0, Round::Ceil).to_string() == "2");
  for (int i = 0; i <= 100; i++) {
    assert(BigDecimal(i * i).sqrt(0) == BigDecimal(i));
  }

  std::stringstream in("-12.340"), out;
  BigDecimal streamed;
  in >> streamed;
  out << streamed;
  assert(out.str() == "-12.340");

  assert((BigDecimal("12345678901234567890.12") * 10).to_string() == "123456789012345678901.20");
  return 0;
}
