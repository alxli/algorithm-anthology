/*

Given a continuous function f(x, y) to double and a (possibly arbitrary) initial
guess (x0, y0), return a potential global minimum found through hill-climbing.
Optionally, two double pointers critical_x and critical_y may be passed to store
the input points to f at which the returned minimum value is attained.

Hill-climbing is a heuristic which starts at the guess, then considers taking
a single step in each of a fixed number of directions. The direction with the
best (in this case, minimum) value is chosen, and further steps are taken in it
until the answer no longer improves. When this happens, the step size is reduced
and the same process repeats until a desired absolute error is reached. The
technique's success heavily depends on the behavior of f and the initial guess.
Therefore, the result is not guaranteed to be the global minimum.

Time Complexity:
- O(d log n) call will be made to f, where d is the number of directions
  considered at each position and n is the search space that is approximately
  proportional to the maximum possible step size divided by the minimum possible
  step size.

Space Complexity:
- O(1) auxiliary.

*/

#include <cstddef>
#include <cmath>

template<class ContinuousFunction>
double find_min(ContinuousFunction f, double x0, double y0,
                double *critical_x = NULL, double *critical_y = NULL,
                const double STEP_MIN = 1e-9, const double STEP_MAX = 1e6,
                const int NUM_DIRECTIONS = 6) {
  static const double PI = acos(-1.0);
  double x = x0, y = y0, res = f(x0, y0);
  for (double step = STEP_MAX; step > STEP_MIN; ) {
    double best = res, best_x = x, best_y = y;
    bool found = false;
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
      double a = 2.0*PI*i / NUM_DIRECTIONS;
      double x2 = x + step*cos(a), y2 = y + step*sin(a);
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
  if (critical_x != NULL && critical_y != NULL) {
    *critical_x = x;
    *critical_y = y;
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <cmath>

bool eq(double a, double b) {
  return fabs(a - b) < 1e-8;
}

// Paraboloid with global minimum at f(2, 3) = 0.
double f(double x, double y) {
  return (x - 2)*(x - 2) + (y - 3)*(y - 3);
}

int main() {
  double x, y;
  assert(eq(find_min(f, 0, 0, &x, &y), 0));
  assert(eq(x, 2) && eq(y, 3));
  return 0;
}
