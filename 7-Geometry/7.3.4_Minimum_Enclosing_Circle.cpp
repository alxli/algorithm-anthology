/*

Given a list of points in two dimensions, finds the circle with smallest area containing all points
using a randomized algorithm.

- `minimum_enclosing_circle(lo, hi)` returns the minimum enclosing circle given a range `[lo, hi)`
  of points. The input range is shuffled after the call. The function is templated on the iterator
  type and works with any point type `Pt` having numeric `.x` and `.y` fields. The output is always
  a `Circle` with `double` coordinates, so integral inputs are accepted and converted internally.

Time Complexity:
- O(n) on average, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(1) auxiliary.

*/

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LE(a, b) ((a) <= (b) + EPS)
#define LT(a, b) ((a) < (b) - EPS)

double sqnorm(double x, double y) {
  return x * x + y * y;
}

// SFINAE guard: valid only when Pt exposes numeric .x/.y members. This keeps the templated point
// constructors from hijacking calls like Circle(double, double, double).
template<class Pt>
using if_point = decltype(std::declval<const Pt &>().x, std::declval<const Pt &>().y, void());

struct Circle {
  double h, k, r;

  Circle() : h(0), k(0), r(0) {}
  Circle(double h, double k, double r) : h(h), k(k), r(fabs(r)) {}

  template<class Pt, class = if_point<Pt>>
  Circle(const Pt &a, const Pt &b) {
    h = (a.x + b.x) / 2.0;
    k = (a.y + b.y) / 2.0;
    r = std::hypot(a.x - h, a.y - k);
  }

  template<class Pt, class = if_point<Pt>>
  Circle(const Pt &a, const Pt &b, const Pt &c) {
    double an = sqnorm(b.x - c.x, b.y - c.y);
    double bn = sqnorm(a.x - c.x, a.y - c.y);
    double cn = sqnorm(a.x - b.x, a.y - b.y);
    double wa = an * (bn + cn - an), wb = bn * (an + cn - bn), wc = cn * (an + bn - cn);
    double w = wa + wb + wc;
    if (EQ(w, 0)) {
      throw std::runtime_error("No circumcircle from collinear points.");
    }
    h = (wa * a.x + wb * b.x + wc * c.x) / w;
    k = (wa * a.y + wb * b.y + wc * c.y) / w;
    r = std::hypot(a.x - h, a.y - k);
  }

  template<class Pt, class = if_point<Pt>>
  bool contains(const Pt &p) const {
    return LE(sqnorm(p.x - h, p.y - k), r * r);
  }
};

// Input points can be any type with .x and .y fields; Circle output is always double.
template<class It>
Circle minimum_enclosing_circle(It lo, It hi) {
  if (lo == hi) {
    return Circle(0, 0, 0);
  }
  if (lo + 1 == hi) {
    return Circle(lo->x, lo->y, 0);
  }
  static std::mt19937 rng(std::random_device{}());
  std::shuffle(lo, hi, rng);
  Circle res(*lo, *(lo + 1));
  for (It i = lo + 2; i != hi; ++i) {
    if (res.contains(*i)) {
      continue;
    }
    res = Circle(*lo, *i);
    for (It j = lo + 1; j != i; ++j) {
      if (res.contains(*j)) {
        continue;
      }
      res = Circle(*i, *j);
      for (It k = lo; k != j; ++k) {
        if (!res.contains(*k)) {
          res = Circle(*i, *j, *k);
        }
      }
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
};

int main() {
  vector<Point> v{{0, 0}, {0, 1}, {1, 0}, {1, 1}};
  Circle res = minimum_enclosing_circle(v.begin(), v.end());
  assert(EQ(res.h, 0.5) && EQ(res.k, 0.5) && EQ(res.r, 1 / sqrt(2)));

  // Integer-coordinate input: Circle output is always double.
  vector<PointI> iv{{0, 0}, {0, 2}, {2, 0}, {2, 2}};
  Circle ic = minimum_enclosing_circle(iv.begin(), iv.end());
  assert(EQ(ic.h, 1.0) && EQ(ic.k, 1.0));
  return 0;
}
