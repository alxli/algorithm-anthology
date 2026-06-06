/*

Common rectangle calculations in two dimensions. The functions are templated on the point type `Pt`,
which should work with `Point`/`PointD`/`PointI` from 7.1.1, or any struct with numeric `.x` and
`.y` fields. All operations use comparisons only and are exact for integer-coordinate points.

- `point_in_rectangle(p, v, w, h)` returns whether point `p` lies within the rectangle defined by a
  vertex `v`, a width of `w`, and a height of `h`. Note that negative widths and heights are
  supported. If the point lies on or close to an edge (by roughly `EPS`), then the result will
  depend on the setting of `EDGE_IS_INSIDE`.
- `point_in_rectangle(p, a, b)` returns whether point `p` lies within the rectangle with opposing
  vertices `a` and `b`. If the point lies on or close to an edge (by roughly `EPS`), then the result
  will depend on the setting of `EDGE_IS_INSIDE`.
- `rectangle_intersection(a1, b1, a2, b2, &p, &q)` determines the intersection region of the
  rectangle with opposing vertices `a_1` and `b_1` and the rectangle with opposing vertices `a_2`
  and `b_2`. Returns $-1$ if the rectangles are completely disjoint, 0 if the rectangles partially
  intersect, 1 if the first rectangle is completely inside the second, and 2 if the second rectangle
  is completely inside the first. If there is an intersection, the lower-left and upper-right
  vertices of the normalized intersection rectangle will be stored into pointers `p` and `q` if they
  are not `nullptr`. If the intersection is a single point or line segment, then the result will
  depend on the setting of `EDGE_IS_INSIDE` within `point_in_rectangle()`.

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
#define GT(a, b) ((a) > (b) + EPS)
#define LE(a, b) ((a) <= (b) + EPS)
#define GE(a, b) ((a) >= (b) - EPS)

template<class Pt, class T>
bool point_in_rectangle(const Pt &p, const Pt &v, const T &w, const T &h) {
  static const bool EDGE_IS_INSIDE = true;
  if (w < 0) {
    return point_in_rectangle(p, Pt(v.x + w, v.y), -w, h);
  }
  if (h < 0) {
    return point_in_rectangle(p, Pt(v.x, v.y + h), w, -h);
  }
  return EDGE_IS_INSIDE ? (GE(p.x, v.x) && LE(p.x, v.x + w) && GE(p.y, v.y) && LE(p.y, v.y + h))
                        : (GT(p.x, v.x) && LT(p.x, v.x + w) && GT(p.y, v.y) && LT(p.y, v.y + h));
}

template<class Pt>
bool point_in_rectangle(const Pt &p, const Pt &a, const Pt &b) {
  auto xl = std::min(a.x, b.x), yl = std::min(a.y, b.y);
  auto xh = std::max(a.x, b.x), yh = std::max(a.y, b.y);
  return point_in_rectangle(Pt(p.x, p.y), Pt(xl, yl), xh - xl, yh - yl);
}

template<class Pt>
int rectangle_intersection(
    const Pt &a1, const Pt &b1, const Pt &a2, const Pt &b2, Pt *p = nullptr, Pt *q = nullptr
) {
  static const bool EDGE_IS_INSIDE = true;
  // Normalize each rectangle to [xl, xh] x [yl, yh] with coordinate-wise min/max only.
  // (Using auto keeps the native coordinate type, so integer rectangles stay exact.)
  auto xl1 = std::min(a1.x, b1.x), xh1 = std::max(a1.x, b1.x);
  auto yl1 = std::min(a1.y, b1.y), yh1 = std::max(a1.y, b1.y);
  auto xl2 = std::min(a2.x, b2.x), xh2 = std::max(a2.x, b2.x);
  auto yl2 = std::min(a2.y, b2.y), yh2 = std::max(a2.y, b2.y);
  // The intersection is the overlap of the two coordinate intervals.
  auto ixl = std::max(xl1, xl2), ixh = std::min(xh1, xh2);
  auto iyl = std::max(yl1, yl2), iyh = std::min(yh1, yh2);
  bool disjoint = EDGE_IS_INSIDE ? (GT(ixl, ixh) || GT(iyl, iyh)) : (GE(ixl, ixh) || GE(iyl, iyh));
  if (disjoint) {
    return -1;  // Completely disjoint.
  }
  // Opposing corners of the overlap: bottom-left in p, top-right in q.
  if (p != nullptr) {
    *p = Pt(ixl, iyl);
  }
  if (q != nullptr) {
    *q = Pt(ixh, iyh);
  }
  // The overlap equals a rectangle exactly when that rectangle is contained in the other.
  if (EQ(ixl, xl1) && EQ(ixh, xh1) && EQ(iyl, yl1) && EQ(iyh, yh1)) {
    return 1;  // Rectangle 1 completely inside 2.
  }
  if (EQ(ixl, xl2) && EQ(ixh, xh2) && EQ(iyl, yl2) && EQ(iyh, yh2)) {
    return 2;  // Rectangle 2 completely inside 1.
  }
  return 0;  // Partial intersection.
}

/*** Example Usage ***/

#include <cassert>

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
};

int main() {
  assert(point_in_rectangle(Point(0, -1), Point(0, -3), 3, 2));
  assert(point_in_rectangle(Point(2, -2), Point(3, -3), -3, 2));
  assert(!point_in_rectangle(Point(0, 0), Point(3, -1), -3, -2));
  assert(point_in_rectangle(Point(2, -2), Point(3, -3), Point(0, -1)));
  assert(!point_in_rectangle(Point(-1, -2), Point(3, -3), Point(0, -1)));

  Point p, q;
  assert(-1 == rectangle_intersection(Point(0, 0), Point(1, 1), Point(2, 2), Point(3, 3)));
  assert(0 == rectangle_intersection(Point(1, 1), Point(7, 7), Point(5, 5), Point(0, 0), &p, &q));
  assert(p == Point(1, 1) && q == Point(5, 5));
  assert(1 == rectangle_intersection(Point(1, 1), Point(0, 0), Point(0, 0), Point(1, 10), &p, &q));
  assert(p == Point(0, 0) && q == Point(1, 1));
  assert(2 == rectangle_intersection(Point(0, 5), Point(5, 7), Point(1, 6), Point(2, 5), &p, &q));
  assert(p == Point(1, 5) && q == Point(2, 6));

  return 0;
}
