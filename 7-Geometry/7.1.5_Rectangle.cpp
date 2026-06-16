/*

Common axis-aligned rectangle calculations in two dimensions. The functions are templated on the
point type `Pt`, which should work with `Point`/`PointD`/`PointI` from 7.1.1, or any struct with
numeric `.x` and `.y` fields.

- `point_in_rectangle(p, v, w, h, EDGE_IS_INSIDE)` returns whether point `p` lies inside the
  axis-aligned rectangle with corner `v`, width `w`, and height `h`. Negative widths and heights are
  supported. The `EDGE_IS_INSIDE` flag (default `true`) controls whether boundary points count.
- `point_in_rectangle(p, a, b, EDGE_IS_INSIDE)` returns whether point `p` lies inside the
  axis-aligned rectangle with opposite corners `a` and `b`.
- `rectangle_intersection(a1, b1, a2, b2, &p, &q, EDGE_IS_INSIDE)` computes the intersection of two
  axis-aligned rectangles, where `a1`/`b1` and `a2`/`b2` are opposite-corner pairs. Returns -1 if
  the rectangles are disjoint, 0 if they partially intersect, 1 if the first rectangle is completely
  inside the second, and 2 if the second rectangle is completely inside the first. If an
  intersection exists, its lower-left and upper-right corners are stored in `p` and `q` when those
  pointers are non-null. The `EDGE_IS_INSIDE` flag (default `true`) controls whether boundary-only
  contact counts as intersecting.

For integer-coordinate inputs, comparisons are exact as long as intermediate coordinate additions,
subtractions, and min/max values do not overflow. Floating-point inputs use EPS-based comparisons.

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
bool point_in_rectangle(
    const Pt &p, const Pt &v, const T &w, const T &h, const bool EDGE_IS_INSIDE = true
) {
  if (w < 0) {
    return point_in_rectangle(p, Pt(v.x + w, v.y), -w, h, EDGE_IS_INSIDE);
  }
  if (h < 0) {
    return point_in_rectangle(p, Pt(v.x, v.y + h), w, -h, EDGE_IS_INSIDE);
  }
  return EDGE_IS_INSIDE ? (GE(p.x, v.x) && LE(p.x, v.x + w) && GE(p.y, v.y) && LE(p.y, v.y + h))
                        : (GT(p.x, v.x) && LT(p.x, v.x + w) && GT(p.y, v.y) && LT(p.y, v.y + h));
}

template<class Pt>
bool point_in_rectangle(const Pt &p, const Pt &a, const Pt &b, const bool EDGE_IS_INSIDE = true) {
  auto xl = std::min(a.x, b.x), yl = std::min(a.y, b.y);
  auto xh = std::max(a.x, b.x), yh = std::max(a.y, b.y);
  return point_in_rectangle(p, Pt(xl, yl), xh - xl, yh - yl, EDGE_IS_INSIDE);
}

template<class Pt>
int rectangle_intersection(
    const Pt &a1, const Pt &b1, const Pt &a2, const Pt &b2, Pt *p = nullptr, Pt *q = nullptr,
    const bool EDGE_IS_INSIDE = true
) {
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
  assert(!point_in_rectangle(Point(0, -1), Point(0, -3), 3, 2, false));
  assert(point_in_rectangle(Point(2, -2), Point(3, -3), -3, 2));
  assert(!point_in_rectangle(Point(0, 0), Point(3, -1), -3, -2));
  assert(point_in_rectangle(Point(2, -2), Point(3, -3), Point(0, -1)));
  assert(!point_in_rectangle(Point(0, -1), Point(3, -3), Point(0, -1), false));
  assert(!point_in_rectangle(Point(-1, -2), Point(3, -3), Point(0, -1)));

  Point p, q;
  assert(-1 == rectangle_intersection(Point(0, 0), Point(1, 1), Point(2, 2), Point(3, 3)));
  assert(0 == rectangle_intersection(Point(1, 1), Point(7, 7), Point(5, 5), Point(0, 0), &p, &q));
  assert(p == Point(1, 1) && q == Point(5, 5));
  assert(1 == rectangle_intersection(Point(1, 1), Point(0, 0), Point(0, 0), Point(1, 10), &p, &q));
  assert(p == Point(0, 0) && q == Point(1, 1));
  assert(2 == rectangle_intersection(Point(0, 5), Point(5, 7), Point(1, 6), Point(2, 5), &p, &q));
  assert(p == Point(1, 5) && q == Point(2, 6));
  assert(
      -1 ==
      rectangle_intersection(Point(0, 0), Point(1, 1), Point(1, 1), Point(2, 2), &p, &q, false)
  );
  return 0;
}
