/*

Common rectangle calculations in two dimensions.

- rectangle_area(a, b) returns the area of a rectangle with opposing vertices a
  and b.
- point_in_rectangle(p, x, y, w, h) returns whether point p lies within the
  rectangle defined by a vertex at v (x, y), a width of w, and a height of h.
  Note that negative widths and heights are supported. If the point lies on or
  close to an edge (by roughly EPS), then the result will depend on the setting
  of EDGE_IS_INSIDE.
- point_in_rectangle(p, a, b) returns whether point p lies within the rectangle
  with opposing vertices a and b. If the point lies on or close to an edge (by
  roughly EPS), then the result will depend on the setting of EDGE_IS_INSIDE.
- rectangle_intersection(a1, b1, a2, b2, &p, &q) determines the intersection
  region of the rectangle with opposing vertices a1 and b1 and the rectangle
  with opposing vertices a2 and b2. Returns -1 if the rectangles are completely
  disjoint, 0 if the rectangles partially intersect, 1 if the first rectangle is
  completely inside the second, and 2 if the second rectangle is completely
  inside the first. If there is an intersection, the opposing vertices of the
  intersection rectangle will be stored into pointers p and q if they are not
  NULL. If the intersection is a single point or line segment, then the result
  will depend on the setting of EDGE_IS_INSIDE within point_in_rectangle().

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

typedef std::pair<double, double> point;
#define x first
#define y second

double rectangle_area(const point &a, const point &b) {
  return fabs((a.x - b.x)*(a.y - b.y));
}

bool point_in_rectangle(const point &p, const point &v, double w, double h) {
  static const bool EDGE_IS_INSIDE = true;
  if (w < 0) {
    return point_in_rectangle(p, point(v.x + w, v.y), -w, h);
  }
  if (h < 0) {
    return point_in_rectangle(p, point(v.x, v.y + h), w, -h);
  }
  return EDGE_IS_INSIDE
      ? (GE(p.x, v.x) && LE(p.x, v.x + w) && GE(p.y, v.y) && LE(p.y, v.y + h))
      : (GT(p.x, v.x) && LT(p.x, v.x + w) && GT(p.y, v.y) && LT(p.y, v.y + h));
}

bool point_in_rectangle(const point &p, const point &a, const point &b) {
  double xl = std::min(a.x, b.x), yl = std::min(a.y, b.y);
  double xh = std::max(a.x, b.x), yh = std::max(a.y, b.y);
  return point_in_rectangle(p, point(xl, yl), xh - xl, yh - yl);
}

int rectangle_intersection(const point &a1, const point &b1, const point &a2,
                           const point &b2, point *p = NULL, point *q = NULL) {
  bool a1in2 = point_in_rectangle(a1, a2, b2);
  bool b1in2 = point_in_rectangle(b1, a2, b2);
  if (a1in2 && b1in2) {
    if (p != NULL && q != NULL) {
      *p = std::min(a1, b1);
      *q = std::max(a1, b1);
    }
    return 1;  // Rectangle 1 completely inside 2.
  }
  if (!a1in2 && !b1in2) {
    if (point_in_rectangle(a2, a1, b1)) {
      if (p != NULL && q != NULL) {
        *p = std::min(a2, b2);
        *q = std::max(a2, b2);
      }
      return 2;  // Rectangle 2 completely inside 1.
    }
    return -1;  // Completely disjoint.
  }
  if (p != NULL && q != NULL) {
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

bool EQP(const point &a, const point &b) {
  return EQ(a.x, b.x) && EQ(a.y, b.y);
}

int main() {
  assert(EQ(20, rectangle_area(point(1, 1), point(5, 6))));

  assert(point_in_rectangle(point(0, -1), point(0, -3), 3, 2));
  assert(point_in_rectangle(point(2, -2), point(3, -3), -3, 2));
  assert(!point_in_rectangle(point(0, 0), point(3, -1), -3, -2));
  assert(point_in_rectangle(point(2, -2), point(3, -3), point(0, -1)));
  assert(!point_in_rectangle(point(-1, -2), point(3, -3), point(0, -1)));

  point p, q;
  assert(-1 == rectangle_intersection(point(0, 0), point(1, 1),
                                      point(2, 2), point(3, 3)));
  assert(0 == rectangle_intersection(point(1, 1), point(7, 7),
                                     point(5, 5), point(0, 0), &p, &q));
  assert(EQP(p, point(1, 1)) && EQP(q, point(5, 5)));
  assert(1 == rectangle_intersection(point(1, 1), point(0, 0),
                                     point(0, 0), point(1, 10), &p, &q));
  assert(EQP(p, point(0, 0)) && EQP(q, point(1, 1)));
  assert(2 == rectangle_intersection(point(0, 5), point(5, 7),
                                     point(1, 6), point(2, 5), &p, &q));
  assert(EQP(p, point(1, 6)) && EQP(q, point(2, 5)));

  return 0;
}
