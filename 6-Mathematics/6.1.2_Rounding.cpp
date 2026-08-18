/*

Rounding is only ambiguous at a tie, when a value sits exactly halfway between two integers, and the
rule chosen for that case is what distinguishes the functions below. The choice matters more than it
appears: always rounding ties away from zero biases a sum of many rounded values upward, which is
why financial and statistical work usually rounds ties to even instead, spreading them evenly
between the two neighbors.

- `floor0(x)` returns `x` rounded down, symmetrically towards zero. This function is analogous to
  `std::trunc()`.
- `ceil0(x)` returns `x` rounded up, symmetrically away from zero. It mirrors `floor0()` and has no
  `<cmath>` equivalent, since `std::ceil()` rounds towards positive infinity rather than outward.
- `round_half_up(x)` returns `x` rounded half up, towards positive infinity.
- `round_half_down(x)` returns `x` rounded half down, towards negative infinity.
- `round_half_to0(x)` returns `x` rounded half down, symmetrically towards zero.
- `round_half_from0(x)` returns `x` rounded half up, symmetrically away from zero. This function is
  analogous to `std::round()`.
- `round_half_even(x, eps = 1e-9)` returns `x` rounded half to even, using `eps` to detect ties for
  banker's rounding.
- `round_half_alternate(x)` returns `x` rounded, where ties are broken by alternating rounds towards
  positive and negative infinity.
- `round_half_alternate0(x)` returns `x` rounded, where ties are broken by alternating symmetric
  rounds towards and away from zero.
- `round_half_random(x)` returns `x` rounded, where ties are broken randomly.
- `round_n_places(x, n, round)` returns `x` rounded to `n` digits after the decimal, using the
  specified rounding function `round(x)`.

Every function propagates `+0.0`, `-0.0`, `Inf`, `-Inf`, `NaN`, and `-NaN` exactly with the same
sign, as `std::round()` does. The four built on adding or subtracting $0.5$ before flooring are
otherwise exact except at $0.5 - 2^{-54}$, which rounds up to $1$ because the sum is already $1$
before flooring; prefer `std::round()` or `round_half_even()` where that last bit matters.

Time Complexity:
- O(1) per call to all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <chrono>
#include <cmath>
#include <random>

template<typename Dbl>
Dbl floor0(const Dbl &x) {
  Dbl res = std::floor(std::fabs(x));
  return std::signbit(x) ? -res : res;
}

template<typename Dbl>
Dbl ceil0(const Dbl &x) {
  Dbl res = std::ceil(std::fabs(x));
  return std::signbit(x) ? -res : res;
}

template<typename Dbl>
Dbl round_half_up(const Dbl &x) {
  return std::copysign(std::floor(x + 0.5), x);  // copysign() only ever fixes a zero result.
}

template<typename Dbl>
Dbl round_half_down(const Dbl &x) {
  return std::copysign(std::ceil(x - 0.5), x);  // copysign() only ever fixes a zero result.
}

template<typename Dbl>
Dbl round_half_to0(const Dbl &x) {
  Dbl res = round_half_down(std::fabs(x));
  return std::signbit(x) ? -res : res;
}

template<typename Dbl>
Dbl round_half_from0(const Dbl &x) {
  Dbl res = round_half_up(std::fabs(x));
  return std::signbit(x) ? -res : res;
}

template<typename Dbl>
Dbl round_half_even(const Dbl &x, const Dbl &eps = 1e-9) {
  if (std::signbit(x)) {
    return -round_half_even(-x, eps);
  }
  Dbl ipart;
  std::modf(x, &ipart);
  if (std::fabs(x - (ipart + 0.5)) < eps) {  // exactly halfway: break the tie towards even
    return (std::fmod(ipart, 2.0) < eps) ? ipart : ceil0(ipart + 0.5);
  }
  return round_half_from0(x);
}

template<typename Dbl>
Dbl round_half_alternate(const Dbl &x) {
  Dbl up = round_half_up(x), down = round_half_down(x);
  if (std::isnan(x) || up == down) {  // A NaN must not consume a toggle: NaN == NaN is false.
    return up;
  }
  static bool round_up = false;
  return (round_up = !round_up) ? up : down;
}

template<typename Dbl>
Dbl round_half_alternate0(const Dbl &x) {
  Dbl away = round_half_from0(x), toward = round_half_to0(x);
  if (std::isnan(x) || away == toward) {  // A NaN must not consume a toggle.
    return away;
  }
  static bool round_away = false;
  return (round_away = !round_away) ? away : toward;
}

template<typename Dbl>
Dbl round_half_random(const Dbl &x) {
  Dbl away = round_half_from0(x), toward = round_half_to0(x);
  if (std::isnan(x) || away == toward) {
    return away;
  }
  static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  return (rng() % 2 == 0) ? away : toward;
}

template<typename Dbl, typename RoundFn>
Dbl round_n_places(const Dbl &x, unsigned int n, RoundFn round) {
  Dbl scale = std::pow(Dbl{10}, n);
  return round(x * scale) / scale;
}

/*** Example Usage ***/

#include <cassert>
#include <limits>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  assert(EQ(floor0(1.5), 1.0) && EQ(ceil0(1.5), 2.0));
  assert(EQ(floor0(-1.5), -1.0) && EQ(ceil0(-1.5), -2.0));
  // Both round outward at any fraction, unlike std::round() which rounds to the nearest.
  assert(EQ(floor0(1.8), 1.0) && EQ(ceil0(1.2), 2.0) && EQ(ceil0(-1.2), -2.0));

  // NaN, the infinities, and the sign of zero are all preserved, as std::round() does.
  const double M_INF = numeric_limits<double>::infinity();
  const double M_NAN = numeric_limits<double>::quiet_NaN();
  assert(isnan(round_half_even(M_NAN)) && isinf(ceil0(-M_INF)));
  assert(signbit(round_half_from0(-0.0)) && !signbit(round_half_down(0.0)));
  assert(EQ(round_half_up(+1.5), +2) && EQ(round_half_down(+1.5), +1));
  assert(EQ(round_half_up(-1.5), -1) && EQ(round_half_down(-1.5), -2));
  assert(EQ(round_half_to0(+1.5), +1) && EQ(round_half_from0(+1.5), +2));
  assert(EQ(round_half_to0(-1.5), -1) && EQ(round_half_from0(-1.5), -2));
  assert(EQ(round_half_even(+1.5), +2) && EQ(round_half_even(-1.5), -2));
  assert(EQ(round_half_even(3.1), 3) && EQ(round_half_even(3.4), 3));  // Non-ties round normally.

  double alt1 = round_half_alternate(+1.5);
  assert(EQ(round_half_alternate(+1.2), +1));  // Non-ties do not consume an alternating turn.
  double alt2 = round_half_alternate(+1.5);
  assert(!EQ(alt1, alt2));
  double alt01 = round_half_alternate0(-1.5);
  assert(EQ(round_half_alternate0(-1.2), -1));
  double alt02 = round_half_alternate0(-1.5);
  assert(!EQ(alt01, alt02));

  assert(EQ(round_n_places(-1.23456, 3, round_half_to0<double>), -1.235));
  return 0;
}
