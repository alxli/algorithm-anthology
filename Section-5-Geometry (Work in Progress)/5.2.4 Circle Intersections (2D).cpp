/*

Circle tangent and intersection calculations in two-dimensions.

- tangent(c, p, &l1, &l2) determines the line(s) tangent to circle c that passes
  through point p, returning -1 if there is no tangent line because p is
  strictly inside c, 0 if there is exactly one tangent line because p is on the
  boundary of c (in which case the line will be stored into pointer l1 if it's
  not NULL), or 1 if there are two tangent lines because p is strictly outside
  of c (in which case the lines will be stored into pointers l1 and l2 if they
  are not NULL).
- intersection(c, l, &p, &q) determines the intersection between the circle c
  and line l, returning -1 if there is no intersection, 0 if the line is one
  intersection point because the line is tangent (in which case it will be
  stored into pointer p if it's not NULL), or 1 if there are two intersection
  points because the line crosses through the circle (in which case they will be
  stored into pointers p and q if they are not NULL).
- intersection(c1, c2, &p, &q) determines the intersection between the circle c
  and line l, returning -1 if there is no intersection, 0 if the line is one
  intersection point because the line is tangent (in which case it will be
  stored into pointer p if it's not NULL), or 1 if there are two intersection
  points because the line crosses through the circle (in which case they will be
  stored into pointers p and q if they are not NULL).
- intersection_area(c1, c2) returns the intersection area of circles c1 and c2.

Time Complexity:
- O(1) for all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <utility>

const double EPS = 1e-9, PI = acos(-1.0);

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define NE(a, b) (fabs((a) - (b)) > EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define GT(a, b) ((a) > (b) + EPS)
#define LE(a, b) ((a) <= (b) + EPS)
#define GE(a, b) ((a) >= (b) - EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double sqnorm(const point &a) { return a.x*a.x + a.y*a.y; }
double norm(const point &a) { return sqrt(sqnorm(a)); }

struct circle {
  double h, k, r;

  circle(double h, double k, double r) {
    this->h = h;
    this->k = k;
    this->r = r;
  }
};

struct line {
  double a, b, c;

  line() : a(0), b(0), c(0) {}

  line(double a, double b, double c) {
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

  line(const point &p, const point &q) : a(0), b(0), c(0) {
    if (EQ(p.x, q.x)) {
      if (NE(p.y, q.y)) {  // Vertical line.
        a = 1;
        b = 0;
        c = -p.x;
      }  // Else, invalid line.
    } else {
      a = -(p.y - q.y) / (p.x - q.x);
      b = 1;
      c = -(a*p.x) - (b*p.y);
    }
  }
};

int tangent(const circle &c, const point &p, line *l1 = NULL, line *l2 = NULL) {
  point vop(p.x - c.h, p.y - c.k);
  if (EQ(sqnorm(vop), c.r*c.r)) {  // Point on an edge.
    if (l1 != 0) {  // Get perpendicular line through p.
      *l1 = line(point(c.h, c.k), p);
      *l1 = line(-l1->b, l1->a, l1->b*p.x - l1->a*p.y);
    }
    return 0;
  }
  if (LE(sqnorm(vop), c.r*c.r)) {
    return -1;  // Point inside circle, no intersection.
  }
  point q(vop.x / c.r, vop.y / c.r);
  double n = sqnorm(q), d = q.y*sqrt(sqnorm(q) - 1.0);
  point t1((q.x - d) / n, c.k), t2((q.x + d) / n, c.k);
  if (NE(q.y, 0)) {  // Common case.
    t1.y += c.r*(1.0 - t1.x*q.x) / q.y;
    t2.y += c.r*(1.0 - t2.x*q.x) / q.y;
  } else {  // Point at center horizontal, y = 0.
    d = c.r*sqrt(1.0 - t1.x*t1.x);
    t1.y += d;
    t2.y -= d;
  }
  t1.x = t1.x*c.r + c.h;
  t2.x = t2.x*c.r + c.h;
  //note: here, t1 and t2 are the two points of tangencies
  if (l1 != NULL && l2 != NULL) {
    *l1 = line(p, t1);
    *l2 = line(p, t2);
  }
  return 1;
}

int intersection(const circle &c, const line &l, point *p = NULL,
                 point *q = NULL) {
  double v = c.h*l.a + c.k*l.b + l.c;
  double aabb = l.a*l.a + l.b*l.b;
  double disc = v*v / aabb - c.r*c.r;
  if (disc > EPS) {
    return -1;
  }
  double x0 = -l.a*l.c / aabb, y0 = -l.b*v / aabb;
  if (disc > -EPS) {
    if (p != NULL) {
      *p = point(x0 + c.h, y0 + c.k);
    }
    return 0;
  }
  double k = sqrt(std::max(0.0, disc / -aabb));
  if (p != NULL && q != NULL) {
    *p = point(x0 + k*l.b + c.h, y0 - k*l.a + c.k);
    *q = point(x0 - k*l.b + c.h, y0 + k*l.a + c.k);
  }
  return 1;
}

int intersection(const circle &c1, const circle &c2, point *p = NULL,
                 point *q = NULL) {
  if (EQ(c1.h, c2.h) && EQ(c1.k, c2.k)) {
    return EQ(c1.r, c2.r) ? 3 : (c1.r > c2.r ? -1 : -2);
  }
  point d12(point(c2.h - c1.h, c2.k - c1.k));
  double d = norm(d12);
  if (GT(d, c1.r + c2.r)) {
    return 0;
  }
  if (LT(d, fabs(c1.r - c2.r))) {
    return c1.r > c2.r ? -1 : -2;
  }
  double a = (c1.r*c1.r - c2.r*c2.r + d*d) / (2*d);
  double x0 = c1.h + (d12.x*a / d), y0 = c1.k + (d12.y*a / d);
  double s = sqrt(c1.r*c1.r - a*a), rx = -d12.y*s / d, ry = d12.x*s / d;
  if (EQ(rx, 0) && EQ(ry, 0)) {
    if (p != NULL) {
      *p = point(x0, y0);
    }
    return 1;
  }
  if (p != NULL && q != NULL) {
    *p = point(x0 - rx, y0 - ry);
    *q = point(x0 + rx, y0 + ry);
  }
  return 2;
}

double intersection_area(const circle &c1, const circle &c2) {
  double r = std::min(c1.r, c2.r), R = std::max(c1.r, c2.r);
  double d = norm(point(c2.h - c1.h, c2.k - c1.k));
  if (LE(d, R - r)) {
    return PI*r*r;
  }
  if (GE(d, R + r)) {
    return 0;
  }
  return r*r*acos((d*d + r*r - R*R) / 2 / d / r) +
         R*R*acos((d*d + R*R - r*r) / 2 / d / R) -
         0.5*sqrt((-d + r + R)*(d + r - R)*(d - r + R)*(d + r + R));
}

/*** Example Usage ***/

#include <cassert>

bool EQP(const point &a, const point &b) {
  return EQ(a.x, b.x) && EQ(a.y, b.y);
}

bool EQL(const line &l1, const line &l2) {
  return EQ(l1.a, l2.a) && EQ(l1.b, l2.b) && EQ(l1.c, l2.c);
}

int main() {
  line l1, l2;
  assert(-1 == tangent(circle(0, 0, 4), point(1, 1), &l1, &l2));
  assert(0 == tangent(circle(0, 0, sqrt(2)), point(1, 1), &l1, &l2));
  assert(EQL(l1, line(-1, -1, 2)));
  assert(1 == tangent(circle(0, 0, 2), point(2, 2), &l1, &l2));
  assert(EQL(l1, line(0, -2, 4)));
  assert(EQL(l2, line(2, 0, -4)));

  point p, q;
  assert(-1 == intersection(circle(1, 1, 3), line(5, 3, -30), &p, &q));
  assert(0 == intersection(circle(1, 1, 3), line(0, 1, -4), &p, &q));
  assert(EQP(p, point(1, 4)));
  assert(1 == intersection(circle(1, 1, 3), line(0, 1, -1), &p, &q));
  assert(EQP(p, point(4, 1)));
  assert(EQP(q, point(-2, 1)));

  assert(-2 == intersection(circle(1, 1, 1), circle(0, 0, 3), &p, &q));
  assert(-1 == intersection(circle(0, 0, 3), circle(1, 1, 1), &p, &q));
  assert(0 == intersection(circle(5, 0, 4), circle(-5, 0, 4), &p, &q));
  assert(1 == intersection(circle(-5, 0, 5), circle(5, 0, 5), &p, &q));
  assert(EQP(p, point(0, 0)));
  assert(2 == intersection(circle(-0.5, 0, 1), circle(0.5, 0, 1), &p, &q));
  assert(EQP(p, point(0, -sqrt(3) / 2)));
  assert(EQP(q, point(0, sqrt(3) / 2)));

  // Each circle passes through the other's center.
  double r = 3, a = intersection_area(circle(-r/2, 0, r), circle(r/2, 0, r));
  assert(EQ(a, r*r*(2*PI / 3 - sqrt(3) / 2)));
  return 0;
}
