/*

Common mathematic constants and functions, most of which already have standard STL equivalents. The
implementations below are for educational purposes only and may not be as heavily optimized as their
standard library counterparts.

Time Complexity:
- O(1) per call to most operations.
- O(1) per call to `mulmod()` with `__uint128_t`, or O(log n) with the portable fallback, where $n$
  is the second argument.
- O(log n) calls to `mulmod()` per call to `powmod(x, n, m)`.
- O(d*e) per call to `convert_base(d, a, b)`, where $d$ is the number of input digits and $e$ is the
  number of output digits.
- O(log_b(x + 1) + 1) per call to `to_base()`.
- O(x / 1000 + 1) per call to `to_roman()` due to the repeated `M` prefix.

Space Complexity:
- O(1) auxiliary for most operations.
- O(d + e) auxiliary for `convert_base(d, a, b)`, where $d$ is the number of input digits and $e$ is
  the number of output digits.
- O(e) auxiliary for `to_base(x, b)`, where $e$ is the number of output digits.
- O(x / 1000 + 1) auxiliary for `to_roman(x)`.

*/

#include <algorithm>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef M_PI
const double M_PI = std::acos(-1.0);  // Or std::numbers::pi in C++20 and later.
#endif
#ifndef M_E
const double M_E = std::exp(1.0);  // or std::numbers::e and std::numbers::e_v<> in C++20 and later
#endif
const double M_PHI = (1.0 + std::sqrt(5.0)) / 2.0;
const double M_INF = std::numeric_limits<double>::infinity();
const double M_NAN = std::numeric_limits<double>::quiet_NaN();

/*

Epsilon Comparisons:

- `EQ()`, `NE()`, `LT()`, `GT()`, `LE()`, and `GE()` relationally compare two values $x$ and $y$.
  Arguments may be of different types; the common type governs behavior. If the common type is a
  floating-point type, exactly equal values (including same-signed infinities) compare equal;
  otherwise absolute-error epsilon comparison is used. Values within `EPS` of each other are
  considered equal, and `LT`/`GT`/`LE`/`GE` shift the boundary by `EPS` accordingly. Otherwise exact
  comparison is used (`==`, `<`, etc.). The branch is selected with `if constexpr`, so exact types
  without floating-point arithmetic (e.g. integers, `Modular`, `Rational`) compose as well.
- `rEQ(ref, val)` returns whether `val` equals reference `ref` within relative error `EPS`. The
  tolerance scales with $|`ref`|$, so `rEQ(ref, val)` is NOT the same as `rEQ(val, ref)`. Use this
  when one argument is a known exact value and the other is a computed approximation. Degenerates to
  exact comparison when `ref` is $0$, since the tolerance collapses to $0$; use `EQ` near zero.
- `rEQ_sym(x, y)` is the symmetric (commutative) variant: tolerance scales with
  $\max(|`x`|, |`y`|)$, so the result is the same regardless of argument order. Still degenerates
  near zero when both arguments are close to $0$. For both relative comparisons, exactly equal
  infinities compare equal, while unequal infinities and comparisons between finite and non-finite
  values compare unequal.

*/

const double EPS = 1e-9;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) == C(b) || std::fabs(C(a) - C(b)) <= EPS;
  return C(a) == C(b);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool LT(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) < C(b) - EPS;
  return C(a) < C(b);
}

// clang-format off
template<typename T, typename U> bool NE(T a, U b) { return !EQ(a, b); }
template<typename T, typename U> bool GT(T a, U b) { return LT(b, a); }
template<typename T, typename U> bool LE(T a, U b) { return !LT(b, a); }
template<typename T, typename U> bool GE(T a, U b) { return !LT(a, b); }
// clang-format on

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool rEQ(T ref, U val) {
  C x = C(ref), y = C(val);
  if (x == y) return true;
  if constexpr (std::is_floating_point_v<C>) {
    if (!std::isfinite(x) || !std::isfinite(y)) return false;
  }
  return std::fabs(x - y) <= EPS * std::fabs(x);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool rEQ_sym(T x, U y) {
  C a = C(x), b = C(y);
  if (a == b) return true;
  if constexpr (std::is_floating_point_v<C>) {
    if (!std::isfinite(a) || !std::isfinite(b)) return false;
  }
  return std::fabs(a - b) <= EPS * std::max(std::fabs(a), std::fabs(b));
}

/*

Sign Functions:

- `sgn(x)` returns $-1$ (if $x < 0$), $0$ (if $x = 0$), or $1$ (if $x > 0$). Unlike `std::signbit()`
  or `std::copysign()`, this does not handle the sign of `NaN`.
- `signbit_(x)` is analogous to `std::signbit()`, returning whether the sign bit of the floating
  point number is set to true. If so, then `x` is considered "negative." Note that this works as
  expected on `+0.0`, `-0.0`, `Inf`, `-Inf`, `NaN`, as well as `-NaN`. Warning: This assumes that
  the sign bit is the leading (most significant) bit in the internal IEEE representation and that
  bytes are stored in little-endian order.
- `copysign_(x, y)` is analogous to `std::copysign()`, returning a number with the magnitude of `x`
  but the sign of `y`.

*/

template<typename T>
int sgn(const T &x) {
  return (T(0) < x) - (x < T(0));
}

template<typename Double>
bool signbit_(Double x) {
  return (((unsigned char *)&x)[sizeof(x) - 1] >> (CHAR_BIT - 1)) & 1;
}

template<typename Double>
Double copysign_(Double x, Double y) {
  return signbit_(y) ? -std::fabs(x) : std::fabs(x);
}

/*

Rounding Functions:

- `floor0(x)` returns `x` rounded down, symmetrically towards zero. This function is analogous to
  `std::trunc()`.
- `ceil0(x)` returns `x` rounded up, symmetrically away from zero. This function is analogous to
  `std::round()`.
- `round_half_up(x)` returns `x` rounded half up, towards positive infinity.
- `round_half_down(x)` returns `x` rounded half down, towards negative infinity.
- `round_half_to0(x)` returns `x` rounded half down, symmetrically towards zero.
- `round_half_from0(x)` returns `x` rounded half up, symmetrically away from zero.
- `round_half_even(x, eps = 1e-9)` returns `x` rounded half to even, using `eps` to detect ties for
  banker's rounding.
- `round_half_alternate(x)` returns `x` rounded, where ties are broken by alternating rounds towards
  positive and negative infinity.
- `round_half_alternate0(x)` returns `x` rounded, where ties are broken by alternating symmetric
  rounds towards and away from zero.
- `round_half_random(x)` returns `x` rounded, where ties are broken randomly.
- `round_n_places(x, n, round)` returns `x` rounded to `n` digits after the decimal, using the
  specified rounding function `round(x)`.

*/

template<typename Double>
Double floor0(const Double &x) {
  Double res = std::floor(std::fabs(x));
  return (x < 0.0) ? -res : res;
}

template<typename Double>
Double ceil0(const Double &x) {
  Double res = std::ceil(std::fabs(x));
  return (x < 0.0) ? -res : res;
}

template<typename Double>
Double round_half_up(const Double &x) {
  return std::floor(x + 0.5);
}

template<typename Double>
Double round_half_down(const Double &x) {
  return std::ceil(x - 0.5);
}

template<typename Double>
Double round_half_to0(const Double &x) {
  Double res = round_half_down(std::fabs(x));
  return (x < 0.0) ? -res : res;
}

template<typename Double>
Double round_half_from0(const Double &x) {
  Double res = round_half_up(std::fabs(x));
  return (x < 0.0) ? -res : res;
}

template<typename Double>
Double round_half_even(const Double &x, const Double &eps = 1e-9) {
  if (x < 0.0) {
    return -round_half_even(-x, eps);
  }
  Double ipart;
  std::modf(x, &ipart);
  if (std::fabs(x - (ipart + 0.5)) < eps) {  // exactly halfway: break the tie towards even
    return (std::fmod(ipart, 2.0) < eps) ? ipart : ceil0(ipart + 0.5);
  }
  return round_half_from0(x);
}

template<typename Double>
Double round_half_alternate(const Double &x) {
  Double up = round_half_up(x), down = round_half_down(x);
  if (up == down) {
    return up;
  }
  static bool round_up = false;
  return (round_up = !round_up) ? up : down;
}

template<typename Double>
Double round_half_alternate0(const Double &x) {
  Double away = round_half_from0(x), toward = round_half_to0(x);
  if (away == toward) {
    return away;
  }
  static bool round_away = false;
  return (round_away = !round_away) ? away : toward;
}

template<typename Double>
Double round_half_random(const Double &x) {
  Double away = round_half_from0(x), toward = round_half_to0(x);
  if (away == toward) {
    return away;
  }
  static std::mt19937 rng(std::random_device{}());
  return (rng() % 2 == 0) ? away : toward;
}

template<typename Double, typename RoundFn>
Double round_n_places(const Double &x, unsigned int n, RoundFn round) {
  Double scale = std::pow(Double(10), n);
  return round(x * scale) / scale;
}

/*

Modular Arithmetic:

- `addmod(a, b, m)` and `submod(a, b, m)` respectively return addition and subtraction modulo `m`,
  each result in $[0, `m`)$. Both operands may be negative or unreduced; they are normalized before
  arithmetic to avoid signed overflow. The modulus `m` must be positive.
- `mulmod(x, n, m)` returns `x` multiplied by `n`, modulo `m`. This is done in a way to avoid
  overflow: on compilers with `__uint128_t` it uses one wide product, while the portable fallback
  uses double-and-add multiplication. The fallback is slower by a logarithmic factor, but avoids
  relying on nonstandard 128-bit integers. Unlike `addmod`/`submod`, this takes unsigned operands
  and supports a full 64-bit modulus.
- `powmod(x, n, m)` returns `x` raised to the power `n`, modulo `m`.

These are lightweight standalone helpers for the occasional modular computation. For a full value
type that wraps a fixed modulus with overloaded operators, modular inverses, and combinatorics, see
the `Modular` ("Mint") class in section 6.3.2.

*/

int64_t addmod(int64_t a, int64_t b, int64_t m) {
  assert(m > 0);
  a %= m;
  b %= m;
  if (a < 0) a += m;
  if (b < 0) b += m;
  return a >= m - b ? a - (m - b) : a + b;
}

int64_t submod(int64_t a, int64_t b, int64_t m) {
  assert(m > 0);
  a %= m;
  b %= m;
  if (a < 0) a += m;
  if (b < 0) b += m;
  return a >= b ? a - b : m - (b - a);
}

uint64_t mulmod(uint64_t x, uint64_t n, uint64_t m) {
  assert(m > 0);
#if defined(__SIZEOF_INT128__)
  return static_cast<uint64_t>(static_cast<__uint128_t>(x) * n % m);
#else
  uint64_t a = 0, b = x % m;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      a = a >= m - b ? a - (m - b) : a + b;
    }
    b = b >= m - b ? b - (m - b) : b + b;
  }
  return a;
#endif
}

uint64_t powmod(uint64_t x, uint64_t n, uint64_t m) {
  assert(m > 0);
  uint64_t a = 1, b = x;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      a = mulmod(a, b, m);
    }
    b = mulmod(b, b, m);
  }
  return a % m;
}

/*

Base Conversion:

- `to_base(x, b = 10)` returns the digits of the unsigned integer `x` in base `b`, where index 0 of
  the result stores the least significant digit.
- `to_roman(x)` returns the Roman numeral representation of the unsigned integer `x` as a string.
- `convert_base(d, a, b)` converts an integer in base `a` as a vector `d` of digits (where `d[0]` is
  the least significant digit) to base `b` as a vector of digits (again with index 0 holding the
  least significant digit). This uses repeated long division, so the value itself does not need to
  fit in a machine integer. Each intermediate `rem*a + digit` must fit in `uint64_t`.

*/

std::vector<int> to_base(unsigned int x, int b = 10) {
  assert(b >= 2);
  std::vector<int> res;
  do {  // do-while so that a value of 0 yields the single digit {0}
    res.push_back(x % b);
    x /= b;
  } while (x != 0);
  return res;
}

std::string to_roman(unsigned int x) {
  static const std::string h[] = {"", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM"};
  static const std::string t[] = {"", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC"};
  static const std::string o[] = {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX"};
  std::string prefix(x / 1000, 'M');
  x %= 1000;
  return prefix + h[x / 100] + t[x / 10 % 10] + o[x % 10];
}

std::vector<int> convert_base(const std::vector<int> &d, int a, int b) {
  assert(a >= 2 && b >= 2);
  std::vector<int> cur = d, res;
  auto trim = [](std::vector<int> &v) {
    while (v.size() > 1 && v.back() == 0) {
      v.pop_back();
    }
  };
  trim(cur);
  if (cur.empty() || (cur.size() == 1 && cur[0] == 0)) {
    return {0};
  }
  while (!(cur.size() == 1 && cur[0] == 0)) {
    std::vector<int> q(cur.size());
    uint64_t rem = 0;
    for (int i = static_cast<int>(cur.size()) - 1; i >= 0; i--) {
      uint64_t x = rem * static_cast<uint64_t>(a) + static_cast<uint64_t>(cur[i]);
      q[i] = static_cast<int>(x / static_cast<uint64_t>(b));
      rem = x % static_cast<uint64_t>(b);
    }
    res.push_back(static_cast<int>(rem));
    trim(q);
    cur = std::move(q);
  }
  return res;
}

/*** Example Usage ***/

using namespace std;

int main() {
  assert(EQ(M_PI, 3.14159265359));
  assert(EQ(M_INF, M_INF) && rEQ(M_INF, M_INF) && rEQ_sym(M_INF, M_INF));
  assert(!rEQ(M_INF, 0.0) && !rEQ_sym(M_INF, -M_INF));
  assert(EQ(M_E, 2.718281828459));
  assert(EQ(M_PHI, 1.61803398875));

  double x = -12345.6789;
  assert((-M_INF < x) && (x < M_INF));
  assert((M_INF + x == M_INF) && (M_INF - x == M_INF));
  assert((M_INF + M_INF == M_INF) && (-M_INF - M_INF == -M_INF));
  assert((M_NAN != x) && (M_NAN != M_INF) && (M_NAN != M_NAN));
  assert(!(M_NAN < x) && !(M_NAN > x) && !(M_NAN <= x) && !(M_NAN >= x));
  assert(isnan(0.0 * M_INF) && isnan(0.0 * -M_INF) && isnan(M_INF / -M_INF));
  assert(isnan(M_NAN) && isnan(-M_NAN) && isnan(M_INF - M_INF));

  assert(sgn(x) == -1 && sgn(0.0) == 0 && sgn(5678) == 1);
  assert(signbit_(x) && !signbit_(0.0) && signbit_(-0.0));
  assert(!signbit_(M_INF) && signbit_(-M_INF));
  assert(!signbit_(M_NAN) && signbit_(-M_NAN));
  assert(copysign(1.0, +2.0) == +1.0 && copysign(M_INF, -2.0) == -M_INF);
  assert(copysign(1.0, -2.0) == -1.0 && signbit(copysign(M_NAN, -2.0)));

  assert(EQ(floor0(1.5), 1.0) && EQ(ceil0(1.5), 2.0));
  assert(EQ(floor0(-1.5), -1.0) && EQ(ceil0(-1.5), -2.0));
  assert(EQ(round_half_up(+1.5), +2) && EQ(round_half_down(+1.5), +1));
  assert(EQ(round_half_up(-1.5), -1) && EQ(round_half_down(-1.5), -2));
  assert(EQ(round_half_to0(+1.5), +1) && EQ(round_half_from0(+1.5), +2));
  assert(EQ(round_half_to0(-1.5), -1) && EQ(round_half_from0(-1.5), -2));
  assert(EQ(round_half_even(+1.5), +2) && EQ(round_half_even(-1.5), -2));
  assert(EQ(round_half_even(3.1), 3) && EQ(round_half_even(3.4), 3));  // non-ties round normally
  double alt1 = round_half_alternate(+1.5);
  assert(EQ(round_half_alternate(+1.2), +1));  // Non-ties do not consume an alternating turn.
  double alt2 = round_half_alternate(+1.5);
  assert(NE(alt1, alt2));
  double alt01 = round_half_alternate0(-1.5);
  assert(EQ(round_half_alternate0(-1.2), -1));
  double alt02 = round_half_alternate0(-1.5);
  assert(NE(alt01, alt02));
  assert(EQ(round_n_places(-1.23456, 3, round_half_to0<double>), -1.235));

  assert(addmod(7, 8, 10) == 5 && submod(2, 5, 10) == 7);
  // Negative and unreduced operands are normalized into [0, m).
  assert(addmod(-3, -4, 10) == 3 && submod(-3, 4, 10) == 3);
  assert(addmod(25, -7, 10) == 8);
  assert(addmod(INT64_MAX - 1, INT64_MAX - 1, INT64_MAX) == INT64_MAX - 2);

  assert(powmod(2, 10, 1000000007) == 1024);
  assert(powmod(2, 62, 1000000) == 387904);
  assert(powmod(10001, 10001, 100000) == 10001);

  assert(to_roman(1234) == "MCCXXXIV");
  assert(to_roman(5678) == "MMMMMDCLXXVIII");
  vector<int> digits{6, 5, 4, 3, 2, 1};
  vector<int> base20 = to_base(123456, 20);
  assert(convert_base(base20, 20, 10) == digits);
  assert(convert_base(vector<int>{0, 0, 0}, 10, 2) == vector<int>{0});

  vector<int> big_decimal(30, 9);  // 10^30 - 1, larger than uint64_t.
  vector<int> big_binary = convert_base(big_decimal, 10, 2);
  assert(convert_base(big_binary, 2, 10) == big_decimal);
  return 0;
}
