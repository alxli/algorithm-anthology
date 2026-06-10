/*

Circle tangent and intersection calculations in two dimensions. Such calculations are floating-point
by nature, so this section mostly computes in double. The input point `p` to `tangent()` can be any
type with numeric `.x` and `.y` fields, and intersection point outputs are written through a
caller-supplied point type.

- `tangent(c, p, &l1, &l2)` determines the line(s) tangent to circle `c` that pass through point
  `p`, returning $-1$ if there is no tangent line because `p` is strictly inside `c`, 0 if there is
  exactly one tangent line because `p` is on the boundary of `c` (in which case the line will be
  stored into pointer `l1` if it's not `nullptr`), or 1 if there are two tangent lines because `p`
  is strictly outside of `c` (in which case the lines will be stored into pointers `l1` and `l2` if
  they are not `nullptr`).
- `intersection(c, l, &p, &q)` determines the intersection between the circle `c` and line `l`,
  returning $-1$ if there is no intersection, 0 if the line has one intersection point because the
  line is tangent (in which case it will be stored into pointer `p` if it's not `nullptr`), or 1 if
  there are two intersection points because the line crosses through the circle (in which case they
  will be stored into pointers `p` and `q` if they are not `nullptr`).
- `intersection(c1, c2, &p, &q)` determines the intersection points between two circles `c1` and
  `c2`, returning $-2$ if circle `c2` completely encloses circle `c1`, $-1$ if circle `c1`
  completely encloses circle `c2`, 0 if the circles are completely disjoint, 1 if the circles are
  tangent with one intersection (stored in `p` if it's not `nullptr`), 2 if the circles intersect at
  two points (stored in `p` and `q` if they are not `nullptr`), 3 if the circles are equal and
  intersect at infinite points.
- `intersection_area(c1, c2)` returns the intersection area of circles `c1` and `c2`.

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
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

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

  Line(double px, double py, double qx, double qy) : a(0), b(0), c(0) {
    if (EQ(px, qx)) {
      if (!EQ(py, qy)) {  // Vertical line.
        a = 1;
        b = 0;
        c = -px;
      }  // Else, invalid line.
    } else {
      a = -(double)(py - qy) / (px - qx);
      b = 1;
      c = -(a * px) - (b * py);
    }
  }

  bool operator==(const Line &l) const { return EQ(a, l.a) && EQ(b, l.b) && EQ(c, l.c); }
};

template<class Pt>
int tangent(const Circle &c, const Pt &p, Line *l1 = nullptr, Line *l2 = nullptr) {
  auto sqnorm = [](double x, double y) { return x * x + y * y; };
  double vopx = p.x - c.h, vopy = p.y - c.k;
  if (EQ(sqnorm(vopx, vopy), c.r * c.r)) {  // Point on an edge.
    if (l1 != nullptr) {                    // Tangent through p is perpendicular to the radius cp.
      *l1 = Line(vopx, vopy, -(vopx * p.x + vopy * p.y));
    }
    return 0;
  }
  if (LE(sqnorm(vopx, vopy), c.r * c.r)) {
    return -1;  // Point inside Circle, no intersection.
  }
  double qx = vopx / c.r, qy = vopy / c.r;
  double n = sqnorm(qx, qy), d = qy * sqrt(sqnorm(qx, qy) - 1.0);
  double t1x = (qx - d) / n, t1y = c.k;
  double t2x = (qx + d) / n, t2y = c.k;
  if (!EQ(qy, 0)) {  // Common case.
    t1y += c.r * (1.0 - t1x * qx) / qy;
    t2y += c.r * (1.0 - t2x * qx) / qy;
  } else {  // Point at center horizontal, y = 0.
    d = c.r * sqrt(1.0 - t1x * t1x);
    t1y += d;
    t2y -= d;
  }
  t1x = t1x * c.r + c.h;
  t2x = t2x * c.r + c.h;
  // Note: here, t1 and t2 are the two points of tangencies
  if (l1 != nullptr) {
    *l1 = Line(p.x, p.y, t1x, t1y);
  }
  if (l2 != nullptr) {
    *l2 = Line(p.x, p.y, t2x, t2y);
  }
  return 1;
}

template<class OutPt>
int intersection(const Circle &c, const Line &l, OutPt *p = nullptr, OutPt *q = nullptr) {
  double v = c.h * l.a + c.k * l.b + l.c;
  double aabb = l.a * l.a + l.b * l.b;
  double disc = v * v / aabb - c.r * c.r;
  if (disc > EPS) {
    return -1;
  }
  double x0 = -l.a * v / aabb, y0 = -l.b * v / aabb;
  if (disc > -EPS) {
    if (p != nullptr) {
      p->x = x0 + c.h;
      p->y = y0 + c.k;
    }
    return 0;
  }
  double k = sqrt(std::max(0.0, disc / -aabb));
  if (p != nullptr && q != nullptr) {
    p->x = x0 + k * l.b + c.h;
    p->y = y0 - k * l.a + c.k;
    q->x = x0 - k * l.b + c.h;
    q->y = y0 + k * l.a + c.k;
  }
  return 1;
}

template<class OutPt>
int intersection(const Circle &c1, const Circle &c2, OutPt *p = nullptr, OutPt *q = nullptr) {
  if (EQ(c1.h, c2.h) && EQ(c1.k, c2.k)) {
    return EQ(c1.r, c2.r) ? 3 : (c1.r > c2.r ? -1 : -2);
  }
  double d12x = c2.h - c1.h, d12y = c2.k - c1.k;
  double d = hypot(d12x, d12y);
  if (LT(c1.r + c2.r, d)) {
    return 0;
  }
  if (LT(d, fabs(c1.r - c2.r))) {
    return c1.r > c2.r ? -1 : -2;
  }
  double a = (c1.r * c1.r - c2.r * c2.r + d * d) / (2 * d);
  double x0 = c1.h + (d12x * a / d), y0 = c1.k + (d12y * a / d);
  double s = sqrt(c1.r * c1.r - a * a), rx = -d12y * s / d, ry = d12x * s / d;
  if (EQ(rx, 0) && EQ(ry, 0)) {
    if (p != nullptr) {
      p->x = x0;
      p->y = y0;
    }
    return 1;
  }
  if (p != nullptr && q != nullptr) {
    p->x = x0 - rx;
    p->y = y0 - ry;
    q->x = x0 + rx;
    q->y = y0 + ry;
  }
  return 2;
}

double intersection_area(const Circle &c1, const Circle &c2) {
  double r = std::min(c1.r, c2.r), R = std::max(c1.r, c2.r);
  double d = std::hypot(c2.h - c1.h, c2.k - c1.k);
  if (LE(d, R - r)) {
    return PI * r * r;
  }
  if (LE(R + r, d)) {
    return 0;
  }
  double alpha = std::max(-1.0, std::min(1.0, (d * d + r * r - R * R) / 2 / d / r));
  double beta = std::max(-1.0, std::min(1.0, (d * d + R * R - r * r) / 2 / d / R));
  return r * r * acos(alpha) + R * R * acos(beta) -
         0.5 * sqrt((-d + r + R) * (d + r - R) * (d - r + R) * (d + r + R));
}

/*** Example Usage ***/

#include <cassert>

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
};

int main() {
  Line l1, l2;
  assert(-1 == tangent(Circle(0, 0, 4), Point(1, 1), &l1, &l2));
  assert(0 == tangent(Circle(0, 0, sqrt(2)), Point(1, 1), &l1, &l2));
  assert(l1 == Line(-1, -1, 2));
  assert(1 == tangent(Circle(0, 0, 2), Point(2, 2), &l1, &l2));
  assert(l1 == Line(0, -2, 4));
  assert(l2 == Line(2, 0, -4));

  Point p, q;
  assert(-1 == intersection(Circle(1, 1, 3), Line(5, 3, -30), &p, &q));
  assert(0 == intersection(Circle(1, 1, 3), Line(0, 1, -4), &p, &q));
  assert(p == Point(1, 4));
  assert(1 == intersection(Circle(1, 1, 3), Line(0, 1, -1), &p, &q));
  assert(p == Point(4, 1));
  assert(q == Point(-2, 1));

  assert(-2 == intersection(Circle(1, 1, 1), Circle(0, 0, 3), &p, &q));
  assert(-1 == intersection(Circle(0, 0, 3), Circle(1, 1, 1), &p, &q));
  assert(0 == intersection(Circle(5, 0, 4), Circle(-5, 0, 4), &p, &q));
  assert(1 == intersection(Circle(-5, 0, 5), Circle(5, 0, 5), &p, &q));
  assert(p == Point(0, 0));
  assert(2 == intersection(Circle(-0.5, 0, 1), Circle(0.5, 0, 1), &p, &q));
  assert(p == Point(0, -sqrt(3) / 2));
  assert(q == Point(0, sqrt(3) / 2));

  // Each circle passes through the other's center.
  double r = 3, a = intersection_area(Circle(-r / 2, 0, r), Circle(r / 2, 0, r));
  assert(EQ(a, r * r * (2 * PI / 3 - sqrt(3) / 2)));
  return 0;
}
