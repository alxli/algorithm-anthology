/*

Tangent lines to circles, circle-line intersections, and circle-circle
intersections (intersection point(s) as well as area) in 2 dimensions.

All operations are O(1) in time and space.

*/

#include <algorithm> /* std::min(), std::max() */
#include <cmath>     /* acos(), fabs(), sqrt() */
#include <utility>   /* std::pair */

typedef std::pair<double, double> point;
#define x first
#define y second

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define NE(a, b) (fabs((a) - (b)) > eps)  /* not equal to */
#define LT(a, b) ((a) < (b) - eps)        /* less than */
#define GT(a, b) ((a) > (b) + eps)        /* greater than */
#define LE(a, b) ((a) <= (b) + eps)       /* less than or equal to */
#define GE(a, b) ((a) >= (b) - eps)       /* greater than or equal to */

struct circle {
  double h, k, r;

  circle(const double & h, const double & k, const double & r) {
    this->h = h;
    this->k = k;
    this->r = r;
  }
};

//note: this is a simplified version of line that is not canonicalized.
// e.g. comparing lines with == signs will not work as intended. For a
//      fully featured line class, see the whole geometry library.
struct line {
  double a, b, c;

  line() { a = b = c = 0; }

  line(const double & a, const double & b, const double & c) {
    this->a = a;
    this->b = b;
    this->c = c;
  }

  line(const point & p, const point & q) {
    a = p.y - q.y,
    b = q.x - p.x;
    c = -a * p.x - b * p.y;
  }
};

double norm(const point & a) { return a.x * a.x + a.y * a.y; }
double abs(const point & a) { return sqrt(norm(a)); }
double dot(const point & a, const point & b) { return a.x * b.x + a.y * b.y; }

//tangent line(s) to circle c passing through p. there are 3 cases:
//returns: 0, if there are no lines (p is strictly inside c)
//         1, if there is 1 tangent line (p is on the edge)
//         2, if there are 2 tangent lines (p is strictly outside)
//If there is only 1 tangent, then the line will be stored in l1.
//If there are 2, then they will be stored in l1 and l2 respectively.
int tangents(const circle & c, const point & p, line * l1 = 0, line * l2 = 0) {
  point vop(p.x - c.h, p.y - c.k);
  if (EQ(norm(vop), c.r * c.r)) { //on an edge, get perpendicular through p
    if (l1 != 0) {
      *l1 = line(point(c.h, c.k), p);
      *l1 = line(-l1->b, l1->a, l1->b * p.x - l1->a * p.y);
    }
    return 1;
  }
  if (LE(norm(vop), c.r * c.r)) return 0; //inside circle
  point q(vop.x / c.r, vop.y / c.r);
  double n = norm(q), d = q.y * sqrt(norm(q) - 1.0);
  point t1((q.x - d) / n, c.k), t2((q.x + d) / n, c.k);
  if (NE(q.y, 0)) { //common case
    t1.y += c.r * (1.0 - t1.x * q.x) / q.y;
    t2.y += c.r * (1.0 - t2.x * q.x) / q.y;
  } else { //point at center horizontal, y = 0
    d = c.r * sqrt(1.0 - t1.x * t1.x);
    t1.y += d;
    t2.y -= d;
  }
  t1.x = t1.x * c.r + c.h;
  t2.x = t2.x * c.r + c.h;
  //note: here, t1 and t2 are the two points of tangencies
  if (l1 != 0) *l1 = line(p, t1);
  if (l2 != 0) *l2 = line(p, t2);
  return 2;
}

//determines the intersection(s) between a circle c and line l
//returns: 0, if the line does not intersect with the circle
//         1, if the line is tangent (one intersection)
//         2, if the line crosses through the circle
//If there is 1 intersection point, it will be stored in p
//If there are 2, they will be stored in p and q respectively
int intersection(const circle & c, const line & l,
                 point * p = 0, point * q = 0) {
  double v = c.h * l.a + c.k * l.b + l.c;
  double aabb = l.a * l.a + l.b * l.b;
  double disc = v * v / aabb - c.r * c.r;
  if (disc > eps) return 0;
  double x0 = -l.a * l.c / aabb, y0 = -l.b * v / aabb;
  if (disc > -eps) {
    if (p != 0) *p = point(x0 + c.h, y0 + c.k);
    return 1;
  }
  double k = sqrt((disc /= -aabb) < 0 ? 0 : disc);
  if (p != 0) *p = point(x0 + k * l.b + c.h, y0 - k * l.a + c.k);
  if (q != 0) *q = point(x0 - k * l.b + c.h, y0 + k * l.a + c.k);
  return 2;
}

//determines the intersection points between two circles c1 and c2
//returns: -2, if circle c2 completely encloses circle c1
//         -1, if circle c1 completely encloses circle c2
//          0, if the circles are completely disjoint
//          1, if the circles are tangent (one intersection point)
//          2, if the circles intersect at two points
//          3, if the circles intersect at infinite points (c1 = c2)
//If one intersection, the intersection point is stored in p
//If two, the intersection points are stored in p and q respectively
int intersection(const circle & c1, const circle & c2,
                 point * p = 0, point * q = 0) {
  if (EQ(c1.h, c2.h) && EQ(c1.k, c2.k))
    return EQ(c1.r, c2.r) ? 3 : (c1.r > c2.r ? -1 : -2);
  point d12(point(c2.h - c1.h, c2.k - c1.k));
  double d = abs(d12);
  if (GT(d, c1.r + c2.r)) return 0;
  if (LT(d, fabs(c1.r - c2.r))) return c1.r > c2.r ? -1 : -2;
  double a = (c1.r * c1.r - c2.r * c2.r + d * d) / (2 * d);
  double x0 = c1.h + (d12.x * a / d);
  double y0 = c1.k + (d12.y * a / d);
  double s = sqrt(c1.r * c1.r - a * a);
  double rx = -d12.y * s / d, ry = d12.x * s / d;
  if (EQ(rx, 0) && EQ(ry, 0)) {
    if (p != 0) *p = point(x0, y0);
    return 1;
  }
  if (p != 0) *p = point(x0 - rx, y0 - ry);
  if (q != 0) *q = point(x0 + rx, y0 + ry);
  return 2;
}

const double PI = acos(-1.0);

//intersection area of circles c1 and c2
double intersection_area(const circle & c1, const circle c2) {
  double r = std::min(c1.r, c2.r), R = std::max(c1.r, c2.r);
  double d = abs(point(c2.h - c1.h, c2.k - c1.k));
  if (LE(d, R - r)) return PI * r * r;
  if (GE(d, R + r)) return 0;
  return r * r * acos((d * d + r * r - R * R) / 2 / d / r) +
         R * R * acos((d * d + R * R - r * r) / 2 / d / R) -
         0.5 * sqrt((-d + r + R) * (d + r - R) * (d - r + R) * (d + r + R));
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;
#define pt point

int main() {
  line l1, l2;
  assert(0 == tangents(circle(0, 0, 4), pt(1, 1), &l1, &l2));
  assert(1 == tangents(circle(0, 0, sqrt(2)), pt(1, 1), &l1, &l2));
  cout << l1.a << " " << l1.b << " " << l1.c << "\n"; // -x - y + 2 = 0
  assert(2 == tangents(circle(0, 0, 2), pt(2, 2), &l1, &l2));
  cout << l1.a << " " << l1.b << " " << l1.c << "\n"; //    -2y + 4 = 0
  cout << l2.a << " " << l2.b << " " << l2.c << "\n"; // 2x     - 4 = 0

  pt p, q;
  assert(0 == intersection(circle(1, 1, 3), line(5, 3, -30), &p, &q));
  assert(1 == intersection(circle(1, 1, 3), line(0, 1, -4), &p, &q));
  assert(p == pt(1, 4));
  assert(2 == intersection(circle(1, 1, 3), line(0, 1, -1), &p, &q));
  assert(p == pt(4, 1));
  assert(q == pt(-2, 1));

  assert(-2 == intersection(circle(1, 1, 1), circle(0, 0, 3), &p, &q));
  assert(-1 == intersection(circle(0, 0, 3), circle(1, 1, 1), &p, &q));
  assert(0 == intersection(circle(5, 0, 4), circle(-5, 0, 4), &p, &q));
  assert(1 == intersection(circle(-5, 0, 5), circle(5, 0, 5), &p, &q));
  assert(p == pt(0, 0));
  assert(2 == intersection(circle(-0.5, 0, 1), circle(0.5, 0, 1), &p, &q));
  assert(p == pt(0, -sqrt(3) / 2));
  assert(q == pt(0, sqrt(3) / 2));

  //example where each circle passes through the other circle's center
  //http://math.stackexchange.com/a/402891
  double r = 3;
  double a = intersection_area(circle(-r / 2, 0, r), circle(r / 2, 0, r));
  assert(EQ(a, r * r * (2 * PI / 3 - sqrt(3) / 2)));
  return 0;
}
