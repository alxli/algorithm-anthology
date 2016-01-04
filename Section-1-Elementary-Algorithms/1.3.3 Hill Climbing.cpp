/*

Given a continuous function f on two real numbers, hill climbing is a
technique that can be used to find the local maximum or minimum point
based on some (possibly random) initial guess. Then, the algorithm
considers taking a single step in each of a fixed number of directions.
The direction with the best result is selected and steps are further
taken there until the answer no longer improves. When this happens, the
step size is reduced and the process repeats until a desired absolute
error is reached. The result is not necessarily the global extrema, and
the algorithm's success will heavily depend on the initial guess.

The following function find_min() takes the function f, any starting
guess (x0, y0), and optionally two pointers to double used for storing
the answer coordinates. find_min() returns a local minimum point near
the initial guess, and if the two pointers are given, then coordinates
will be stored into the variables pointed to by x_ans and y_ans.

Time Complexity: At most O(d log n) calls to f, where d is the number
of directions considered at each position and n is the search space,
roughly proportional to the largest possible step size divided by the
smallest possible step size.

*/

#include <cmath>
#include <iostream>
using namespace std;

template<class BinaryFunction>
double find_min(BinaryFunction f, double x0, double y0,
                double *x_ans = 0, double *y_ans = 0) {
  static const double PI = acos(-1.0);
  static const double STEP_MAX = 1000000;
  static const double STEP_MIN = 1e-9;
  static const int DIRECTIONS = 6;
  double x = x0, y = y0, res = f(x0, y0);
  for (double step = STEP_MAX; step > STEP_MIN; ) {
    double best = res, best_x = x, best_y = y;
    bool found = false;
    for (int i = 0; i < DIRECTIONS; i++) {
      double a = 2.0 * PI * i / DIRECTIONS;
      double x2 = x + step * cos(a);
      double y2 = y + step * sin(a);
      double val = f(x2, y2);
      if (best > val) {
        best_x = x2;
        best_y = y2;
        best = val;
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
  if (x_ans != 0 && y_ans != 0) {
    *x_ans = x;
    *y_ans = y;
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <cmath>

bool eq(double a, double b) {
  return fabs(a - b) < 1e-8;
}

//minimized at f(2, 3) = 0
double f(double x, double y) {
  return (x - 2)*(x - 2) + (y - 3)*(y - 3);
}

int main() {
  double x, y;
  assert(eq(find_min(f, 0, 0, &x, &y), 0));
  assert(eq(x, 2) && eq(y, 3));
  return 0;
}
