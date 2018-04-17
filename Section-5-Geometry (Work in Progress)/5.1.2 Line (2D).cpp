/*

A straight line in two dimensions supporting epsilon comparisons. The line is
represented by the form a*x + b*y + c = 0, where the coefficients are normalized
so that b is always 1 except for when the line is vertical, in which case b = 0.
Operations include checking if the line is horizontal or vertical, finding the
slope, evaluating y at some x (and vice versa), checking if a point falls on the
line, checking if another line is parallel or perpendicular, and finding the
parallel or perpendicular line through a point.

Time Complexity:
- O(1) per call to the constructor and all other operations.

Space Complexity:
- O(1) for storage of the line.
- O(1) auxiliary for all operations.

*/

#include <cmath>
#include <limits>
#include <ostream>

const double EPS = 1e-9;
const double M_NAN = std::numeric_limits<double>::quiet_NaN();

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)

struct line {
  double a, b, c;

  line() : a(0), b(0), c(0) {}  // Invalid or uninitialized line.

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

  template<class Point>
  line(double slope, const Point &p) {
    a = -slope;
    b = 1;
    c = slope * p.x - p.y;
  }

  template<class Point>
  line(const Point &p, const Point &q) : a(0), b(0), c(0) {
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

  bool operator==(const line &l) const {
    return EQ(a, l.a) && EQ(b, l.b) && EQ(c, l.c);
  }

  bool operator!=(const line &l) const {
    return !(*this == l);
  }

  // Returns whether the line is initialized and normalized.
  bool valid() const {
    if (EQ(a, 0)) {
      return !EQ(b, 0);
    }
    return EQ(b, 1) || (EQ(b, 0) && EQ(a, 1));
  }

  bool horizontal() const { return valid() && EQ(a, 0); }
  bool vertical() const { return valid() && EQ(b, 0); }
  double slope() const { return (!valid() || EQ(b, 0)) ? M_NAN : -a; }

  // Solve for x at a given y. If the line is horizontal, then either -INF, INF,
  // or NAN is returned based on whether y is below, above, or on the line.
  double x(double y) const {
    if (!valid() || EQ(a, 0)) {
      return M_NAN;  // Invalid or horizontal line.
    }
    return (-c - b*y) / a;
  }

  // Solve for y at a given x. If the line is vertical, then either -INF, INF,
  // or NAN is returned based on whether x is left of, right of, or on the line.
  double y(double x) const {
    if (!valid() || EQ(b, 0)) {
      return M_NAN;  // Invalid or vertical line.
    }
    return (-c - a*x) / b;
  }

  template<class Point>
  bool contains(const Point &p) const { return EQ(a*p.x + b*p.y + c, 0); }

  bool is_parallel(const line &l) const { return EQ(a, l.a) && EQ(b, l.b); }
  bool is_perpendicular(const line &l) const { return EQ(-a*l.a, b*l.b); }

  // Return the parallel line passing through point p.
  template<class Point>
  line parallel(const Point &p) const {
    return line(a, b, -a*p.x - b*p.y);
  }

  // Return the perpendicular line passing through point p.
  template<class Point>
  line perpendicular(const Point &p) const {
    return line(-b, a, b*p.x - a*p.y);
  }

  friend std::ostream& operator<<(std::ostream &out, const line &l) {
    return out << (fabs(l.a) < EPS ? 0 : l.a) << "x" << std::showpos
               << (fabs(l.b) < EPS ? 0 : l.b) << "y"
               << (fabs(l.c) < EPS ? 0 : l.c) << "=0" << std::noshowpos;
  }
};

/*** Example Usage ***/

#include <cassert>

struct point {
  double x, y;
  point(double x, double y) : x(x), y(y) {}
};

int main() {
  line l(2, -5, -8);
  line para = line(2, -5, -8).parallel(point(-6, -2));
  line perp = line(2, -5, -8).perpendicular(point(-6, -2));
  assert(l.is_parallel(para) && l.is_perpendicular(perp));
  assert(l.slope() == 0.4);
  assert(para == line(-0.4, 1, -0.4));  // -0.4x + y - 0.4 = 0.
  assert(perp == line(2.5, 1, 17));  // 2.5x + y + 17 = 0.
  return 0;
}
