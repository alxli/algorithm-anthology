/*

Given a range of points on the 2D cartesian plane, determine
the equation of the circle with smallest possible area which
encloses all of the points. Note: in an attempt to avoid the
worst case, the circles are randomly shuffled before the
algorithm is performed. This is not necessary to obtain the
correct answer, and may be removed if the input order must
be preserved.

Time Complexity: O(n) average on the number of points given.

*/

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

const double eps = 1e-9;

#define LE(a, b) ((a) <= (b) + eps)       /* less than or equal to */

typedef std::pair<double, double> point;
#define x first
#define y second

double norm(const point & a) { return a.x * a.x + a.y * a.y; }
double abs(const point & a) { return sqrt(norm(a)); }

struct circle {

  double h, k, r;

  circle(): h(0), k(0), r(0) {}
  circle(const double & H, const double & K, const double & R):
    h(H), k(K), r(fabs(R)) {}

  //circumcircle with the diameter equal to the distance from a to b
  circle(const point & a, const point & b) {
    h = (a.x + b.x) / 2.0;
    k = (a.y + b.y) / 2.0;
    r = abs(point(a.x - h, a.y - k));
  }

  //circumcircle of 3 points - throws exception if abc are collinear/equal
  circle(const point & a, const point & b, const point & c) {
    double an = norm(point(b.x - c.x, b.y - c.y));
    double bn = norm(point(a.x - c.x, a.y - c.y));
    double cn = norm(point(a.x - b.x, a.y - b.y));
    double wa = an * (bn + cn - an);
    double wb = bn * (an + cn - bn);
    double wc = cn * (an + bn - cn);
    double w = wa + wb + wc;
    if (fabs(w) < eps)
      throw std::runtime_error("No circle from collinear points.");
    h = (wa * a.x + wb * b.x + wc * c.x) / w;
    k = (wa * a.y + wb * b.y + wc * c.y) / w;
    r = abs(point(a.x - h, a.y - k));
  }

  bool contains(const point & p) const {
    return LE(norm(point(p.x - h, p.y - k)), r * r);
  }

};

template<class It> circle smallest_circle(It lo, It hi) {
  if (lo == hi) return circle(0, 0, 0);
  if (lo + 1 == hi) return circle(lo->x, lo->y, 0);
  std::random_shuffle(lo, hi);
  circle res(*lo, *(lo + 1));
  for (It i = lo + 2; i != hi; ++i) {
    if (res.contains(*i)) continue;
    res = circle(*lo, *i);
    for (It j = lo + 1; j != i; ++j) {
      if (res.contains(*j)) continue;
      res = circle(*i, *j);
      for (It k = lo; k != j; ++k)
        if (!res.contains(*k)) res = circle(*i, *j, *k);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <iostream>
#include <vector>
using namespace std;

int main() {
  vector<point> v;
  v.push_back(point(0, 0));
  v.push_back(point(0, 1));
  v.push_back(point(1, 0));
  v.push_back(point(1, 1));
  circle res = smallest_circle(v.begin(), v.end());
  cout << "center: (" << res.h << ", " << res.k << ")\n";
  cout << "radius: " << res.r << "\n";
  return 0;
}
