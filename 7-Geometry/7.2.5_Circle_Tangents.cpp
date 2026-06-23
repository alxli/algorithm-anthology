/*

Given two circles, compute their common tangent lines by returning the two points of tangency on
each circle. External tangents touch both circles on the same side of the line joining their
centers; internal tangents cross between the circles. A point can be treated as a zero-radius
circle, so this also covers tangents from a point to a circle.

- `circle_tangents(c1, r1, c2, r2, INTERNAL)` returns $0$, $1$, or $2$ tangent point pairs. Each
  pair `{p, q}` means the tangent line touches the first circle at `p` and the second circle at `q`.
  Set `INTERNAL = true` to get internal tangents; otherwise external tangents are returned.
- `all_circle_tangents(c1, r1, c2, r2)` returns all external and internal tangents.

If the two circles are identical, there are infinitely many tangents and the functions return an
empty vector. If one circle strictly contains the other, there are no external tangents; if the
circles overlap, there are no internal tangents.

Time Complexity:
- O(1) per call.

Space Complexity:
- O(1) auxiliary, excluding the returned vector.

*/

#include <algorithm>
#include <cmath>
#include <type_traits>
#include <utility>
#include <vector>

const double EPS = 1e-9;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return std::fabs(C(a) - C(b)) <= static_cast<C>(EPS);
  return C(a) == C(b);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool LT(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) < C(b) - static_cast<C>(EPS);
  return C(a) < C(b);
}

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  friend bool EQ(const Point &a, const Point &b) { return EQ(a.x, b.x) && EQ(a.y, b.y); }
  Point operator+(const Point &p) const { return {x + p.x, y + p.y}; }
  Point operator-(const Point &p) const { return {x - p.x, y - p.y}; }
  Point operator*(double k) const { return {x * k, y * k}; }
  Point operator/(double k) const { return {x / k, y / k}; }
  double sqnorm() const { return x * x + y * y; }
  Point rotate90() const { return {-y, x}; }
};

template<typename PtA, typename PtB>
std::vector<std::pair<Point, Point>> circle_tangents(
    const PtA &c1, double r1, const PtB &c2, double r2, const bool INTERNAL = false
) {
  r1 = fabs(r1);
  r2 = fabs(r2);
  if (INTERNAL) {
    r2 = -r2;
  }
  Point d(c2.x - c1.x, c2.y - c1.y);
  double dr = r1 - r2, d2 = d.sqnorm(), h2 = d2 - dr * dr;
  if (EQ(d2, 0) || LT(h2, 0)) {
    return {};
  }
  std::vector<std::pair<Point, Point>> res;
  for (double s : {-1.0, 1.0}) {
    Point v = (d * dr + d.rotate90() * (sqrt(std::max(0.0, h2)) * s)) / d2;
    res.push_back({Point(c1.x, c1.y) + v * r1, Point(c2.x, c2.y) + v * r2});
  }
  if (EQ(h2, 0)) {
    res.pop_back();
  }
  return res;
}

template<typename PtA, typename PtB>
std::vector<std::pair<Point, Point>> all_circle_tangents(
    const PtA &c1, double r1, const PtB &c2, double r2
) {
  auto res = circle_tangents(c1, r1, c2, r2);
  auto internal = circle_tangents(c1, r1, c2, r2, true);
  res.insert(res.end(), internal.begin(), internal.end());
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  Point a(0, 0), b(4, 0);
  auto ext = circle_tangents(a, 1, b, 1);
  assert(ext.size() == 2);
  assert(EQ(ext[0].first, Point(0, -1)) && EQ(ext[0].second, Point(4, -1)));
  assert(EQ(ext[1].first, Point(0, 1)) && EQ(ext[1].second, Point(4, 1)));

  auto in = circle_tangents(a, 1, b, 1, true);
  assert(in.size() == 2);
  assert(
      EQ(in[0].first, Point(0.5, -sqrt(3.0) / 2)) && EQ(in[0].second, Point(3.5, sqrt(3.0) / 2))
  );
  assert(
      EQ(in[1].first, Point(0.5, sqrt(3.0) / 2)) && EQ(in[1].second, Point(3.5, -sqrt(3.0) / 2))
  );

  auto point_to_circle = circle_tangents(Point(0, 0), 0, Point(2, 0), 1);
  assert(point_to_circle.size() == 2);
  assert(EQ(point_to_circle[0].first, Point(0, 0)));

  assert(circle_tangents(Point(0, 0), 5, Point(1, 0), 1).empty());        // contained
  assert(circle_tangents(Point(0, 0), 2, Point(3, 0), 2, true).empty());  // overlapping
  assert(all_circle_tangents(Point(0, 0), 1, Point(4, 0), 1).size() == 4);
  return 0;
}
