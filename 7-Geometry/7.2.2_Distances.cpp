/*

Distance calculations in two dimensions for points, lines, and line segments. The functions are
templated on the point type `Pt`: pass any struct with numeric `.x` and `.y` fields (for example
`PointD`/ `PointI` from 7.1.1, or the local example struct). `sqnorm`, `dot`, `cross`, and `sqdist`
preserve the coordinate type and are exact for integer points. Functions that return an actual
distance use `double` for the final division/square root. `closest_point` returns a point of the
same type as its inputs, so use a floating-point `Pt` there for a meaningful (non-truncated) result.

- `dist(a, b)` and `sqdist(a, b)` respectively return the distance and squared distance between
  points `a` and `b`.
- `line_dist(p, a, b, c)` returns the distance from point `p` to the line $`a`x + `b`y + `c` = 0$.
- `line_dist(p, a, b)` returns the distance from point `p` to the infinite line containing points
  `a` and `b`. If `a` = `b`, returns the distance from `p` to `a`.
- `line_dist(a1, b1, c1, a2, b2, c2)` returns the distance between two lines.
- `seg_dist(p, a, b)` returns the distance from point `p` to the line segment $ab$.
- `seg_dist(a, b, c, d)` returns the minimum distance between line segments `ab` and `cd`. Returns 0
  if the segments touch or intersect.
- `closest_point(a, b, p)` returns the point on segment `ab` closest to point `p`.

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
#define GE(a, b) ((a) >= (b) - EPS)

// clang-format off
template<class Pt> auto sqnorm(const Pt &a) { return a.x*a.x + a.y*a.y; }
template<class Pt> double norm(const Pt &a) { return sqrt((double)sqnorm(a)); }
template<class Pt> auto dot(const Pt &a, const Pt &b) { return a.x*b.x + a.y*b.y; }
template<class Pt> auto cross(const Pt &a, const Pt &b) { return a.x*b.y - a.y*b.x; }
// clang-format on

// sqdist returns the coordinate type (exact for integer points).
template<class Pt>
auto sqdist(const Pt &a, const Pt &b) {
  auto dx = b.x - a.x, dy = b.y - a.y;
  return dx * dx + dy * dy;
}

template<class Pt>
double dist(const Pt &a, const Pt &b) {
  return sqrt((double)sqdist(a, b));
}

template<class Pt, class T>
double line_dist(const Pt &p, const T &a, const T &b, const T &c) {
  return fabs((double)a * p.x + (double)b * p.y + c) / sqrt((double)a * a + (double)b * b);
}

template<class Pt>
double line_dist(const Pt &p, const Pt &a, const Pt &b) {
  if (EQ(a.x, b.x) && EQ(a.y, b.y)) {
    return dist(p, a);
  }
  auto n = sqdist(a, b);
  auto d = (p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y);
  double u = (double)d / n;
  double dx = a.x + u * (b.x - a.x) - p.x, dy = a.y + u * (b.y - a.y) - p.y;
  return sqrt(dx * dx + dy * dy);
}

template<class T>
double line_dist(const T &a1, const T &b1, const T &c1, const T &a2, const T &b2, const T &c2) {
  if (EQ(a1 * b2, a2 * b1)) {
    double factor = EQ(b1, 0) ? ((double)a1 / a2) : ((double)b1 / b2);
    return EQ(c1, c2 * factor) ? 0
                               : fabs(c2 * factor - c1) / sqrt((double)a1 * a1 + (double)b1 * b1);
  }
  return 0;
}

template<class Pt>
double seg_dist(const Pt &p, const Pt &a, const Pt &b) {
  if (EQ(a.x, b.x) && EQ(a.y, b.y)) {
    return dist(p, a);
  }
  auto n = sqdist(a, b);
  auto d = (p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y);
  if (LE(d, 0) || EQ(n, 0)) {
    return dist(p, a);
  }
  if (GE(d, n)) {
    return dist(p, b);
  }
  double t = (double)d / n;
  double dx = a.x + t * (b.x - a.x) - p.x, dy = a.y + t * (b.y - a.y) - p.y;
  return sqrt(dx * dx + dy * dy);
}

template<class Pt>
double seg_dist(const Pt &a, const Pt &b, const Pt &c, const Pt &d) {
  if (EQ(a.x, b.x) && EQ(a.y, b.y)) {
    return seg_dist(a, c, d);
  }
  if (EQ(c.x, d.x) && EQ(c.y, d.y)) {
    return seg_dist(c, a, b);
  }
  auto ab_x = b.x - a.x, ab_y = b.y - a.y;
  auto ac_x = c.x - a.x, ac_y = c.y - a.y;
  auto cd_x = d.x - c.x, cd_y = d.y - c.y;
  auto c1 = ab_x * cd_y - ab_y * cd_x;
  auto c2 = ac_x * ab_y - ac_y * ab_x;
  if (EQ(c1, 0) && EQ(c2, 0)) {
    auto less = [](const Pt &u, const Pt &v) { return u.x != v.x ? u.x < v.x : u.y < v.y; };
    auto minp = [&](const Pt &u, const Pt &v) -> const Pt & { return less(v, u) ? v : u; };
    auto maxp = [&](const Pt &u, const Pt &v) -> const Pt & { return less(u, v) ? v : u; };
    const Pt &res1 = maxp(minp(a, b), minp(c, d));
    const Pt &res2 = minp(maxp(a, b), maxp(c, d));
    if (!less(res2, res1)) {
      return 0;
    }
  } else if (!EQ(c1, 0)) {
    auto t_num = ac_x * cd_y - ac_y * cd_x;
    bool c1_pos = c1 > 0;
    bool t_between_01 = c1_pos ? (LE(0, t_num) && LE(t_num, c1)) : (LE(t_num, 0) && LE(c1, t_num));
    bool u_between_01 = c1_pos ? (LE(0, c2) && LE(c2, c1)) : (LE(c2, 0) && LE(c1, c2));
    if (t_between_01 && u_between_01) {
      return 0;
    }
  }
  return std::min(
      std::min(seg_dist(a, c, d), seg_dist(b, c, d)), std::min(seg_dist(c, a, b), seg_dist(d, a, b))
  );
}

template<class Pt>
Pt closest_point(const Pt &a, const Pt &b, const Pt &p) {
  Pt res{};
  if (EQ(a.x, b.x) && EQ(a.y, b.y)) {
    res.x = a.x;
    res.y = a.y;
    return res;
  }
  auto n = sqdist(a, b);
  auto d = (p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y);
  double t = (double)d / n;
  if (t <= 0) {
    res.x = a.x;
    res.y = a.y;
  } else if (t >= 1) {
    res.x = b.x;
    res.y = b.y;
  } else {
    res.x = a.x + t * (b.x - a.x);
    res.y = a.y + t * (b.y - a.y);
  }
  return res;
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
  assert(EQ(5, dist(Point(-1, -1), Point(2, 3))));
  assert(EQ(25, sqdist(Point(-1, -1), Point(2, 3))));
  assert(EQ(1.2, line_dist(Point(2, 1), -4, 3, -1)));
  assert(EQ(0.8, line_dist(Point(3, 3), Point(-1, -1), Point(2, 3))));
  assert(EQ(1.2, line_dist(Point(2, 1), Point(-1, -1), Point(2, 3))));
  assert(EQ(0, line_dist(-4, 3, -1, 8, 6, 2)));
  assert(EQ(0.8, line_dist(-4, 3, -1, -8, 6, -10)));
  assert(EQ(1.0, seg_dist(Point(3, 3), Point(-1, -1), Point(2, 3))));
  assert(EQ(1.2, seg_dist(Point(2, 1), Point(-1, -1), Point(2, 3))));
  assert(EQ(0, seg_dist(Point(0, 2), Point(3, 3), Point(-1, -1), Point(2, 3))));
  assert(EQ(0.6, seg_dist(Point(-1, 0), Point(-2, 2), Point(-1, -1), Point(2, 3))));

  // Integer points: sqdist is exact, dist returns double.
  PointI ia{-1, -1}, ib{2, 3};
  assert(sqdist(ia, ib) == 25);   // int result
  assert(dot(ia, ib) == -5);      // int result
  assert(EQ(dist(ia, ib), 5.0));  // double result

  return 0;
}
