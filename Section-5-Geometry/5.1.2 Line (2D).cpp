/*

A 2D line is expressed in the form Ax + By + C = 0. All lines can be
"normalized" to a canonical form by insisting that the y-coefficient
equal 1 if it is non-zero. Otherwise, we set the x-coefficient to 1.
If B is non-zero, then we have the common case where the slope = -A
after normalization.

All operations are O(1) in time and space.

*/

#include <cmath>   /* fabs() */
#include <limits>  /* std::numeric_limits */
#include <ostream>
#include <utility> /* std::pair */

const double eps = 1e-9, NaN = std::numeric_limits<double>::quiet_NaN();

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define LT(a, b) ((a) < (b) - eps)        /* less than */

typedef std::pair<double, double> point;
#define x first
#define y second

struct line {

  double a, b, c;

  line(): a(0), b(0), c(0) {} //invalid or uninitialized line

  line(const double & A, const double & B, const double & C) {
    a = A;
    b = B;
    c = C;
    if (!EQ(b, 0)) {
      a /= b; c /= b; b = 1;
    } else {
      c /= a; a = 1; b = 0;
    }
  }

  line(const double & slope, const point & p) {
    a = -slope;
    b = 1;
    c = slope * p.x - p.y;
  }

  line(const point & p, const point & q): a(0), b(0), c(0) {
    if (EQ(p.x, q.x)) {
      if (EQ(p.y, q.y)) return; //invalid line
      //vertical line
      a = 1;
      b = 0;
      c = -p.x;
      return;
    }
    a = -(p.y - q.y) / (p.x - q.x);
    b = 1;
    c = -(a * p.x) - (b * p.y);
  }

  bool operator == (const line & l) const {
    return EQ(a, l.a) && EQ(b, l.b) && EQ(c, l.c);
  }

  bool operator != (const line & l) const {
    return !(*this == l);
  }

  //whether the line is initialized and normalized
  bool valid() const {
    if (EQ(a, 0)) return !EQ(b, 0);
    return EQ(b, 1) || (EQ(b, 0) && EQ(a, 1));
  }

  bool horizontal() const { return valid() && EQ(a, 0); }
  bool vertical() const { return valid() && EQ(b, 0); }

  double slope() const {
    if (!valid() || EQ(b, 0)) return NaN; //vertical
    return -a;
  }

  //solve for x, given y
  //for horizontal lines, either +inf, -inf, or nan is returned
  double x(const double & y) const {
    if (!valid() || EQ(a, 0)) return NaN; //invalid or horizontal
    return (-c - b * y) / a;
  }

  //solve for y, given x
  //for vertical lines, either +inf, -inf, or nan is returned
  double y(const double & x) const {
    if (!valid() || EQ(b, 0)) return NaN; //invalid or vertical
    return (-c - a * x) / b;
  }

  //returns whether p exists on the line
  bool contains(const point & p) const {
    return EQ(a * p.x + b * p.y + c, 0);
  }

  //returns whether the line is parallel to l
  bool parallel(const line & l) const {
    return EQ(a, l.a) && EQ(b, l.b);
  }

  //returns whether the line is perpendicular to l
  bool perpendicular(const line & l) const {
    return EQ(-a * l.a, b * l.b);
  }

  //return the parallel line passing through point p
  line parallel(const point & p) const {
    return line(a, b, -a * p.x - b * p.y);
  }

  //return the perpendicular line passing through point p
  line perpendicular(const point & p) const {
    return line(-b, a, b * p.x - a * p.y);
  }

  friend std::ostream & operator << (std::ostream & out, const line & l) {
    out << (fabs(l.a) < eps ? 0 : l.a) << "x" << std::showpos;
    out << (fabs(l.b) < eps ? 0 : l.b) << "y";
    out << (fabs(l.c) < eps ? 0 : l.c) << "=0" << std::noshowpos;
    return out;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  line l(2, -5, -8);
  line para = line(2, -5, -8).parallel(point(-6, -2));
  line perp = line(2, -5, -8).perpendicular(point(-6, -2));
  assert(l.parallel(para) && l.perpendicular(perp));
  assert(l.slope() == 0.4);
  assert(para == line(-0.4, 1, -0.4)); //-0.4x+1y-0.4=0
  assert(perp == line(2.5, 1, 17));    //2.5x+1y+17=0
  return 0;
}
