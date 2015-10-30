/*

1.3.3 - Hill Climbing

Hill climbing is an optimization technique which finds the
local extrema of a function. It is an iterative algorithm
that starts with an arbitrary solution, then attempts to
find a better solution by incrementally changing a single
element of the solution. If the change produces a better
solution, an incremental change is made to the new solution,
repeating until no further improvements can be found.

Complexity: O(log SIZE) where SIZE is the search space.

*/

#include <cmath>
#include <iostream>
using namespace std;

template<class BinaryFunction>
double find_minimum(BinaryFunction f) {
  static const double PI = acos(-1.0);
  double x = 0, y = 0, res = f(x, y);
  for (double step = 1e6; step > 1e-7; ) {
    double best = res, bestx = x, besty = y;
    bool found = false;
    for (int i = 0; i < 6; i++) {
      double a = 2.0 * PI * i / 6.0;
      double nextx = x + step * cos(a);
      double nexty = y + step * sin(a);
      double next = f(nextx, nexty);
      if (best > next) {
        bestx = nextx;
        besty = nexty;
        best = next;
        found = true;
      }
    }
    if (!found) {
      step /= 2.0;
    } else {
      x = bestx;
      y = besty;
      res = best;
    }
  }
  cout << fixed << x << " " << y << "\n";
  return res;
}

/*** Example Usage ***/

//minimized at f(2, 3) = 0
double f(double x, double y) {
  return (x - 2)*(x - 2) + (y - 3)*(y - 3);
}

int main() {
  cout.precision(1);
  cout << fixed << find_minimum(f) << "\n";
  return 0;
}
