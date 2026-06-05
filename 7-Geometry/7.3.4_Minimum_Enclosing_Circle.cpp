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

// Replace with PointD/PointI from 7.1.1, or any struct with numeric .x and .y.
struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

double sqnorm(const Point &a) {
  return a.x * a.x + a.y * a.y;
}

double norm(const Point &a) {
  return sqrt(sqnorm(a));
}

struct Circle {
  double h, k, r;

  Circle() : h(0), k(0), r(0) {}
  Circle(double h, double k, double r) : h(h), k(k), r(fabs(r)) {}

  Circle(const Point &a, const Point &b) {
    h = (a.x + b.x) / 2.0;
    k = (a.y + b.y) / 2.0;
    r = norm(Point(a.x - h, a.y - k));
  }

  Circle(const Point &a, const Point &b, const Point &c) {
    double an = sqnorm(Point(b.x - c.x, b.y - c.y));
    double bn = sqnorm(Point(a.x - c.x, a.y - c.y));
    double cn = sqnorm(Point(a.x - b.x, a.y - b.y));
    double wa = an * (bn + cn - an), wb = bn * (an + cn - bn), wc = cn * (an + bn - cn);
    double w = wa + wb + wc;
    if (EQ(w, 0)) {
      throw std::runtime_error("No circumcircle from collinear points.");
    }
    h = (wa * a.x + wb * b.x + wc * c.x) / w;
    k = (wa * a.y + wb * b.y + wc * c.y) / w;
    r = norm(Point(a.x - h, a.y - k));
  }

  bool contains(const Point &p) const { return LE(sqnorm(Point(p.x - h, p.y - k)), r * r); }
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
  // Helper converts any Pt to the local double Point.
  auto pt = [](const auto &p) { return Point((double)p.x, (double)p.y); };
  Circle res(pt(*lo), pt(*(lo + 1)));
  for (It i = lo + 2; i != hi; ++i) {
    if (res.contains(pt(*i))) {
      continue;
    }
    res = Circle(pt(*lo), pt(*i));
    for (It j = lo + 1; j != i; ++j) {
      if (res.contains(pt(*j))) {
        continue;
      }
      res = Circle(pt(*i), pt(*j));
      for (It k = lo; k != j; ++k) {
        if (!res.contains(pt(*k))) {
          res = Circle(pt(*i), pt(*j), pt(*k));
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

struct PointI {
  int x, y;
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
