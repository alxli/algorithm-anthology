/*

2.4 - Hill Climbing

Description: Hill climbing is an optimization technique
which finds the local extrema of a function. It is an
iterative algorithm that starts with an arbitrary solution,
then attempts to find a better solution by incrementally
changing a single element of the solution. If the change
produces a better solution, an incremental change is made
to the new solution, repeating until no further
improvements can be found.

Complexity: O(log(SIZE)) where SIZE is the search space.

*/

#include <cmath>
#include <iostream>
using namespace std;

const double PI = acos(-1.0);

template<class BinaryFunction>
double find_minimum(BinaryFunction f) {
  double curX = 0, curY = 0;
  double curF = f(curX, curY);
  for (double step = 1e6; step > 1e-7; ) {
    double bestF = curF;
    double bestX = curX;
    double bestY = curY;
    bool find = false;
    for (int i = 0; i < 6; i++) {
      double a = 2.0*PI*i / 6.0;
      double nextX = curX + step*cos(a);
      double nextY = curY + step*sin(a);
      double nextF = f(nextX, nextY);
      if (bestF > nextF) {
        bestF = nextF;
        bestX = nextX;
        bestY = nextY;
        find = true;
      }
    }
    if (!find) {
      step /= 2;
    } else {
      curX = bestX;
      curY = bestY;
      curF = bestF;
    }
  }
  cout << fixed << curX << " " << curY << "\n";
  return curF;
}

//minimized at f(2, 3) = 0
double f(double x, double y) {
  return (x - 2)*(x - 2) + (y - 3)*(y - 3);
}

int main() {
  cout.precision(1);
  cout << fixed << find_minimum(f) << "\n";
  return 0;
}
