/*

Distance calculations in two-dimensions for points, lines, and line segments.

- dist(a, b) and sqdist(a, b) respectively return the distance and squared
  distance between points a and b.
- line_dist(p, a, b, c) returns the distance from point p to the line
  a*x + b*y + c = 0. If the line is invalid (i.e. a = b = 0), then -INF, INF,
  or NaN is returned based on the sign of c.
- line_dist(p, a, b) returns the distance from point p to the infinite line
  containing points a and b. If the line is invalid (i.e. a = b), then the
  distance from p to the single point is returned.
- line_dist(a1, b1, c1, a2, b2, c2) returns the distance between two lines. If
  the lines are non-parallel then the distance is considered to be 0. Otherwise,
  the distance is considered to be the perpendicular distance from any point on
  one line to the other line.
- seg_dist(p, a, b) returns the distance from point p to the line segment ab.
- seg_dist(a, b, c, d) returns the minimum distance from any point on the line
  segment ab to any point on the line segment cd. This is 0 if the segments
  touch or intersect.

Time Complexity:
- O(1) for all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LE(a, b) ((a) <= (b) + EPS)
#define GE(a, b) ((a) >= (b) - EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double sqnorm(const point &a) { return a.x*a.x + a.y*a.y; }
double norm(const point &a) { return sqrt(sqnorm(a)); }
double dot(const point &a, const point &b) { return a.x*b.x + a.y*b.y; }
double cross(const point &a, const point &b) { return a.x*b.y - a.y*b.x; }

double dist(const point &a, const point &b) {
  return norm(point(b.x - a.x, b.y - a.y));
}

double sqdist(const point &a, const point &b) {
  return sqnorm(point(b.x - a.x, b.y - a.y));
}

double line_dist(const point &p, double a, double b, double c) {
  return fabs(a*p.x + b*p.y + c) / sqrt(a*a + b*b);
}

double line_dist(const point &p, const point &a, const point &b) {
  if (EQ(a.x, b.x) && EQ(a.y, b.y)) {
    return dist(p, a);
  }
  double u = ((p.x - a.x)*(b.x - a.x) + (p.y - a.y)*(b.y - a.y)) / sqdist(a, b);
  return norm(point(a.x + u*(b.x - a.x) - p.x, a.y + u*(b.y - a.y) - p.y));
}

double line_dist(double a1, double b1, double c1,
                 double a2, double b2, double c2) {
  if (EQ(a1*b2, a2*b1)) {
    double factor = EQ(b1, 0) ? (a1 / a2) : (b1 / b2);
    return EQ(c1, c2*factor) ? 0
                             : fabs(c2*factor - c1) / sqrt(a1*a1 + b1*b1);
  }
  return 0;
}

double seg_dist(const point &p, const point &a, const point &b) {
  if (EQ(a.x, b.x) && EQ(a.y, b.y)) {
    return dist(p, a);
  }
  point ab(b.x - a.x, b.y - a.y), ap(p.x - a.x, p.y - a.y);
  double n = sqnorm(ab), d = dot(ab, ap);
  if (LE(d, 0) || EQ(n, 0)) {
    return norm(ap);
  }
  return GE(d, n) ? norm(point(ap.x - ab.x, ap.y - ab.y))
                  : norm(point(ap.x - ab.x*(d / n), ap.y - ab.y*(d / n)));
}

double seg_dist(const point &a, const point &b,
                const point &c, const point &d) {
  point ab(b.x - a.x, b.y - a.y);
  point ac(c.x - a.x, c.y - a.y);
  point cd(d.x - c.x, d.y - c.y);
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) {
    double t0 = dot(ac, ab) / norm(ab), t1 = t0 + dot(cd, ab) / norm(ab);
    if (LE(std::min(t0, t1), 1) && LE(0, std::max(t0, t1))) {
      return 0;
    }
  } else {
    double t = cross(ac, cd) / c1, u = c2 / c1;
    if (!EQ(c1, 0) && LE(0, t) && LE(t, 1) && LE(0, u) && LE(u, 1)) {
      return 0;
    }
  }
  return std::min(std::min(seg_dist(a, c, d), seg_dist(b, c, d)),
                  std::min(seg_dist(c, a, b), seg_dist(d, a, b)));
}

point closest_point(const point &a, const point &b, const point &p) {
  if (EQ(a.x, b.x) && EQ(a.y, b.y)) {
    return a;
  }
  point ap(p.x - a.x, p.y - a.y), ab(b.x - a.x, b.y - a.y);
  double t = dot(ap, ab) / sqnorm(ab);
  return (t <= 0) ? a : ((t >= 1) ? b : point(a.x + t*ab.x, a.y + t*ab.y));
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(EQ(5, dist(point(-1, -1), point(2, 3))));
  assert(EQ(25, sqdist(point(-1, -1), point(2, 3))));
  assert(EQ(1.2, line_dist(point(2, 1), -4, 3, -1)));
  assert(EQ(0.8, line_dist(point(3, 3), point(-1, -1), point(2, 3))));
  assert(EQ(1.2, line_dist(point(2, 1), point(-1, -1), point(2, 3))));
  assert(EQ(0, line_dist(-4, 3, -1, 8, 6, 2)));
  assert(EQ(0.8, line_dist(-4, 3, -1, -8, 6, -10)));
  assert(EQ(1.0, seg_dist(point(3, 3), point(-1, -1), point(2, 3))));
  assert(EQ(1.2, seg_dist(point(2, 1), point(-1, -1), point(2, 3))));
  assert(EQ(0, seg_dist(point(0, 2), point(3, 3), point(-1, -1), point(2, 3))));
  assert(EQ(0.6,
            seg_dist(point(-1, 0), point(-2, 2), point(-1, -1), point(2, 3))));
  return 0;
}
