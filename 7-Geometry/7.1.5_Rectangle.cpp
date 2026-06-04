/*

Common rectangle calculations in two dimensions.

- `rectangle_area(a, b)` returns the area of a rectangle with opposing vertices $a$ and $b$.
- `point_in_rectangle(p, v, w, h)` returns whether point $p$ lies within the rectangle defined by a
  vertex `v` at $(x, y)$, a width of $w$, and a height of $h$. Note that negative widths and heights
  are supported. If the point lies on or close to an edge (by roughly `EPS`), then the result will
  depend on the setting of `EDGE_IS_INSIDE`.
- `point_in_rectangle(p, a, b)` returns whether point $p$ lies within the rectangle with opposing
  vertices $a$ and $b$. If the point lies on or close to an edge (by roughly `EPS`), then the result
  will depend on the setting of `EDGE_IS_INSIDE`.
- `rectangle_intersection(a1, b1, a2, b2, &p, &q)` determines the intersection region of the
  rectangle with opposing vertices $a_1$ and $b_1$ and the rectangle with opposing vertices $a_2$
  and $b_2$. Returns $-1$ if the rectangles are completely disjoint, 0 if the rectangles partially
  intersect, 1 if the first rectangle is completely inside the second, and 2 if the second rectangle
  is completely inside the first. If there is an intersection, the opposing vertices of the
  intersection rectangle will be stored into pointers `p` and `q` if they are not `nullptr`. If the
  intersection is a single point or line segment, then the result will depend on the setting of
  `EDGE_IS_INSIDE` within `point_in_rectangle()`.

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

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

double rectangle_area(const Point &a, const Point &b) {
  return fabs((a.x - b.x) * (a.y - b.y));
}

bool point_in_rectangle(const Point &p, const Point &v, double w, double h) {
  static const bool EDGE_IS_INSIDE = true;
  if (w < 0) {
    return point_in_rectangle(p, Point(v.x + w, v.y), -w, h);
  }
  if (h < 0) {
    return point_in_rectangle(p, Point(v.x, v.y + h), w, -h);
  }
  return EDGE_IS_INSIDE ? (GE(p.x, v.x) && LE(p.x, v.x + w) && GE(p.y, v.y) && LE(p.y, v.y + h))
                        : (GT(p.x, v.x) && LT(p.x, v.x + w) && GT(p.y, v.y) && LT(p.y, v.y + h));
}

bool point_in_rectangle(const Point &p, const Point &a, const Point &b) {
  double xl = std::min(a.x, b.x), yl = std::min(a.y, b.y);
  double xh = std::max(a.x, b.x), yh = std::max(a.y, b.y);
  return point_in_rectangle(p, Point(xl, yl), xh - xl, yh - yl);
}

int rectangle_intersection(
    const Point &a1, const Point &b1, const Point &a2, const Point &b2, Point *p = nullptr,
    Point *q = nullptr
) {
  bool a1in2 = point_in_rectangle(a1, a2, b2);
  bool b1in2 = point_in_rectangle(b1, a2, b2);
  if (a1in2 && b1in2) {
    if (p != nullptr && q != nullptr) {
      *p = std::min(a1, b1);
      *q = std::max(a1, b1);
    }
    return 1;  // Rectangle 1 completely inside 2.
  }
  if (!a1in2 && !b1in2) {
    if (point_in_rectangle(a2, a1, b1)) {
      if (p != nullptr && q != nullptr) {
        *p = std::min(a2, b2);
        *q = std::max(a2, b2);
      }
      return 2;  // Rectangle 2 completely inside 1.
    }
    return -1;  // Completely disjoint.
  }
  if (p != nullptr && q != nullptr) {
    if (a1in2) {
      *p = a1;
      *q = (a1 < b1) ? std::max(a2, b2) : std::min(a2, b2);
    } else {
      *p = b1;
      *q = (b1 < a1) ? std::max(a2, b2) : std::min(a2, b2);
    }
    if (*p > *q) {
      std::swap(p, q);
    }
  }
  return 0;
}

/*** Example Usage ***/

#include <cassert>

bool EQP(const Point &a, const Point &b) {
  return EQ(a.x, b.x) && EQ(a.y, b.y);
}

int main() {
  assert(EQ(20, rectangle_area(Point(1, 1), Point(5, 6))));

  assert(point_in_rectangle(Point(0, -1), Point(0, -3), 3, 2));
  assert(point_in_rectangle(Point(2, -2), Point(3, -3), -3, 2));
  assert(!point_in_rectangle(Point(0, 0), Point(3, -1), -3, -2));
  assert(point_in_rectangle(Point(2, -2), Point(3, -3), Point(0, -1)));
  assert(!point_in_rectangle(Point(-1, -2), Point(3, -3), Point(0, -1)));

  Point p, q;
  assert(-1 == rectangle_intersection(Point(0, 0), Point(1, 1), Point(2, 2), Point(3, 3)));
  assert(0 == rectangle_intersection(Point(1, 1), Point(7, 7), Point(5, 5), Point(0, 0), &p, &q));
  assert(EQP(p, Point(1, 1)) && EQP(q, Point(5, 5)));
  assert(1 == rectangle_intersection(Point(1, 1), Point(0, 0), Point(0, 0), Point(1, 10), &p, &q));
  assert(EQP(p, Point(0, 0)) && EQP(q, Point(1, 1)));
  assert(2 == rectangle_intersection(Point(0, 5), Point(5, 7), Point(1, 6), Point(2, 5), &p, &q));
  assert(EQP(p, Point(1, 6)) && EQP(q, Point(2, 5)));

  return 0;
}
