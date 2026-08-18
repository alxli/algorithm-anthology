/*

Floating-point results carry rounding error, so two values that should be equal in exact arithmetic
usually differ in their last bits. Comparing them with `==` is therefore a bug in almost every
numeric setting, and the helpers below compare within a tolerance instead. This section also
collects the mathematical constants and the sign inspections that the rest of the anthology uses.

- `M_PI`, `M_E`, `M_PHI`, `M_INF`, and `M_NAN` are the usual constants, computed rather than assumed
  so that no platform-specific macro is required.
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
- `rEQ_sym(a, b)` is the symmetric (commutative) variant: tolerance scales with
  $\max(|`a`|, |`b`|)$, so the result is the same regardless of argument order. Still degenerates
  near zero when both arguments are close to $0$. For both relative comparisons, exactly equal
  infinities compare equal, while unequal infinities and comparisons between finite and non-finite
  values compare unequal. Non-floating common types use exact equality.
- `sgn(x)` returns $-1$ (if $x < 0$), $0$ (if $x = 0$), or $1$ (if $x > 0$). Unlike `std::signbit()`
  or `std::copysign()`, this does not handle the sign of `NaN`.
- `sign_bit(x)` is analogous to `std::signbit()`, returning whether the sign bit of the floating
  point number is set to true. If so, then `x` is considered "negative." Note that this works as
  expected on `+0.0`, `-0.0`, `Inf`, `-Inf`, `NaN`, as well as `-NaN`. Warning: This assumes that
  the sign bit is the leading (most significant) bit in the internal IEEE representation and that
  bytes are stored in little-endian order.
- `copy_sign(x, y)` is analogous to `std::copysign()`, returning a number with the magnitude of `x`
  but the sign of `y`.

Time Complexity:
- O(1) per call to all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <climits>
#include <cmath>
#include <limits>
#include <type_traits>

#ifndef M_PI
const double M_PI = std::acos(-1.0);  // Or std::numbers::pi in C++20 and later.
#endif
#ifndef M_E
const double M_E = std::exp(1.0);  // or std::numbers::e and std::numbers::e_v<> in C++20 and later
#endif
const double M_PHI = (1.0 + std::sqrt(5.0)) / 2.0;
const double M_INF = std::numeric_limits<double>::infinity();
const double M_NAN = std::numeric_limits<double>::quiet_NaN();

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
  if constexpr (!std::is_floating_point_v<C>) {
    return false;
  }
  if (!std::isfinite(x) || !std::isfinite(y)) return false;
  return std::fabs(x - y) <= EPS * std::fabs(x);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool rEQ_sym(T a, U b) {
  C x = C(a), y = C(b);
  if (x == y) return true;
  if constexpr (!std::is_floating_point_v<C>) {
    return false;
  }
  if (!std::isfinite(x) || !std::isfinite(y)) return false;
  return std::fabs(x - y) <= EPS * std::max(std::fabs(x), std::fabs(y));
}

template<typename T>
int sgn(const T &x) {
  return (T{0} < x) - (x < T{0});
}

template<typename Dbl>
bool sign_bit(Dbl x) {
  return (((unsigned char *)&x)[sizeof(x) - 1] >> (CHAR_BIT - 1)) & 1;
}

template<typename Dbl>
Dbl copy_sign(Dbl x, Dbl y) {
  return sign_bit(y) ? -std::fabs(x) : std::fabs(x);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(EQ(M_PI, 3.14159265359));
  assert(EQ(M_E, 2.718281828459) && EQ(M_PHI, 1.61803398875));
  assert(EQ(M_INF, M_INF) && rEQ(M_INF, M_INF) && rEQ_sym(M_INF, M_INF));
  assert(!rEQ(M_INF, 0.0) && !rEQ_sym(M_INF, -M_INF));
  assert(!rEQ(1000000000000LL, 1000000000001LL));

  double x = -12345.6789;
  assert((-M_INF < x) && (x < M_INF));
  assert((M_INF + x == M_INF) && (M_INF - x == M_INF));
  assert((M_NAN != x) && (M_NAN != M_INF) && (M_NAN != M_NAN));
  assert(!(M_NAN < x) && !(M_NAN > x) && !(M_NAN <= x) && !(M_NAN >= x));
  assert(isnan(0.0 * M_INF) && isnan(M_INF - M_INF));

  assert(sgn(x) == -1 && sgn(0.0) == 0 && sgn(5678) == 1);
  assert(sign_bit(x) && !sign_bit(0.0) && sign_bit(-0.0));
  assert(!sign_bit(M_INF) && sign_bit(-M_INF));
  assert(!sign_bit(M_NAN) && sign_bit(-M_NAN));
  assert(copy_sign(1.0, +2.0) == +1.0 && copy_sign(M_INF, -2.0) == -M_INF);
  assert(copy_sign(1.0, -2.0) == -1.0 && sign_bit(copy_sign(M_NAN, -2.0)));
  return 0;
}
