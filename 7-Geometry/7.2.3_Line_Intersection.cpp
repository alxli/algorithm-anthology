/*

Intersection calculations in two dimensions for straight lines and line segments. The functions
are templated on the point type `Pt`: pass any struct with numeric `.x` and `.y` fields (for example
`PointD`/ `PointI` from 7.1.1, or the local example struct) for which `operator<` orders points
lexicographically using exact coordinate comparisons. Point outputs are written through a
caller-supplied pointer whose type is deduced, so the output point type may differ from the input --
e.g. integer endpoints with a floating-point output point.

`seg_intersection()` has a detection-only overload (called without the output pointers) whose
calculations are exact when `Pt` has integer coordinates.

- `line_intersection(a1, b1, c1, a2, b2, c2, &p)` intersects lines $a_1 x + b_1 y + c_1 = 0$ and
  $a_2 x + b_2 y + c_2 = 0$, returning $-1$ (parallel), $0$ (one point, stored into `p`), or $1$
  (identical).
- `line_intersection(p1, p2, p3, p4, &p)` intersects the infinite lines through `p1`-`p2` and
  `p3`-`p4`, returning $-1$ (parallel), $0$ (one point, stored into `p`), or $1$ (identical).
- `seg_intersection(a, b, c, d, &p, &q, TOUCH_IS_INTERSECT)` intersects segments `a`-`b` and
  `c`-`d`, returning $-1$ (none), $0$ (one point), or $1$ (overlapping segment). The
  `TOUCH_IS_INTERSECT` flag (default `true`) controls whether segments that meet only at an
  endpoint count as intersecting. If the intersection is a point (and `p` is not `nullptr`), it
  will be stored into `p`. If the intersection is an overlapping segment (and `p` and `q` are not
  `nullptr`) then their endpoints will be stored into pointers `p` and `q`, which must reference a
  floating-point point type.
- `seg_intersection(a, b, c, d, TOUCH_IS_INTERSECT)` is a simplified, detection-only version of the
  above. Both versions are exact for detection if the input point type is integral.
- `closest_point(a, b, c, p)` returns the closest point on line $ax + by + c = 0$ to point `p`.

Overflow warning: the exact integer paths multiply coordinate differences (cross products and
squared lengths), which grow like the squared coordinate magnitude. With 32-bit `int` coordinates
these overflow once coordinates exceed $\sim 46000$, so for larger integer inputs use a point type
with 64-bit (`int64_t`) coordinates.

Time Complexity:
- O(1) for all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

const double EPS = 1e-9;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return std::fabs(C(a) - C(b)) <= static_cast<C>(EPS);
  return C(a) == C(b);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool LT(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) < C(b) - static_cast<C>(EPS);
  return C(a) < C(b);
}

template<typename T, typename U>
bool LE(T a, U b) {
  return !LT(b, a);
}

template<typename OutPt>
int line_intersection(double a1, double b1, double c1, double a2, double b2, double c2, OutPt *p) {
  // Cramer's rule for a1*x + b1*y = -c1, a2*x + b2*y = -c2.
  double det = a1 * b2 - a2 * b1;
  if (EQ(det, 0)) {  // Parallel (lines need not be given in normalized form).
    // Identical iff the other two 2x2 determinants also vanish.
    return (EQ(a1 * c2 - a2 * c1, 0) && EQ(b1 * c2 - b2 * c1, 0)) ? 1 : -1;
  }
  p->x = (b1 * c2 - b2 * c1) / det;
  if (!EQ(b1, 0)) {
    p->y = -(a1 * p->x + c1) / b1;
  } else {
    p->y = -(a2 * p->x + c2) / b2;
  }
  return 0;
}

template<typename Pt, typename OutPt>
int line_intersection(const Pt &p1, const Pt &p2, const Pt &p3, const Pt &p4, OutPt *p) {
  auto a1 = p2.y - p1.y, b1 = p1.x - p2.x;
  auto c1 = -(p1.x * p2.y - p2.x * p1.y);
  auto a2 = p4.y - p3.y, b2 = p3.x - p4.x;
  auto c2 = -(p3.x * p4.y - p4.x * p3.y);
  auto x = -(c1 * b2 - c2 * b1), y = -(a1 * c2 - a2 * c1);
  auto det = a1 * b2 - a2 * b1;
  if (EQ(det, 0)) {
    return (EQ(x, 0) && EQ(y, 0)) ? 1 : -1;
  }
  p->x = static_cast<double>(x) / det;
  p->y = static_cast<double>(y) / det;
  return 0;
}

template<typename Pt>
bool point_on_segment(const Pt &p, const Pt &a, const Pt &b) {
  // Overflow risk for integer Pt: these products are ~O(max_coord^2); use int64_t if necessary.
  return EQ((p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x), 0) &&
         LE(std::min(a.x, b.x), p.x) && LE(p.x, std::max(a.x, b.x)) &&
         LE(std::min(a.y, b.y), p.y) && LE(p.y, std::max(a.y, b.y));
}

// Detection is exact for integer Pt. Intersection point output may use a separate float OutPt.
template<typename Pt, typename OutPt>
int seg_intersection(
    const Pt &a, const Pt &b, const Pt &c, const Pt &d, OutPt *p = nullptr, OutPt *q = nullptr,
    const bool TOUCH_IS_INTERSECT = true
) {
  auto ab_x = b.x - a.x, ab_y = b.y - a.y;
  auto ac_x = c.x - a.x, ac_y = c.y - a.y;
  auto cd_x = d.x - c.x, cd_y = d.y - c.y;
  auto ab2 = ab_x * ab_x + ab_y * ab_y;
  auto cd2 = cd_x * cd_x + cd_y * cd_y;
  if (EQ(ab2, 0)) {
    if (TOUCH_IS_INTERSECT && point_on_segment(a, c, d)) {
      if (p != nullptr) {
        p->x = static_cast<double>(a.x);
        p->y = static_cast<double>(a.y);
      }
      return 0;  // Degenerate: first segment is a point intersecting the second segment.
    }
    return -1;  // Degenerate: first segment is a point not intersecting the second segment.
  }
  if (EQ(cd2, 0)) {
    if (TOUCH_IS_INTERSECT && point_on_segment(c, a, b)) {
      if (p != nullptr) {
        p->x = static_cast<double>(c.x);
        p->y = static_cast<double>(c.y);
      }
      return 0;  // Degenerate: second segment is a point intersecting the first segment.
    }
    return -1;  // Degenerate: second segment is a point not intersecting the first segment.
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
          p->x = static_cast<double>(res1.x);
          p->y = static_cast<double>(res1.y);
        }
        return 0;  // Collinear and overlapping: touch at one endpoint.
      }
      if (p != nullptr && q != nullptr) {
        p->x = static_cast<double>(res1.x);
        p->y = static_cast<double>(res1.y);
        q->x = static_cast<double>(res2.x);
        q->y = static_cast<double>(res2.y);
      }
      return 1;  // Collinear and overlapping: overlap is a segment.
    }
    return -1;  // Collinear and disjoint.
  }
  if (EQ(c1, 0)) {
    return -1;  // Parallel and disjoint.
  }
  auto t_num = ac_x * cd_y - ac_y * cd_x;
  bool c1_pos = c1 > 0;
  bool t_ok = c1_pos ? (TOUCH_IS_INTERSECT ? (LE(0, t_num) && LE(t_num, c1))
                                           : (LT(0, t_num) && LT(t_num, c1)))
                     : (TOUCH_IS_INTERSECT ? (LE(c1, t_num) && LE(t_num, 0))
                                           : (LT(c1, t_num) && LT(t_num, 0)));
  bool u_ok = c1_pos ? (TOUCH_IS_INTERSECT ? (LE(0, c2) && LE(c2, c1)) : (LT(0, c2) && LT(c2, c1)))
                     : (TOUCH_IS_INTERSECT ? (LE(c1, c2) && LE(c2, 0)) : (LT(c1, c2) && LT(c2, 0)));
  if (t_ok && u_ok) {
    if (p != nullptr) {
      double t = static_cast<double>(t_num) / c1;
      p->x = static_cast<double>(a.x + t * ab_x);
      p->y = static_cast<double>(a.y + t * ab_y);
    }
    return 0;  // Non-parallel with one intersection.
  }
  return -1;  // Non-parallel with no intersection.
}

// Simplified detection-only version (no output points needed); exact for integer Pt.
// Alternatively, just call the version above and pass static_cast<Pt *>(nullptr) for p and q.
template<typename Pt>
int seg_intersection(
    const Pt &a, const Pt &b, const Pt &c, const Pt &d, const bool TOUCH_IS_INTERSECT = true
) {
  auto ab_x = b.x - a.x, ab_y = b.y - a.y;
  auto ac_x = c.x - a.x, ac_y = c.y - a.y;
  auto cd_x = d.x - c.x, cd_y = d.y - c.y;
  auto ab2 = ab_x * ab_x + ab_y * ab_y;
  auto cd2 = cd_x * cd_x + cd_y * cd_y;
  if (EQ(ab2, 0)) {
    return (TOUCH_IS_INTERSECT && point_on_segment(a, c, d)) ? 0 : -1;
  }
  if (EQ(cd2, 0)) {
    return (TOUCH_IS_INTERSECT && point_on_segment(c, a, b)) ? 0 : -1;
  }
  auto c1 = ab_x * cd_y - ab_y * cd_x;
  auto c2 = ac_x * ab_y - ac_y * ab_x;
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    Pt res1 = std::max(std::min(a, b), std::min(c, d));
    Pt res2 = std::min(std::max(a, b), std::max(c, d));
    bool overlap = TOUCH_IS_INTERSECT ? !(res2 < res1) : (res1 < res2);
    if (overlap) {
      return (res1 == res2) ? 0 : 1;
    }
    return -1;
  }
  if (EQ(c1, 0)) {
    return -1;
  }
  auto t_num = ac_x * cd_y - ac_y * cd_x;
  bool c1_pos = c1 > 0;
  bool t_ok = c1_pos ? (TOUCH_IS_INTERSECT ? (LE(0, t_num) && LE(t_num, c1))
                                           : (LT(0, t_num) && LT(t_num, c1)))
                     : (TOUCH_IS_INTERSECT ? (LE(c1, t_num) && LE(t_num, 0))
                                           : (LT(c1, t_num) && LT(t_num, 0)));
  bool u_ok = c1_pos ? (TOUCH_IS_INTERSECT ? (LE(0, c2) && LE(c2, c1)) : (LT(0, c2) && LT(c2, c1)))
                     : (TOUCH_IS_INTERSECT ? (LE(c1, c2) && LE(c2, 0)) : (LT(c1, c2) && LT(c2, 0)));
  return (t_ok && u_ok) ? 0 : -1;
}

template<typename Pt>
Pt closest_point(double a, double b, double c, const Pt &p) {
  Pt res{};
  if (EQ(a, 0)) {
    res.x = p.x;
    res.y = -c / b;
  } else if (EQ(b, 0)) {
    res.x = -c / a;
    res.y = p.y;
  } else {
    line_intersection(a, b, c, -b, a, b * p.x - a * p.y, &res);
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  friend bool EQ(const Point &a, const Point &b) { return EQ(a.x, b.x) && EQ(a.y, b.y); }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
  bool operator==(const PointI &p) const { return x == p.x && y == p.y; }
  bool operator!=(const PointI &p) const { return !(*this == p); }
  bool operator<(const PointI &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const PointI &p) const { return p < *this; }
};

int main() {
  Point p, q;

  assert(line_intersection(-1.0, 1.0, 0.0, 1.0, 1.0, -3.0, &p) == 0);
  assert(EQ(p, Point(1.5, 1.5)));
  assert(line_intersection(Point(0, 0), Point(1, 1), Point(0, 4), Point(4, 0), &p) == 0);
  assert(EQ(p, Point(2, 2)));

  auto test = [&](double a, double b, double c, double d, double e, double f, double g, double h) {
    return seg_intersection(Point(a, b), Point(c, d), Point(e, f), Point(g, h), &p, &q);
  };
  assert(0 == test(-4, 0, 4, 0, 0, -4, 0, 4) && EQ(p, Point(0, 0)));
  assert(0 == test(0, 0, 10, 10, 2, 2, 16, 4) && EQ(p, Point(2, 2)));
  assert(0 == test(-2, 2, -2, -2, -2, 0, 0, 0) && EQ(p, Point(-2, 0)));
  assert(0 == test(0, 4, 4, 4, 4, 0, 4, 8) && EQ(p, Point(4, 4)));
  assert(1 == test(10, 10, 0, 0, 2, 2, 6, 6));
  assert(EQ(p, Point(2, 2)) && EQ(q, Point(6, 6)));
  assert(-1 == test(6, 8, 8, 10, 12, 12, 4, 4));
  assert(-1 == test(-4, 2, -8, 8, 0, 0, -4, 6));

  assert(EQ(Point(2.5, 2.5), closest_point(-1.0, -1.0, 5.0, Point(0, 0))));
  assert(EQ(Point(3, 0), closest_point(1.0, 0.0, -3.0, Point(0, 0))));

  // Detection-only with integer coordinates (exact, no float needed).
  assert(seg_intersection(PointI(0, 0), PointI(4, 4), PointI(0, 4), PointI(4, 0)) == 0);
  assert(seg_intersection(PointI(0, 0), PointI(1, 0), PointI(2, 0), PointI(3, 0)) == -1);
  assert(seg_intersection(PointI(0, 0), PointI(4, 4), PointI(0, 4), PointI(4, 0), &p) == 0);
  assert(EQ(p, Point(2, 2)));

  // TOUCH_IS_INTERSECT = false treats endpoint-only contact as non-intersecting. The T-junction
  // and shared-endpoint cases below intersect by default but are rejected when the flag is off.
  assert(0 == seg_intersection(Point(0, 4), Point(4, 4), Point(4, 0), Point(4, 8), &p, &q));
  assert(-1 == seg_intersection(Point(0, 4), Point(4, 4), Point(4, 0), Point(4, 8), &p, &q, false));
  assert(-1 == seg_intersection(PointI(0, 0), PointI(2, 2), PointI(2, 2), PointI(6, 6), false));
  return 0;
}
