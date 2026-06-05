/*

Given a list of distinct points in two-dimensions, order them into a valid polygon and determine the
area.

The functions below are templated on the point type. The local `Point` struct (`double` coordinates)
is the default; replace it with `Point`/`PointD`/ `PointI` from 7.1.1 or any struct with numeric
`.x` and `.y` fields and `<` / `==` operators.

`polygon_area_2x` returns twice the area as the coordinate type - exact for integer points (avoids
floating-point entirely). `polygon_area` always returns `double`.

- `mean_center(lo, hi)` returns the arithmetic mean of a range of points (as the local `Point` type
  with `double` coordinates).
- `cw_comp(a, b, c)` returns whether point `a` compares clockwise before `b` about `c`.
- `CWComparator(c)` / `CCWComparator(c)` return comparators for `std::sort`.
- `polygon_area_2x(lo, hi)` returns exactly double the area of the polygon with vertices specified
  by the input range `[lo, hi)`. The return value is integral or floating-point, depending on the
  input point type). For integer vertices, divide by 2 in the caller if the exact area is needed.
- `polygon_area(lo, hi)` returns the area as `double`.

Time Complexity:
- O(n) per call to `mean_center` and `polygon_area`, where $n$ is the number of points.
- O(1) per call to `cw_comp` and the comparators.

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

template<class Pt>
auto sqnorm(const Pt &a) {
  return a.x * a.x + a.y * a.y;
}

template<class Pt>
auto cross(const Pt &a, const Pt &b) {
  return a.x * b.y - a.y * b.x;
}

template<class Pt>
bool cw_comp(const Pt &a, const Pt &b, const Pt &c) {
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
  Pt ac{a.x - c.x, a.y - c.y};
  Pt bc{b.x - c.x, b.y - c.y};
  auto det = cross(ac, bc);
  if (EQ(det, 0)) {
    return sqnorm(ac) > sqnorm(bc);
  }
  return det < 0;
}

// Returns 2 * area. Result is exact (integer) for integer-coordinate points.
template<class It>
auto polygon_area_2x(It lo, It hi) {
  using T = decltype(lo->x * lo->y);
  if (lo == hi) {
    return T(0);
  }
  T area = 0;
  if (*lo != *--hi) {
    area += (T)(lo->x - hi->x) * (T)(lo->y + hi->y);
  }
  for (It i = hi, j = --hi; i != lo; --i, --j) {
    area += (T)(i->x - j->x) * (T)(i->y + j->y);
  }
  return area < T(0) ? -area : area;
}

template<class It>
double polygon_area(It lo, It hi) {
  return (double)polygon_area_2x(lo, hi) / 2.0;
}

/*** Example Usage and Output:

Clockwise vertices: (1, 3) (1, 2) (2, 1) (0, 0) (-1, 3) 
Counterclockwise vertices: (-1, 3) (0, 0) (2, 1) (1, 2) (1, 3) 

***/

#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

struct PointI {
  int x, y;
  bool operator==(const PointI &p) const { return x == p.x && y == p.y; }
  bool operator!=(const PointI &p) const { return !(*this == p); }
  bool operator<(const PointI &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const PointI &p) const { return p < *this; }
};

template<class It>
Point mean_center(It lo, It hi) {
  if (lo == hi) {
    throw std::runtime_error("Cannot get center of an empty range.");
  }
  double x_sum = 0, y_sum = 0, n = hi - lo;
  for (It it = lo; it != hi; ++it) {
    x_sum += it->x;
    y_sum += it->y;
  }
  return Point(x_sum / n, y_sum / n);
}

int main() {
  vector<Point> points{Point(1, 3), Point(1, 2), Point(2, 1), Point(0, 0), Point(-1, 3)};
  vector<Point> v(points);
  std::mt19937 rng(12345);
  std::shuffle(v.begin(), v.end(), rng);
  Point c = mean_center(v.begin(), v.end());
  assert(EQ(c.x, 0.6) && EQ(c.y, 1.8));
  sort(v.begin(), v.end(), [c](const Point &a, const Point &b) { return cw_comp(a, b, c); });
  cout << "Clockwise vertices: ";
  for (const auto &p : v) {
    cout << "(" << p.x << ", " << p.y << ") ";
  }
  cout << endl;
  assert(EQ(polygon_area(v.begin(), v.end()), 5));

  sort(v.begin(), v.end(), [c](const Point &a, const Point &b) { return cw_comp(b, a, c); });
  cout << "Counterclockwise vertices: ";
  for (const auto &p : v) {
    cout << "(" << p.x << ", " << p.y << ") ";
  }
  cout << endl;
  assert(EQ(polygon_area(v.begin(), v.end()), 5));

  // Integer points: polygon_area_2x is exact (no float arithmetic).
  vector<PointI> iv{{0, 0}, {4, 0}, {0, 3}};            // right triangle, area = 6
  assert(polygon_area_2x(iv.begin(), iv.end()) == 12);  // exact int
  assert(EQ(polygon_area(iv.begin(), iv.end()), 6.0));

  return 0;
}
