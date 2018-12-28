/*

Common triangle calculations in two dimensions.

- triangle_area(a, b, c) returns the area of the triangle abc.
- triangle_area_sides(s1, s2, s3) returns the area of a triangle with side
  lengths s1, s2, and s3. The given lengths must be non-negative and form a
  valid triangle.
- trinagle_area_medians(m1, m2, m3) returns the area of a triangle with medians
  of lengths m1, m2, and m3. The median of a triangle is a line segment joining
  a vertex to the midpoint of the opposing edge.
- triangle_area_altitudes(h1, h2, h3) returns the area of a triangle with
  altitudes h1, h2, and h3. An altitude of a triangle is the shortest line
  between a vertex and the infinite line that is extended from its opposite
  edge.
- same_side(p1, p2, a, b) returns whether points p1 and p2 lie on the same side
  of the line containing points a and b. If one or both points lie exactly on
  the line, then the result will depend on the setting of EDGE_IS_SAME_SIDE.
- point_in_triangle(p, a, b, c) returns whether point p lies within the triangle
  abc. If the point lies on or close to an edge (by roughly EPS), then the
  result will depend on the setting of EDGE_IS_SAME_SIDE in the function above.

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
#define LT(a, b) ((a) < (b) - EPS)
#define GT(a, b) ((a) > (b) + EPS)
#define LE(a, b) ((a) <= (b) + EPS)
#define GE(a, b) ((a) >= (b) - EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double cross(const point &a, const point &b) { return a.x*b.y - a.y*b.x; }

double triangle_area(const point &a, const point &b, const point &c) {
  point ac(a.x - c.x, a.y - c.y), bc(b.x - c.x, b.y - c.y);
  return fabs(cross(ac, bc)) / 2.0;
}

double triangle_area_sides(double s1, double s2, double s3) {
  double s = (s1 + s2 + s3) / 2.0;
  return sqrt(s*(s - s1)*(s - s2)*(s - s3));
}

double triangle_area_medians(double m1, double m2, double m3) {
  return 4.0*triangle_area_sides(m1, m2, m3) / 3.0;
}

double triangle_area_altitudes(double h1, double h2, double h3) {
  if (EQ(h1, 0) || EQ(h2, 0) || EQ(h3, 0)) {
    return 0;
  }
  double x = h1*h1, y = h2*h2, z = h3*h3;
  double v = 2.0/(x*y) + 2.0/(x*z) + 2.0/(y*z);
  return 1.0/sqrt(v - 1.0/(x*x) - 1.0/(y*y) - 1.0/(z*z));
}

bool same_side(const point &p1, const point &p2, const point &a,
               const point &b) {
  static const bool EDGE_IS_SAME_SIDE = true;
  point ab(b.x - a.x, b.y - a.y);
  point p1a(p1.x - a.x, p1.y - a.y), p2a(p2.x - a.x, p2.y - a.y);
  double c1 = cross(ab, p1a), c2 = cross(ab, p2a);
  return EDGE_IS_SAME_SIDE ? GE(c1*c2, 0) : GT(c1*c2, 0);
}

bool point_in_triangle(const point &p, const point &a, const point &b,
                       const point &c) {
  return same_side(p, a, b, c) &&
         same_side(p, b, a, c) &&
         same_side(p, c, a, b);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(EQ(6, triangle_area(point(0, -1), point(4, -1), point(0, -4))));
  assert(EQ(6, triangle_area_sides(3, 4, 5)));
  assert(EQ(6, triangle_area_medians(3.605551275, 2.5, 4.272001873)));
  assert(EQ(6, triangle_area_altitudes(3, 4, 2.4)));

  assert(point_in_triangle(point(0, 0),
                           point(-1, 0), point(0, -2), point(4, 0)));
  assert(!point_in_triangle(point(0, 1),
                            point(-1, 0), point(0, -2), point(4, 0)));
  assert(point_in_triangle(point(-2.44, 0.82),
                           point(-1, 0), point(-3, 1), point(4, 0)));
  assert(!point_in_triangle(point(-2.44, 0.7),
                            point(-1, 0), point(-3, 1), point(4, 0)));
  return 0;
}
