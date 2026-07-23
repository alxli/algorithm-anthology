/*

Given a continuous geometric objective $f(x, y)$ and a (possibly arbitrary) starting guess
$(x_0, y_0)$, search for a small value using two-dimensional hill climbing.

The heuristic starts at the guess and evaluates one step in each of eight evenly spaced directions.
It moves to the best improving neighbor, checks all eight directions again, and reduces the step
size when no neighbor improves the answer. This repeats until the step size falls below the chosen
threshold. Success depends heavily on the behavior of $f$ and the initial guess, so the result is
not guaranteed to be the global minimum.

- `hill_climb_min(f, x0, y0, &best_x, &best_y, step_min = 1e-9, step_max = 1e6)` returns an
  approximate minimum value of function `f` reached by hill climbing an initial guess (`x0`, `y0`).
  If either optional pointer `best_x` or `best_y` is supplied, the corresponding coordinate of the
  point attaining the returned value is stored through it. The search starts with step size
  `step_max` and stops below `step_min`.

Time Complexity:
- O(k + s) calls to `f()` per call, where $k$ is the number of times the step size is reduced and
  $s$ is the total number of improving moves accepted.

Space Complexity:
- O(1) auxiliary.

*/

template<typename Fn>
double hill_climb_min(
    Fn f, double x0, double y0, double *best_x = nullptr, double *best_y = nullptr,
    const double step_min = 1e-9, const double step_max = 1e6
) {
  static const double inv_sqrt2 = 0.7071067811865476;
  static const double dx[] = {1, inv_sqrt2, 0, -inv_sqrt2, -1, -inv_sqrt2, 0, inv_sqrt2};
  static const double dy[] = {0, inv_sqrt2, 1, inv_sqrt2, 0, -inv_sqrt2, -1, -inv_sqrt2};
  double x = x0, y = y0, res = f(x0, y0);
  for (double step = step_max; step > step_min;) {
    double next_value = res, next_x = x, next_y = y;
    bool found = false;
    for (int i = 0; i < 8; i++) {
      double x2 = x + step * dx[i], y2 = y + step * dy[i];
      double value = f(x2, y2);
      if (value < next_value) {
        next_x = x2;
        next_y = y2;
        next_value = value;
        found = true;
      }
    }
    if (!found) {
      step /= 2.0;
    } else {
      x = next_x;
      y = next_y;
      res = next_value;
    }
  }
  if (best_x != nullptr) {
    *best_x = x;
  }
  if (best_y != nullptr) {
    *best_y = y;
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
  assert(EQ(hill_climb_min(f, 0, 0, &x, &y), 0));
  assert(EQ(x, 2) && EQ(y, 3));
  return 0;
}
