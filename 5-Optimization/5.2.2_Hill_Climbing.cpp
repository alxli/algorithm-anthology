/*

Given a continuous function $f(x, y)$ returning a real number and a (possibly arbitrary) starting
guess $(x_0, y_0)$, search for a global minimum using the hill-climbing heuristic.

Hill-climbing is a heuristic which starts at the guess, then considers taking a single step in each
of a fixed number of directions. The direction with the best (in this case, minimum) value is
chosen, and further steps are taken in it until the answer no longer improves. When this happens,
the step size is reduced and the same process repeats until a desired absolute error is reached. The
technique's success heavily depends on the behavior of $f$ and the initial guess. Therefore, the
result is not guaranteed to be the global minimum.

- `find_min(f, x0, y0, &critical_x, &critical_y)` returns a candidate global minimum value of
  function `f` reached by hill-climbing from the starting guess (`x0`, `y0`). If the optional
  pointers `critical_x` and `critical_y` are supplied, the point attaining the returned value is
  stored through them. The step-size bounds and number of directions sampled are additional optional
  parameters.

Time Complexity:
- O(d log n) calls will be made to `f()`, where $d$ is the number of directions considered at each
  position and $n$ is the search space that is approximately proportional to the maximum possible
  step size divided by the minimum possible step size.

Space Complexity:
- O(1) auxiliary.

*/

#include <cmath>
#include <cstddef>

template<typename Fn>
double find_min(
    Fn f, double x0, double y0, double *critical_x = nullptr, double *critical_y = nullptr,
    const double STEP_MIN = 1e-9, const double STEP_MAX = 1e6, const int NUM_DIRECTIONS = 6
) {
  static const double PI = acos(-1.0);
  double x = x0, y = y0, res = f(x0, y0);
  for (double step = STEP_MAX; step > STEP_MIN;) {
    double best = res, best_x = x, best_y = y;
    bool found = false;
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
      double a = 2.0 * PI * i / NUM_DIRECTIONS;
      double x2 = x + step * cos(a), y2 = y + step * sin(a);
      double value = f(x2, y2);
      if (best > value) {
        best_x = x2;
        best_y = y2;
        best = value;
        found = true;
      }
    }
    if (!found) {
      step /= 2.0;
    } else {
      x = best_x;
      y = best_y;
      res = best;
    }
  }
  if (critical_x != nullptr && critical_y != nullptr) {
    *critical_x = x;
    *critical_y = y;
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <cmath>

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

// Paraboloid with global minimum at f(2, 3) = 0.
double f(double x, double y) {
  return (x - 2) * (x - 2) + (y - 3) * (y - 3);
}

int main() {
  double x, y;
  assert(EQ(find_min(f, 0, 0, &x, &y), 0));
  assert(EQ(x, 2) && EQ(y, 3));
  return 0;
}
