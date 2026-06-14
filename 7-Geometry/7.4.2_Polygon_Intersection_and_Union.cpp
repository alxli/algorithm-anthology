/*

Given two simple (non-self-intersecting) polygons, determine the areas of their intersection and
union using a sweep line algorithm and the inclusion-exclusion principle. Every vertex and every
pairwise edge intersection contributes its $x$-coordinate as a sweep boundary, so within each
vertical slab between consecutive boundaries the two borders cannot cross, and the slab's overlap
is accumulated exactly from trapezoid areas. The union then follows by inclusion-exclusion as
area(A) + area(B) minus the intersection.

- `intersection_area(lo1, hi1, lo2, hi2)` returns the intersection area of two polygons respectively
  specified by two ranges `[lo1, hi1)` and `[lo2, hi2)` of vertices in clockwise order, where `lo1`,
  `hi1`, `lo2`, and `hi2` must be random-access iterators.
- `union_area(lo1, hi1, lo2, hi2)` returns the union area of two polygons respectively specified by
  two ranges `[lo1, hi1)` and `[lo2, hi2)` of vertices in clockwise order, where `lo1`, `hi1`,
  `lo2`, and `hi2` must be random-access iterators.

Overflow warning: the segment-intersection tests form cross products in the input point's coordinate
type, which grow like the squared coordinate magnitude. For integral point types, use 64-bit
coordinates (e.g. `PointL` from 7.1.1) if necessary.

Time Complexity:
- O(n*m*(n + m)*log(n + m)) per call to `intersection_area(lo1, hi1, lo2, hi2)` and
  `union_area(lo1, hi1, lo2, hi2)`, where $n$ the number of vertices in the first polygon, and $m$
  is the number of points in the second polygon (or equivalently O(N^3 log N) for $N = n + m$).

Space Complexity:
- O(nm) auxiliary heap space for `intersection_area(lo1, hi1, lo2, hi2)` and
  `union_area(lo1, hi1, lo2, hi2)`, where $n$ is the sum of distances between `lo1` and `hi1` and
  `lo2` and `hi2` respectively.

*/

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

template<class Pt>
bool point_on_segment(const Pt &p, const Pt &a, const Pt &b) {
  // Overflow risk for integer Pt: these products are ~O(max_coord^2); use int64_t if necessary.
  return EQ((p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x), 0) &&
         LE(std::min(a.x, b.x), p.x) && LE(p.x, std::max(a.x, b.x)) &&
         LE(std::min(a.y, b.y), p.y) && LE(p.y, std::max(a.y, b.y));
}

// Specialized version of seg_intersection() from 7.2.3, simplified for touch_is_intersect = true,
// returning -1 for no intersection, 0 for one intersection point, 1 for positive length overlap.
template<class Pt>
int seg_intersection1(
    const Pt &a, const Pt &b, const Pt &c, const Pt &d, double *outx, double *outy
) {
  auto ab_x = b.x - a.x, ab_y = b.y - a.y;
  auto ac_x = c.x - a.x, ac_y = c.y - a.y;
  auto cd_x = d.x - c.x, cd_y = d.y - c.y;
  auto ab2 = ab_x * ab_x + ab_y * ab_y;
  auto cd2 = cd_x * cd_x + cd_y * cd_y;
  if (EQ(ab2, 0)) {
    if (point_on_segment(a, c, d)) {
      *outx = a.x;
      *outy = a.y;
      return 0;
    }
    return -1;
  }
  if (EQ(cd2, 0)) {
    if (point_on_segment(c, a, b)) {
      *outx = c.x;
      *outy = c.y;
      return 0;
    }
    return -1;
  }
  auto c1 = ab_x * cd_y - ab_y * cd_x;
  auto c2 = ac_x * ab_y - ac_y * ab_x;
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    Pt res1 = std::max(std::min(a, b), std::min(c, d));
    Pt res2 = std::min(std::max(a, b), std::max(c, d));
    if (!(res2 < res1)) {
      if (res1 == res2) {
        *outx = static_cast<double>(res1.x);
        *outy = static_cast<double>(res1.y);
        return 0;
      }
      return 1;
    }
    return -1;
  }
  if (EQ(c1, 0)) {
    return -1;
  }
  auto t_num = ac_x * cd_y - ac_y * cd_x;
  bool c1_pos = c1 > 0;
  bool t_ok = c1_pos ? (LE(0, t_num) && LE(t_num, c1)) : (LE(c1, t_num) && LE(t_num, 0));
  bool u_ok = c1_pos ? (LE(0, c2) && LE(c2, c1)) : (LE(c1, c2) && LE(c2, 0));
  if (t_ok && u_ok) {
    double t = static_cast<double>(t_num) / c1;
    *outx = static_cast<double>(a.x + t * ab_x);
    *outy = static_cast<double>(a.y + t * ab_y);
    return 0;
  }
  return -1;
}

// The two segments may use different point types (e.g. polygon edge vs. sweep line).
template<class PtA, class PtB>
int line_intersection1(
    const PtA &p1, const PtA &p2, const PtB &p3, const PtB &p4, double *outx, double *outy
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
  if (outx != nullptr && outy != nullptr) {
    *outx = static_cast<double>(x) / det;
    *outy = static_cast<double>(y) / det;
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
  if (lo1 == hi1 || lo2 == hi2) {
    return 0;
  }
  struct _PointD {
    double x, y;
  };  // For line intersection with the sweep line.
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
      double outx, outy;
      if (seg_intersection1(*i1, *j1, *i2, *j2, &outx, &outy) == 0) {
        xs.insert(outx);
      }
    }
  }
  std::vector<double> xsa(xs.begin(), xs.end());
  double res = 0;
  for (int k = 0; k + 1 < static_cast<int>(xsa.size()); k++) {
    double x = (xsa[k] + xsa[k + 1]) / 2;
    _PointD sweep0{x, 0}, sweep1{x, 1};
    std::vector<Event> events;
    for (int poly = 0; poly < 2; poly++) {
      It lo = plo[poly], hi = phi[poly];
      double area = 0;
      for (It i = lo, j = hi - 1; i != hi; j = i++) {
        area += (j->x - i->x) * (j->y + i->y);
      }
      for (It j = lo, i = hi - 1; j != hi; i = j++) {
        double px, py;
        if (line_intersection1(*j, *i, sweep0, sweep1, &px, &py) == 0) {
          double y = py, x0 = i->x, x1 = j->x;
          int sgn_area = (area < 0 ? 1 : (area > 0 ? -1 : 0));
          if (x0 < x && x1 > x) {
            events.emplace_back(y, sgn_area * (1 << poly));
          } else if (x0 > x && x1 < x) {
            events.emplace_back(y, -sgn_area * (1 << poly));
          }
        }
      }
    }
    std::sort(events.begin(), events.end(), [](const Event &e1, const Event &e2) {
      if (!EQ(e1.y, e2.y)) {
        return e1.y < e2.y;
      }
      return e1.mask_delta < e2.mask_delta;
    });
    double a = 0;
    int cnt[2] = {0, 0};
    for (int j = 0; j < static_cast<int>(events.size()); j++) {
      if (cnt[0] != 0 && cnt[1] != 0) {
        a += events[j].y - events[j - 1].y;
      }
      int bit = std::abs(events[j].mask_delta);
      int poly = (bit == 1 ? 0 : 1);
      cnt[poly] += events[j].mask_delta / bit;
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
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    area += (j->x - i->x) * (j->y + i->y);
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

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
  bool operator==(const PointI &o) const { return x == o.x && y == o.y; }
  bool operator!=(const PointI &p) const { return !(*this == p); }
  bool operator<(const PointI &p) const { return x != p.x ? x < p.x : y < p.y; }
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
