/*

Circle tangent and intersection calculations in two dimensions.

- `tangent(c, p, &l1, &l2)` determines the line(s) tangent to circle $c$ that pass through point
  $p$, returning $-1$ if there is no tangent line because $p$ is strictly inside $c$, 0 if there is
  exactly one tangent line because $p$ is on the boundary of $c$ (in which case the line will be
  stored into pointer `l1` if it's not `nullptr`), or 1 if there are two tangent lines because $p$
  is strictly outside of $c$ (in which case the lines will be stored into pointers `l1` and `l2` if
  they are not `nullptr`).
- `intersection(c, l, &p, &q)` determines the intersection between the circle $c$ and line $l$,
  returning $-1$ if there is no intersection, 0 if the line has one intersection point because the
  line is tangent (in which case it will be stored into pointer `p` if it's not `nullptr`), or 1 if
  there are two intersection points because the line crosses through the circle (in which case they
  will be stored into pointers `p` and `q` if they are not `nullptr`).
- `intersection(c1, c2, &p, &q)` determines the intersection points between two circles $c_1$ and
  $c_2$, returning $-2$ if circle $c_2$ completely encloses circle $c_1$, $-1$ if circle $c_1$
  completely encloses circle $c_2$, 0 if the circles are completely disjoint, 1 if the circles are
  tangent with one intersection (stored in `p`), 2 if the circles intersect at two points (stored in
  `p` and `q`), 3 if the circles are equal and intersect at infinite points.
- `intersection_area(c1, c2)` returns the intersection area of circles $c_1$ and $c_2$.

Time Complexity:
- O(1) for all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

const double EPS = 1e-9, PI = acos(-1.0);

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define NE(a, b) (fabs((a) - (b)) > EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define GT(a, b) ((a) > (b) + EPS)
#define LE(a, b) ((a) <= (b) + EPS)
#define GE(a, b) ((a) >= (b) - EPS)

// SFINAE guard: valid only when Pt exposes numeric .x/.y members, so templated
// point overloads don't hijack double-argument calls.
template<class Pt>
using if_point = decltype(std::declval<const Pt &>().x, std::declval<const Pt &>().y, void());

// Circle and Line are floating-point by nature; this file computes in double. The
// point inputs (tangent's p, Line's two-point constructor) accept any Pt type, and
// the Point outputs are this file's double Point.
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

double norm(const Point &a) {
  return sqrt(sqnorm(a));
}

struct Circle {
  double h, k, r;

  Circle(double h, double k, double r) {
    this->h = h;
    this->k = k;
    this->r = r;
  }
};

struct Line {
  double a, b, c;

  Line() : a(0), b(0), c(0) {}

  Line(double a, double b, double c) {
    if (!EQ(b, 0)) {
      this->a = a / b;
      this->c = c / b;
      this->b = 1;
    } else {
      this->c = c / a;
      this->a = 1;
      this->b = 0;
    }
  }

  template<class Pt, class = if_point<Pt>>
  Line(const Pt &p, const Pt &q) : a(0), b(0), c(0) {
    if (EQ(p.x, q.x)) {
      if (NE(p.y, q.y)) {  // Vertical line.
        a = 1;
        b = 0;
        c = -p.x;
      }  // Else, invalid line.
    } else {
      a = -(double)(p.y - q.y) / (p.x - q.x);
      b = 1;
      c = -(a * p.x) - (b * p.y);
    }
  }
};

template<class Pt>
int tangent(const Circle &c, const Pt &p, Line *l1 = nullptr, Line *l2 = nullptr) {
  Point vop(p.x - c.h, p.y - c.k);
  if (EQ(sqnorm(vop), c.r * c.r)) {  // Point on an edge.
    if (l1 != nullptr) {             // Get perpendicular line through p.
      *l1 = Line(Point(c.h, c.k), p);
      *l1 = Line(-l1->b, l1->a, l1->b * p.x - l1->a * p.y);
    }
    return 0;
  }
  if (LE(sqnorm(vop), c.r * c.r)) {
    return -1;  // Point inside Circle, no intersection.
  }
  Point q(vop.x / c.r, vop.y / c.r);
  double n = sqnorm(q), d = q.y * sqrt(sqnorm(q) - 1.0);
  Point t1((q.x - d) / n, c.k), t2((q.x + d) / n, c.k);
  if (NE(q.y, 0)) {  // Common case.
    t1.y += c.r * (1.0 - t1.x * q.x) / q.y;
    t2.y += c.r * (1.0 - t2.x * q.x) / q.y;
  } else {  // Point at center horizontal, y = 0.
    d = c.r * sqrt(1.0 - t1.x * t1.x);
    t1.y += d;
    t2.y -= d;
  }
  t1.x = t1.x * c.r + c.h;
  t2.x = t2.x * c.r + c.h;
  // Note: here, t1 and t2 are the two points of tangencies
  if (l1 != nullptr && l2 != nullptr) {
    *l1 = Line(p, t1);
    *l2 = Line(p, t2);
  }
  return 1;
}

int intersection(const Circle &c, const Line &l, Point *p = nullptr, Point *q = nullptr) {
  double v = c.h * l.a + c.k * l.b + l.c;
  double aabb = l.a * l.a + l.b * l.b;
  double disc = v * v / aabb - c.r * c.r;
  if (disc > EPS) {
    return -1;
  }
  double x0 = -l.a * l.c / aabb, y0 = -l.b * v / aabb;
  if (disc > -EPS) {
    if (p != nullptr) {
      *p = Point(x0 + c.h, y0 + c.k);
    }
    return 0;
  }
  double k = sqrt(std::max(0.0, disc / -aabb));
  if (p != nullptr && q != nullptr) {
    *p = Point(x0 + k * l.b + c.h, y0 - k * l.a + c.k);
    *q = Point(x0 - k * l.b + c.h, y0 + k * l.a + c.k);
  }
  return 1;
}

int intersection(const Circle &c1, const Circle &c2, Point *p = nullptr, Point *q = nullptr) {
  if (EQ(c1.h, c2.h) && EQ(c1.k, c2.k)) {
    return EQ(c1.r, c2.r) ? 3 : (c1.r > c2.r ? -1 : -2);
  }
  Point d12(Point(c2.h - c1.h, c2.k - c1.k));
  double d = norm(d12);
  if (GT(d, c1.r + c2.r)) {
    return 0;
  }
  if (LT(d, fabs(c1.r - c2.r))) {
    return c1.r > c2.r ? -1 : -2;
  }
  double a = (c1.r * c1.r - c2.r * c2.r + d * d) / (2 * d);
  double x0 = c1.h + (d12.x * a / d), y0 = c1.k + (d12.y * a / d);
  double s = sqrt(c1.r * c1.r - a * a), rx = -d12.y * s / d, ry = d12.x * s / d;
  if (EQ(rx, 0) && EQ(ry, 0)) {
    if (p != nullptr) {
      *p = Point(x0, y0);
    }
    return 1;
  }
  if (p != nullptr && q != nullptr) {
    *p = Point(x0 - rx, y0 - ry);
    *q = Point(x0 + rx, y0 + ry);
  }
  return 2;
}

double intersection_area(const Circle &c1, const Circle &c2) {
  double r = std::min(c1.r, c2.r), R = std::max(c1.r, c2.r);
  double d = norm(Point(c2.h - c1.h, c2.k - c1.k));
  if (LE(d, R - r)) {
    return PI * r * r;
  }
  if (GE(d, R + r)) {
    return 0;
  }
  return r * r * acos((d * d + r * r - R * R) / 2 / d / r) +
         R * R * acos((d * d + R * R - r * r) / 2 / d / R) -
         0.5 * sqrt((-d + r + R) * (d + r - R) * (d - r + R) * (d + r + R));
}

/*** Example Usage ***/

#include <cassert>

bool EQL(const Line &l1, const Line &l2) {
  return EQ(l1.a, l2.a) && EQ(l1.b, l2.b) && EQ(l1.c, l2.c);
}

int main() {
  Line l1, l2;
  assert(-1 == tangent(Circle(0, 0, 4), Point(1, 1), &l1, &l2));
  assert(0 == tangent(Circle(0, 0, sqrt(2)), Point(1, 1), &l1, &l2));
  assert(EQL(l1, Line(-1, -1, 2)));
  assert(1 == tangent(Circle(0, 0, 2), Point(2, 2), &l1, &l2));
  assert(EQL(l1, Line(0, -2, 4)));
  assert(EQL(l2, Line(2, 0, -4)));

  Point p, q;
  assert(-1 == intersection(Circle(1, 1, 3), Line(5, 3, -30), &p, &q));
  assert(0 == intersection(Circle(1, 1, 3), Line(0, 1, -4), &p, &q));
  assert((p == Point{1, 4}));
  assert(1 == intersection(Circle(1, 1, 3), Line(0, 1, -1), &p, &q));
  assert((p == Point{4, 1}));
  assert((q == Point{-2, 1}));

  assert(-2 == intersection(Circle(1, 1, 1), Circle(0, 0, 3), &p, &q));
  assert(-1 == intersection(Circle(0, 0, 3), Circle(1, 1, 1), &p, &q));
  assert(0 == intersection(Circle(5, 0, 4), Circle(-5, 0, 4), &p, &q));
  assert(1 == intersection(Circle(-5, 0, 5), Circle(5, 0, 5), &p, &q));
  assert((p == Point{0, 0}));
  assert(2 == intersection(Circle(-0.5, 0, 1), Circle(0.5, 0, 1), &p, &q));
  assert((p == Point{0, -sqrt(3) / 2}));
  assert((q == Point{0, sqrt(3) / 2}));

  // Each circle passes through the other's center.
  double r = 3, a = intersection_area(Circle(-r / 2, 0, r), Circle(r / 2, 0, r));
  assert(EQ(a, r * r * (2 * PI / 3 - sqrt(3) / 2)));

  return 0;
}
