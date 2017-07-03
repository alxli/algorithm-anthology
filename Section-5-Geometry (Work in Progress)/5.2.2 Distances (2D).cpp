/*

Distance calculations in 2 dimensions between points, lines, and segments.
All operations are O(1) in time and space.

*/

#include <algorithm> /* std::max(), std::min() */
#include <cmath>     /* fabs(), sqrt() */
#include <utility>   /* std::pair */

typedef std::pair<double, double> point;
#define x first
#define y second

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps)  /* equal to */
#define LE(a, b) ((a) <= (b) + eps)        /* less than or equal to */
#define GE(a, b) ((a) >= (b) - eps)        /* greater than or equal to */

double norm(const point & a) { return a.x * a.x + a.y * a.y; }
double abs(const point & a) { return sqrt(norm(a)); }

//distance from point a to point b
double dist(const point & a, const point & b) {
  return abs(point(b.x - a.x, b.y - a.y));
}

//squared distance from point a to point b
double dist2(const point & a, const point & b) {
  return norm(point(b.x - a.x, b.y - a.y));
}

//minimum distance from point p to line l denoted by ax + by + c = 0
//if a = b = 0, then -inf, nan, or +inf is returned depending on sgn(c)
double dist_line(const point & p,
                 const double & a, const double & b, const double & c) {
  return fabs(a * p.x + b * p.y + c) / sqrt(a * a + b * b);
}

//minimum distance from point p to the infinite line containing a and b
//if a = b, then the point distance from p to the single point is returned
double dist_line(const point & p, const point & a, const point & b) {
  double ab2 = dist2(a, b);
  if (EQ(ab2, 0)) return dist(p, a);
  double u = ((p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y)) / ab2;
  return abs(point(a.x + u * (b.x - a.x) - p.x, a.y + u * (b.y - a.y) - p.y));
}

//distance between two lines each denoted by the form ax + by + c = 0
//if the lines are nonparallel, then the distance is 0, otherwise
//it is the perpendicular distance from a point on one line to the other
double dist_lines(const double & a1, const double & b1, const double & c1,
                  const double & a2, const double & b2, const double & c2) {
  if (EQ(a1 * b2, a2 * b1)) {
    double factor = EQ(b1, 0) ? (a1 / a2) : (b1 / b2);
    if (EQ(c1, c2 * factor)) return 0;
    return fabs(c2 * factor - c1) / sqrt(a1 * a1 + b1 * b1);
  }
  return 0;
}

//distance between two infinite lines respectively containing ab and cd
//same results as above, except we solve for the lines here first.
double dist_lines(const point & a, const point & b,
                  const point & c, const point & d) {
  double A1 = a.y - b.y, B1 = b.x - a.x;
  double A2 = c.y - d.y, B2 = d.x - c.x;
  double C1 = -A1 * a.x - B1 * a.y, C2 = -A2 * c.x - B2 * c.y;
  return dist_lines(A1, B1, C1, A2, B2, C2);
}

//minimum distance from point p to any point on segment ab
double dist_seg(const point & p, const point & a, const point & b) {
  if (a == b) return dist(p, a);
  point ab(b.x - a.x, b.y - a.y), ap(p.x - a.x, p.y - a.y);
  double n = norm(ab), d = ab.x * ap.x + ab.y * ap.y;
  if (LE(d, 0) || EQ(n, 0)) return abs(ap);
  if (GE(d, n)) return abs(point(ap.x - ab.x, ap.y - ab.y));
  return abs(point(ap.x - ab.x * (d / n), ap.y - ab.y * (d / n)));
}

double dot(const point & a, const point & b) { return a.x * b.x + a.y * b.y; }
double cross(const point & a, const point & b) { return a.x * b.y - a.y * b.x; }

//minimum distance from any point on segment ab to any point on segment cd
double dist_segs(const point & a, const point & b,
                 const point & c, const point & d) {
  //check if segments are touching or intersecting - if so, distance is 0
  point ab(b.x - a.x, b.y - a.y);
  point ac(c.x - a.x, c.y - a.y);
  point cd(d.x - c.x, d.y - c.y);
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) {
    double t0 = dot(ac, ab) / norm(ab);
    double t1 = t0 + dot(cd, ab) / norm(ab);
    if (LE(std::min(t0, t1), 1) && LE(0, std::max(t0, t1)))
      return 0;
  } else {
    double t = cross(ac, cd) / c1, u = c2 / c1;
    if (!EQ(c1, 0) && LE(0, t) && LE(t, 1) && LE(0, u) && LE(u, 1))
      return 0;
  }
  //find min distances across each endpoint to opposing segment
  return std::min(std::min(dist_seg(a, c, d), dist_seg(b, c, d)),
                  std::min(dist_seg(c, a, b), dist_seg(d, a, b)));
}

/*** Example Usage ***/

#include <cassert>
#define pt point

int main() {
  assert(EQ(5, dist(pt(-1, -1), pt(2, 3))));
  assert(EQ(25, dist2(pt(-1, -1), pt(2, 3))));
  assert(EQ(1.2, dist_line(pt(2, 1), -4, 3, -1)));
  assert(EQ(0.8, dist_line(pt(3, 3), pt(-1, -1), pt(2, 3))));
  assert(EQ(1.2, dist_line(pt(2, 1), pt(-1, -1), pt(2, 3))));
  assert(EQ(0.0, dist_lines(-4, 3, -1, 8, 6, 2)));
  assert(EQ(0.8, dist_lines(-4, 3, -1, -8, 6, -10)));
  assert(EQ(1.0, dist_seg(pt(3, 3), pt(-1, -1), pt(2, 3))));
  assert(EQ(1.2, dist_seg(pt(2, 1), pt(-1, -1), pt(2, 3))));
  assert(EQ(0.0, dist_segs(pt(0, 2), pt(3, 3), pt(-1, -1), pt(2, 3))));
  assert(EQ(0.6, dist_segs(pt(-1, 0), pt(-2, 2), pt(-1, -1), pt(2, 3))));
  return 0;
}
