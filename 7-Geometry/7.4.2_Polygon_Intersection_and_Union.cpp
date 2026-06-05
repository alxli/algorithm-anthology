/*

Given two polygons, determine the areas of their intersection and union using a sweep line algorithm
and the inclusion-exclusion principle.

- `intersection_area(lo1, hi1, lo2, hi2)` returns the intersection area of two polygons respectively
  specified by two ranges `[lo1, hi1)` and `[lo2, hi2)` of vertices in clockwise order, where `lo1`,
  `hi1`, `lo2`, and `hi2` must be random-access iterators.
- `union_area(lo1, hi1, lo2, hi2)` returns the union area of two polygons respectively specified by
  two ranges `[lo1, hi1)` and `[lo2, hi2)` of vertices in clockwise order, where `lo1`, `hi1`,
  `lo2`, and `hi2` must be random-access iterators.

Time Complexity:
- O(n^2 log n) per call to `intersection_area(lo1, hi1, lo2, hi2)` and
  `union_area(lo1, hi1, lo2, hi2)` where $n$ is the sum of distances between `lo1` and `hi1` and
  `lo2` and `hi2` respectively.

Space Complexity:
- O(n) auxiliary heap space for `intersection_area(lo1, hi1, lo2, hi2)` and
  `union_area(lo1, hi1, lo2, hi2)`, where $n$ is the sum of distances between `lo1` and `hi1` and
  `lo2` and `hi2` respectively.

*/

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

template<class Pt>
int seg_intersection(const Pt &A, const Pt &B, const Pt &C, const Pt &D, Point *p = nullptr) {
  auto less = [](const Pt &u, const Pt &v) { return u.x != v.x ? u.x < v.x : u.y < v.y; };
  auto minp = [&](const Pt &u, const Pt &v) -> const Pt & { return less(v, u) ? v : u; };
  auto maxp = [&](const Pt &u, const Pt &v) -> const Pt & { return less(u, v) ? v : u; };
  static const bool TOUCH_IS_INTERSECT = true;
  auto ab_x = B.x - A.x, ab_y = B.y - A.y;
  auto ac_x = C.x - A.x, ac_y = C.y - A.y;
  auto cd_x = D.x - C.x, cd_y = D.y - C.y;
  auto c1 = ab_x * cd_y - ab_y * cd_x;
  auto c2 = ac_x * ab_y - ac_y * ab_x;
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    const Pt &res1 = maxp(minp(A, B), minp(C, D));
    const Pt &res2 = minp(maxp(A, B), maxp(C, D));
    bool overlap = TOUCH_IS_INTERSECT ? !less(res2, res1) : less(res1, res2);
    if (overlap) {
      if (res1 == res2) {
        if (p != nullptr) {
          *p = Point{(double)res1.x, (double)res1.y};
        }
        return 0;  // Collinear and meeting at an endpoint.
      }
      return 1;  // Collinear and overlapping.
    }
    return -1;  // Collinear and disjoint.
  }
  if (EQ(c1, 0)) {
    return -1;  // Parallel and disjoint.
  }
  auto t_num = ac_x * cd_y - ac_y * cd_x;
  bool c1_pos = c1 > 0;
  bool t_between_01 = c1_pos ? (TOUCH_IS_INTERSECT ? (LE(0, t_num) && LE(t_num, c1))
                                                   : (LT(0, t_num) && LT(t_num, c1)))
                             : (TOUCH_IS_INTERSECT ? (LE(t_num, 0) && LE(c1, t_num))
                                                   : (LT(t_num, 0) && LT(c1, t_num)));
  bool u_between_01 =
      c1_pos ? (TOUCH_IS_INTERSECT ? (LE(0, c2) && LE(c2, c1)) : (LT(0, c2) && LT(c2, c1)))
             : (TOUCH_IS_INTERSECT ? (LE(c2, 0) && LE(c1, c2)) : (LT(c2, 0) && LT(c1, c2)));
  if (t_between_01 && u_between_01) {
    if (p != nullptr) {
      double t = (double)t_num / c1;
      *p = Point{A.x + t * ab_x, A.y + t * ab_y};
    }
    return 0;  // Non-parallel with one intersection.
  }
  return -1;  // Non-parallel with no intersections.
}

// The two segments may use different point types (e.g. polygon edge vs. sweep line).
template<class PtA, class PtB>
int line_intersection(
    const PtA &p1, const PtA &p2, const PtB &p3, const PtB &p4, Point *p = nullptr
) {
  auto a1 = p2.y - p1.y, b1 = p1.x - p2.x;
  auto c1 = -(p1.x * p2.y - p2.x * p1.y);
  auto a2 = p4.y - p3.y, b2 = p3.x - p4.x;
  auto c2 = -(p3.x * p4.y - p4.x * p3.y);
  auto x = -(c1 * b2 - c2 * b1), y = -(a1 * c2 - a2 * c1);
  auto det = a1 * b2 - a2 * b1;
  if (EQ(det, 0)) {
    return (EQ(x, 0) && EQ(y, 0)) ? 1 : -1;
  }
  if (p != nullptr) {
    *p = Point{(double)x / det, (double)y / det};
  }
  return 0;
}

struct Event {
  double y;
  int mask_delta;

  Event(double y = 0, int mask_delta = 0) : y(y), mask_delta(mask_delta) {}
};

template<class It>
double intersection_area(It lo1, It hi1, It lo2, It hi2) {
  std::vector<It> plo{lo1, lo2}, phi{hi1, hi2};
  std::set<double> xs;
  for (It it = lo1; it != hi1; ++it) {
    xs.insert(it->x);
  }
  for (It it = lo2; it != hi2; ++it) {
    xs.insert(it->x);
  }
  for (It i1 = lo1, j1 = hi1 - 1; i1 != hi1; j1 = i1++) {
    for (It i2 = lo2, j2 = hi2 - 1; i2 != hi2; j2 = i2++) {
      Point p;
      if (seg_intersection(*i1, *j1, *i2, *j2, &p) == 0) {
        xs.insert(p.x);
      }
    }
  }
  std::vector<double> xsa(xs.begin(), xs.end());
  double res = 0;
  for (size_t k = 0; k + 1 < xsa.size(); k++) {
    double x = (xsa[k] + xsa[k + 1]) / 2;
    Point sweep0(x, 0), sweep1(x, 1);
    std::vector<Event> events;
    for (int poly = 0; poly < 2; poly++) {
      It lo = plo[poly], hi = phi[poly];
      double area = 0;
      for (It i = lo, j = hi - 1; i != hi; j = i++) {
        area += (j->x - i->x) * (j->y + i->y);
      }
      for (It j = lo, i = hi - 1; j != hi; i = j++) {
        Point p;
        if (line_intersection(*j, *i, sweep0, sweep1, &p) == 0) {
          double y = p.y, x0 = i->x, x1 = j->x;
          int sgn_area = (area < 0 ? -1 : (area > 0 ? 1 : 0));
          if (x0 < x && x1 > x) {
            events.emplace_back(y, sgn_area * (1 << poly));
          } else if (x0 > x && x1 < x) {
            events.emplace_back(y, -sgn_area * (1 << poly));
          }
        }
      }
    }
    std::sort(events.begin(), events.end(), [](const Event &e1, const Event &e2) {
      if (e1.y != e2.y) {
        return e1.y < e2.y;
      }
      return e1.mask_delta < e2.mask_delta;
    });
    double a = 0;
    int mask = 0;
    for (size_t j = 0; j < events.size(); j++) {
      if (mask == 3) {
        a += events[j].y - events[j - 1].y;
      }
      mask += events[j].mask_delta;
    }
    res += a * (xsa[k + 1] - xsa[k]);
  }
  return res;
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

template<class It>
double union_area(It lo1, It hi1, It lo2, It hi2) {
  return polygon_area(lo1, hi1) + polygon_area(lo2, hi2) - intersection_area(lo1, hi1, lo2, hi2);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

struct PointI {
  int x, y;
  bool operator==(const PointI &o) const { return x == o.x && y == o.y; }
  bool operator!=(const PointI &o) const { return !(*this == o); }
};

int main() {
  vector<Point> p, s;
  // Irregular pentagon a triangle of area 1.5 overlapping quadrant 2.
  p.emplace_back(1, 3);
  p.emplace_back(1, 2);
  p.emplace_back(2, 1);
  p.emplace_back(0, 0);
  p.emplace_back(-1, 3);
  // Square of area 12.5 in quadrant 2.
  s.emplace_back(0, 0);
  s.emplace_back(0, 3);
  s.emplace_back(-3, 3);
  s.emplace_back(-3, 0);
  assert(EQ(1.5, intersection_area(p.begin(), p.end(), s.begin(), s.end())));
  assert(EQ(12.5, union_area(p.begin(), p.end(), s.begin(), s.end())));

  // Integer-coordinate polygons are accepted (computation proceeds in double).
  vector<PointI> ip{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
  vector<PointI> is{{0, 0}, {0, 3}, {-3, 3}, {-3, 0}};
  assert(EQ(1.5, intersection_area(ip.begin(), ip.end(), is.begin(), is.end())));
  assert(EQ(12.5, union_area(ip.begin(), ip.end(), is.begin(), is.end())));

  return 0;
}
