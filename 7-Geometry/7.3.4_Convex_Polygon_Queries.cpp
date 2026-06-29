/*

Given a convex polygon in counter-clockwise order, answer containment and tangent/intersection
queries. Point containment is logarithmic by splitting the polygon into triangles sharing `poly[0]`.
The line and tangent helpers below are written as simple linear scans; this is often shorter and
less assumption-heavy than the classic logarithmic versions, while still documenting the exact query
semantics in one place.

- `point_in_convex_polygon(poly, p, edge_is_inside = true)` returns whether point `p` lies inside
  the convex polygon `poly`. The polygon must be in counter-clockwise order, with no repeated final
  vertex. Points on the boundary count as inside unless `edge_is_inside` is set to `false`.
- `line_convex_polygon_intersection(poly, a, b)` describes where the infinite directed line through
  `a` $\to$ `b` hits the polygon: $(-1, -1)$ for no intersection, $(i, -1)$ for touching vertex $i$,
  $(i, i)$ for containing side $i \to i+1$, or $(i, j)$ for crossing sides $i \to i+1$ and
  $j \to j+1$.
- `convex_polygon_tangents(poly, p)` returns the two tangent vertex indices from an outside point
  `p` as (`left`, `right`). The left tangent has all polygon vertices on or to the left of the
  directed line `p` $\to$ `poly[left]`; the right tangent is analogous for the right side.

For integer-coordinate inputs, all tests are exact provided the cross products do not overflow.

Time Complexity:
- O(log n) per query, where $n$ is the number of polygon vertices.
- O(n) per call to `line_convex_polygon_intersection` and `convex_polygon_tangents`.

Space Complexity:
- O(1) auxiliary.

*/

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

template<typename Pt>
auto cross(const Pt &a, const Pt &b, const Pt &o) {
  // Overflow risk for integer Pt: these products are ~O(max_coord^2); use int64_t if necessary.
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

template<typename T>
int sgn(const T &x) {
  return (T(0) < x) - (x < T(0));
}

template<typename Pt>
bool on_segment(const Pt &p, const Pt &a, const Pt &b) {
  return cross(a, b, p) == 0 && std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x) &&
         std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
}

template<typename Pt>
bool point_in_convex_polygon(
    const std::vector<Pt> &poly, const Pt &p, const bool edge_is_inside = true
) {
  int n = static_cast<int>(poly.size());
  if (n == 0) {
    return false;
  }
  if (n == 1) {
    return edge_is_inside && p.x == poly[0].x && p.y == poly[0].y;
  }
  if (n == 2) {
    return edge_is_inside && on_segment(p, poly[0], poly[1]);
  }
  int left = sgn(cross(poly[1], p, poly[0]));
  int right = sgn(cross(poly[n - 1], p, poly[0]));
  if (left < 0 || right > 0) {
    return false;
  }
  if (left == 0) {
    return edge_is_inside && on_segment(p, poly[0], poly[1]);
  }
  if (right == 0) {
    return edge_is_inside && on_segment(p, poly[0], poly[n - 1]);
  }
  int lo = 1, hi = n - 1;
  while (hi - lo > 1) {
    int mid = (lo + hi) / 2;
    if (cross(poly[mid], p, poly[0]) >= 0) {
      lo = mid;
    } else {
      hi = mid;
    }
  }
  int s = sgn(cross(poly[lo + 1], p, poly[lo]));
  return edge_is_inside ? s >= 0 : s > 0;
}

template<typename Pt>
std::pair<int, int> line_convex_polygon_intersection(
    const std::vector<Pt> &poly, const Pt &a, const Pt &b
) {
  int n = static_cast<int>(poly.size());
  std::vector<int> on, cross_edges;
  bool has_pos = false, has_neg = false;
  for (int i = 0; i < n; i++) {
    int s = sgn(cross(b, poly[i], a));
    has_pos |= s > 0;
    has_neg |= s < 0;
    if (s == 0) {
      on.push_back(i);
    }
  }
  if (!has_pos && !has_neg) {
    return n >= 2 ? std::make_pair(0, 0) : std::make_pair(-1, -1);
  }
  if (!(has_pos && has_neg)) {
    for (int i = 0; i < static_cast<int>(on.size()); i++) {
      int j = (i + 1) % static_cast<int>(on.size());
      if ((on[i] + 1) % n == on[j]) {
        return {on[i], on[i]};
      }
    }
    return on.empty() ? std::make_pair(-1, -1) : std::make_pair(on[0], -1);
  }
  for (int i = 0; i < n; i++) {
    int j = (i + 1) % n;
    int si = sgn(cross(b, poly[i], a)), sj = sgn(cross(b, poly[j], a));
    if (si == 0 || si * sj < 0) {
      cross_edges.push_back(i);
    }
  }
  if (cross_edges.size() == 1) {
    return {cross_edges[0], -1};
  }
  auto line_parameter = [&](int i) {
    int j = (i + 1) % n;
    auto ex = poly[j].x - poly[i].x, ey = poly[j].y - poly[i].y;
    auto ax = a.x, ay = a.y, dx = b.x - a.x, dy = b.y - a.y;
    auto det = dx * ey - dy * ex;
    return static_cast<double>((poly[i].x - ax) * ey - (poly[i].y - ay) * ex) / det;
  };
  if (line_parameter(cross_edges[1]) < line_parameter(cross_edges[0])) {
    std::swap(cross_edges[0], cross_edges[1]);
  }
  return {cross_edges[0], cross_edges[1]};
}

template<typename Pt>
std::pair<int, int> convex_polygon_tangents(const std::vector<Pt> &poly, const Pt &p) {
  int n = static_cast<int>(poly.size());
  std::pair<int, int> res{-1, -1};
  for (int i = 0; i < n; i++) {
    bool left = true, right = true;
    for (int j = 0; j < n; j++) {
      int s = sgn(cross(poly[i], poly[j], p));
      left &= s >= 0;
      right &= s <= 0;
    }
    if (left) {
      res.first = i;
    }
    if (right) {
      res.second = i;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <algorithm>
using namespace std;

struct PointI {
  int64_t x, y;
  PointI(int64_t x = 0, int64_t y = 0) : x(x), y(y) {}
};

int main() {
  vector<PointI> square{{0, 0}, {4, 0}, {4, 4}, {0, 4}};
  assert(point_in_convex_polygon(square, PointI(2, 2)));
  assert(point_in_convex_polygon(square, PointI(4, 2)));
  assert(!point_in_convex_polygon(square, PointI(4, 2), false));
  assert(!point_in_convex_polygon(square, PointI(5, 2)));

  vector<PointI> pentagon{{0, 0}, {3, 0}, {5, 2}, {2, 5}, {-1, 3}};
  assert(point_in_convex_polygon(pentagon, PointI(2, 3)));
  assert(point_in_convex_polygon(pentagon, PointI(0, 0)));
  assert(!point_in_convex_polygon(pentagon, PointI(0, 0), false));
  assert(!point_in_convex_polygon(pentagon, PointI(5, 4)));

  assert(line_convex_polygon_intersection(square, PointI(-1, 2), PointI(5, 2)) == make_pair(3, 1));
  assert(line_convex_polygon_intersection(square, PointI(0, 0), PointI(4, 0)) == make_pair(0, 0));
  assert(line_convex_polygon_intersection(square, PointI(5, 0), PointI(5, 4)) == make_pair(-1, -1));

  // From the point to the right of the square, the upper-right and lower-right vertices are
  // tangent.
  auto tangents = convex_polygon_tangents(square, PointI(6, 2));
  assert(tangents == make_pair(2, 1));
  return 0;
}
