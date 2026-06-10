/*

Common triangle calculations in two dimensions. The functions are templated on the point type `Pt`,
which should work with `Point`/ `PointD`/`PointI` from 7.1.1 or any struct with numeric
`.x` and `.y` fields. `triangle_area()` returns `double` regardless of input type (the area is
fractional via the `/2`). `same_side()` and `point_in_triangle()` do all arithmetic in the point's
own coordinate type and are exact for integer points: they reduce each cross product to a sign
before combining, so no precision is lost and cross products are never multiplied together.

Note: with an integer point type, coordinates must be integers - fractional literals like `-2.44`
cannot be represented. Use a floating-point point type for fractional coordinates.

- `triangle_area(a, b, c)` returns the area of the triangle with vertices `a`, `b`, and `c`.
- `triangle_area_sides(s1, s2, s3)` returns the area of a triangle with side lengths `s1`, `s2`, and
  `s3`. The given lengths must be non-negative and form a valid triangle.
- `triangle_area_medians(m1, m2, m3)` returns the area of a triangle with medians of lengths `m1`,
  `m2`, and `m3`. The median of a triangle is a line segment joining a vertex to the midpoint of
  the opposing edge.
- `triangle_area_altitudes(h1, h2, h3)` returns the area of a triangle with altitudes `h1`, `h2`,
  and `h3`. An altitude of a triangle is the shortest line between a vertex and the infinite line
  that is extended from its opposite edge.
- `same_side(p1, p2, a, b)` returns whether points `p1` and `p2` lie on the same side of the line
  containing points `a` and `b`. If one or both points lie exactly on the line, then the result will
  depend on the setting of `EDGE_IS_SAME_SIDE`.
- `point_in_triangle(p, a, b, c)` returns whether point `p` lies within the triangle with vertices
  `a`, `b`, and `c`. If the point lies on or close to an edge (by roughly `EPS`), then the result
  will depend on the setting of `EDGE_IS_SAME_SIDE` in the function above.

Overflow warning: each individual cross product is still on the order of the squared coordinate
magnitude, so for integer point types use a 64-bit coordinate type (e.g. `PointL` from 7.1.1) once
coordinates exceed ~46000.

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
#define LE(a, b) ((a) <= (b) + EPS)

template<class Pt>
double triangle_area(const Pt &a, const Pt &b, const Pt &c) {
  double acx = a.x - c.x, acy = a.y - c.y, bcx = b.x - c.x, bcy = b.y - c.y;
  return fabs(acx * bcy - acy * bcx) / 2.0;
}

double triangle_area_sides(double s1, double s2, double s3) {
  double s = (s1 + s2 + s3) / 2.0;
  return sqrt(s * (s - s1) * (s - s2) * (s - s3));
}

double triangle_area_medians(double m1, double m2, double m3) {
  return 4.0 * triangle_area_sides(m1, m2, m3) / 3.0;
}

double triangle_area_altitudes(double h1, double h2, double h3) {
  if (EQ(h1, 0) || EQ(h2, 0) || EQ(h3, 0)) {
    return 0;
  }
  double x = h1 * h1, y = h2 * h2, z = h3 * h3;
  double v = 2.0 / (x * y) + 2.0 / (x * z) + 2.0 / (y * z);
  return 1.0 / sqrt(v - 1.0 / (x * x) - 1.0 / (y * y) - 1.0 / (z * z));
}

template<class Pt>
bool same_side(const Pt &p1, const Pt &p2, const Pt &a, const Pt &b) {
  static const bool EDGE_IS_SAME_SIDE = true;
  // Cross products in the point's native coordinate type (exact for integer points).
  auto c1 = (b.x - a.x) * (p1.y - a.y) - (b.y - a.y) * (p1.x - a.x);
  auto c2 = (b.x - a.x) * (p2.y - a.y) - (b.y - a.y) * (p2.x - a.x);
  // Compare the signs, not the product c1 * c2, to avoid integer overflow.
  int s1 = LT(0, c1) ? 1 : (LT(c1, 0) ? -1 : 0);
  int s2 = LT(0, c2) ? 1 : (LT(c2, 0) ? -1 : 0);
  return EDGE_IS_SAME_SIDE ? (s1 * s2 >= 0) : (s1 * s2 > 0);
}

template<class Pt>
bool point_in_triangle(const Pt &p, const Pt &a, const Pt &b, const Pt &c) {
  return same_side(p, a, b, c) && same_side(p, b, a, c) && same_side(p, c, a, b);
}

/*** Example Usage ***/

#include <cassert>

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
};

int main() {
  assert(EQ(6, triangle_area(Point(0, -1), Point(4, -1), Point(0, -4))));
  assert(EQ(6, triangle_area_sides(3, 4, 5)));
  assert(EQ(6, triangle_area_medians(3.605551275, 2.5, 4.272001873)));
  assert(EQ(6, triangle_area_altitudes(3, 4, 2.4)));

  // Fractional coordinates require a floating-point point type.
  assert(point_in_triangle(Point(0, 0), Point(-1, 0), Point(0, -2), Point(4, 0)));
  assert(!point_in_triangle(Point(0, 1), Point(-1, 0), Point(0, -2), Point(4, 0)));
  assert(point_in_triangle(Point(-2.44, 0.82), Point(-1, 0), Point(-3, 1), Point(4, 0)));
  assert(!point_in_triangle(Point(-2.44, 0.7), Point(-1, 0), Point(-3, 1), Point(4, 0)));

  // Integer coordinates: same_side / point_in_triangle are computed exactly in int.
  assert(EQ(8, triangle_area(PointI(0, 0), PointI(4, 0), PointI(0, 4))));
  assert(point_in_triangle(PointI(1, 1), PointI(0, 0), PointI(4, 0), PointI(0, 4)));
  assert(!point_in_triangle(PointI(5, 5), PointI(0, 0), PointI(4, 0), PointI(0, 4)));
  assert(point_in_triangle(PointI(2, 2), PointI(0, 0), PointI(4, 0), PointI(0, 4)));  // on edge
  return 0;
}
