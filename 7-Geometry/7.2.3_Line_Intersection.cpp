/*

Intersection calculations in two dimensions for straight lines and line segments.

The functions below are templated on the point type `Pt`. The local `Point` struct (double
coordinates) is the default; replace it with `PointD`/ `PointI` from 7.1.1 or any struct with
numeric `.x` and `.y` fields.

For `seg_intersection()` detection-only use (`p=nullptr`/`q=nullptr`), calculations are exact when
`Pt` has integer coordinates. Segment intersection point output may use a different floating-point
point type, e.g. integer endpoints with `Point*` outputs.

- `line_intersection(a1, b1, c1, a2, b2, c2, &p)` intersects lines $`a1`x + `b1`y + `c1`=0$ and
  $`a2`x + `b2`y + `c2` = 0$, returning $-1$ (parallel), 0 (one point, stored in `p`), or 1
  (identical).
- `line_intersection(p1, p2, p3, p4, &p)` intersects the infinite lines through `p1`-`p2` and
  `p3`-`p4`.
- `seg_intersection(a, b, c, d, &p, &q)` intersects segments `a`-`b` and `c`-`d`, returning $-1$
  (none), 0 (one point), or 1 (overlapping segment). Touching behavior is controlled by
  `TOUCH_IS_INTERSECT`.
- `closest_point(a, b, c, p)` returns the closest point on line $`a`x + `b`y + `c`=0$ to $p$.

Time Complexity:
- O(1) for all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return LT(x, p.x) || (EQ(x, p.x) && LT(y, p.y)); }
  bool operator>(const Point &p) const { return p < *this; }
};

// clang-format off
template<class Pt> auto sqnorm(const Pt &a) { return a.x*a.x + a.y*a.y; }
template<class Pt> auto dot(const Pt &a, const Pt &b) { return a.x*b.x + a.y*b.y; }
template<class Pt> auto cross(const Pt &a, const Pt &b) { return a.x*b.y - a.y*b.x; }
// clang-format on

template<class Pt>
bool point_on_segment(const Pt &p, const Pt &a, const Pt &b) {
  return EQ((p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x), 0) &&
         LE(std::min(a.x, b.x), p.x) && LE(p.x, std::max(a.x, b.x)) &&
         LE(std::min(a.y, b.y), p.y) && LE(p.y, std::max(a.y, b.y));
}

int line_intersection(
    double a1, double b1, double c1, double a2, double b2, double c2, Point *p = nullptr
) {
  if (EQ(a1, a2) && EQ(b1, b2)) return EQ(c1, c2) ? 1 : -1;
  if (p != nullptr) {
    // Cramer's rule for a1*x + b1*y = -c1, a2*x + b2*y = -c2.
    double det = a1 * b2 - a2 * b1;
    p->x = (b1 * c2 - b2 * c1) / det;
    if (!EQ(b1, 0)) {
      p->y = -(a1 * p->x + c1) / b1;
    } else {
      p->y = -(a2 * p->x + c2) / b2;
    }
  }
  return 0;
}

template<class Pt>
int line_intersection(const Pt &p1, const Pt &p2, const Pt &p3, const Pt &p4, Point *p = nullptr) {
  auto a1 = p2.y - p1.y, b1 = p1.x - p2.x;
  auto c1 = -(p1.x * p2.y - p2.x * p1.y);
  auto a2 = p4.y - p3.y, b2 = p3.x - p4.x;
  auto c2 = -(p3.x * p4.y - p4.x * p3.y);
  auto x = -(c1 * b2 - c2 * b1), y = -(a1 * c2 - a2 * c1);
  auto det = a1 * b2 - a2 * b1;
  if (EQ(det, 0)) return (EQ(x, 0) && EQ(y, 0)) ? 1 : -1;
  if (p != nullptr) *p = Point{(double)x / det, (double)y / det};
  return 0;
}

// Detection is exact for integer Pt. Intersection point output may use a separate float OutPt.
template<class Pt, class OutPt = Point>
int seg_intersection(
    const Pt &a, const Pt &b, const Pt &c, const Pt &d, OutPt *p = nullptr, OutPt *q = nullptr
) {
  static const bool TOUCH_IS_INTERSECT = true;
  auto ab_x = b.x - a.x, ab_y = b.y - a.y;
  auto ac_x = c.x - a.x, ac_y = c.y - a.y;
  auto cd_x = d.x - c.x, cd_y = d.y - c.y;
  auto ab2 = ab_x * ab_x + ab_y * ab_y;
  auto cd2 = cd_x * cd_x + cd_y * cd_y;
  if (EQ(ab2, 0)) {
    if (TOUCH_IS_INTERSECT && point_on_segment(a, c, d)) {
      if (p != nullptr) {
        *p = OutPt{(double)a.x, (double)a.y};
      }
      return 0;
    }
    return -1;
  }
  if (EQ(cd2, 0)) {
    if (TOUCH_IS_INTERSECT && point_on_segment(c, a, b)) {
      if (p != nullptr) {
        *p = OutPt{(double)c.x, (double)c.y};
      }
      return 0;
    }
    return -1;
  }
  auto c1 = ab_x * cd_y - ab_y * cd_x;
  auto c2 = ac_x * ab_y - ac_y * ab_x;
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    Pt res1 = std::max(std::min(a, b), std::min(c, d));
    Pt res2 = std::min(std::max(a, b), std::max(c, d));
    bool overlap = TOUCH_IS_INTERSECT ? !(res2 < res1) : (res1 < res2);
    if (overlap) {
      if (res1 == res2) {
        if (p != nullptr) {
          *p = OutPt{(double)res1.x, (double)res1.y};
        }
        return 0;
      }
      if (p != nullptr && q != nullptr) {
        *p = OutPt{(double)res1.x, (double)res1.y};
        *q = OutPt{(double)res2.x, (double)res2.y};
      }
      return 1;
    }
    return -1;
  }
  if (EQ(c1, 0)) return -1;
  auto t_num = ac_x * cd_y - ac_y * cd_x;
  bool c1_pos = c1 > 0;
  bool t_ok = c1_pos ? (TOUCH_IS_INTERSECT ? (LE(0, t_num) && LE(t_num, c1))
                                           : (LT(0, t_num) && LT(t_num, c1)))
                     : (TOUCH_IS_INTERSECT ? (LE(t_num, 0) && LE(c1, t_num))
                                           : (LT(t_num, 0) && LT(c1, t_num)));
  bool u_ok = c1_pos ? (TOUCH_IS_INTERSECT ? (LE(0, c2) && LE(c2, c1)) : (LT(0, c2) && LT(c2, c1)))
                     : (TOUCH_IS_INTERSECT ? (LE(c2, 0) && LE(c1, c2)) : (LT(c2, 0) && LT(c1, c2)));
  if (t_ok && u_ok) {
    if (p != nullptr) {
      double t = (double)t_num / c1;
      *p = OutPt{a.x + t * ab_x, a.y + t * ab_y};
    }
    return 0;
  }
  return -1;
}

Point closest_point(double a, double b, double c, const Point &p) {
  if (EQ(a, 0)) return Point{p.x, -c / b};
  if (EQ(b, 0)) return Point{-c / a, p.y};
  Point res;
  line_intersection(a, b, c, -b, a, b * p.x - a * p.y, &res);
  return res;
}

/*** Example Usage ***/

#include <cassert>

struct PointI {
  int x, y;
  bool operator==(const PointI &p) const { return x == p.x && y == p.y; }
  bool operator!=(const PointI &p) const { return !(*this == p); }
  bool operator<(const PointI &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const PointI &p) const { return p < *this; }
};

int main() {
  Point p, q;

  assert(line_intersection(-1.0, 1.0, 0.0, 1.0, 1.0, -3.0, &p) == 0);
  assert((p == Point{1.5, 1.5}));
  assert(line_intersection(Point{0, 0}, Point{1, 1}, Point{0, 4}, Point{4, 0}, &p) == 0);
  assert((p == Point{2, 2}));

  {
#define test(a, b, c, d, e, f, g, h) \
  seg_intersection(Point{a, b}, Point{c, d}, Point{e, f}, Point{g, h}, &p, &q)

    assert((0 == test(-4, 0, 4, 0, 0, -4, 0, 4) && p == Point{0, 0}));
    assert((0 == test(0, 0, 10, 10, 2, 2, 16, 4) && p == Point{2, 2}));
    assert((0 == test(-2, 2, -2, -2, -2, 0, 0, 0) && p == Point{-2, 0}));
    assert((0 == test(0, 4, 4, 4, 4, 0, 4, 8) && p == Point{4, 4}));
    assert(1 == test(10, 10, 0, 0, 2, 2, 6, 6));
    assert((p == Point{2, 2} && q == Point{6, 6}));
    assert(-1 == test(6, 8, 8, 10, 12, 12, 4, 4));
    assert(-1 == test(-4, 2, -8, 8, 0, 0, -4, 6));
  }

  assert((Point{2.5, 2.5} == closest_point(-1.0, -1.0, 5.0, Point{0, 0})));
  assert((Point{3, 0} == closest_point(1.0, 0.0, -3.0, Point{0, 0})));

  // Detection-only with integer coordinates (exact, no float needed).
  assert(seg_intersection(PointI{0, 0}, PointI{4, 4}, PointI{0, 4}, PointI{4, 0}) == 0);
  assert(seg_intersection(PointI{0, 0}, PointI{1, 0}, PointI{2, 0}, PointI{3, 0}) == -1);
  assert(seg_intersection(PointI{0, 0}, PointI{4, 4}, PointI{0, 4}, PointI{4, 0}, &p) == 0);
  assert((p == Point{2, 2}));

  return 0;
}
