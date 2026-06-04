/*

Given a list of distinct points in two-dimensions, order them into a valid polygon and determine the
area.

- `mean_center(lo, hi)` returns the arithmetic mean of a range `[lo, hi)` of points, where `lo` and
  `hi` must be random-access iterators. This point is mathematically guaranteed to lie within the
  non-self-intersecting closed polygon constructed by sorting all other points clockwise about it.
  Note that this is different from the geometric centroid (a.k.a. barycenter) of a polygon.
- `cw_comp(a, b, c)` returns whether point $a$ compares clockwise "before" point $b$ when using $c$
  as a central reference point.
- `CWComparator(c)` constructs a wrapper class of `cw_comp()` that may be passed to `std::sort()` a
  range of points clockwise to produce a valid polygon.
- `CCWComparator(c)` constructs a wrapper class of `cw_comp()` that may be passed to `std::sort()`
  a range of points counter-clockwise to produce a valid polygon.
- `polygon_area(lo, hi)` returns the area of the polygon specified by the range `[lo, hi)` of
  points, where `lo` and `hi` must be BidirectionalIterators. The points are interpreted as a
  polygon based on the order given in the range. The input polygon does not have to be sorted using
  the methods above, but must be given in some ordering that yields a valid non-self-intersecting
  closed polygon. Optionally, the last point may be equal to the first point in the input without
  affecting the result. The area is computed using the shoelace formula.

Time Complexity:
- O(n) per call to `mean_center(lo, hi)` and `polygon_area(lo, hi)`, where $n$ is the distance
  between `lo` and `hi`.
- O(1) per call to `cw_comp()` and the related class comparators.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define GE(a, b) ((a) >= (b) - EPS)

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

double sqnorm(const Point &a) {
  return a.x * a.x + a.y * a.y;
}

double cross(const Point &a, const Point &b) {
  return a.x * b.y - a.y * b.x;
}

template<class It>
Point mean_center(It lo, It hi) {
  if (lo == hi) {
    throw std::runtime_error("Cannot get center of an empty range.");
  }
  double x_sum = 0, y_sum = 0;
  double num_points = hi - lo;
  for (It it = lo; it != hi; ++it) {
    x_sum += it->x;
    y_sum += it->y;
  }
  return Point(x_sum / num_points, y_sum / num_points);
}

bool cw_comp(const Point &a, const Point &b, const Point &c) {
  if (GE(a.x - c.x, 0) && LT(b.x - c.x, 0)) {
    return true;
  }
  if (LT(a.x - c.x, 0) && GE(b.x - c.x, 0)) {
    return false;
  }
  if (EQ(a.x - c.x, 0) && EQ(b.x - c.x, 0)) {
    if (GE(a.y - c.y, 0) || GE(b.y - c.y, 0)) {
      return a.y > b.y;
    }
    return b.y > a.y;
  }
  Point ac(a.x - c.x, a.y - c.y), bc(b.x - c.x, b.y - c.y);
  double det = cross(ac, bc);
  if (EQ(det, 0)) {
    return sqnorm(ac) > sqnorm(bc);
  }
  return det < 0;
}

auto CWComparator(const Point &c) {
  return [c](const Point &a, const Point &b) { return cw_comp(a, b, c); };
}

auto CCWComparator(const Point &c) {
  return [c](const Point &a, const Point &b) { return cw_comp(b, a, c); };
}

template<class It>
double polygon_area(It lo, It hi) {
  if (lo == hi) {
    return 0;
  }
  double area = 0;
  if (*lo != *--hi) {
    area += (lo->x - hi->x) * (lo->y + hi->y);
  }
  for (It i = hi, j = --hi; i != lo; --i, --j) {
    area += (i->x - j->x) * (i->y + j->y);
  }
  return fabs(area / 2.0);
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  // Irregular pentagon with only the vertex (1, 2) not on its convex hull. The ordering here is
  // already sorted in ccw order around their mean center, though we will shuffle them to verify our
  // sorting comparator.
  vector<Point> points{Point(1, 3), Point(1, 2), Point(2, 1), Point(0, 0), Point(-1, 3)};
  vector<Point> v(points);
  std::mt19937 rng(12345);
  std::shuffle(v.begin(), v.end(), rng);
  Point c = mean_center(v.begin(), v.end());
  assert(EQ(c.x, 0.6) && EQ(c.y, 1.8));
  sort(v.begin(), v.end(), CWComparator(c));
  auto expected = points.begin();
  for (const auto &p : v) {
    assert(p == *expected++);
  }
  assert(EQ(polygon_area(v.begin(), v.end()), 5));
  return 0;
}
