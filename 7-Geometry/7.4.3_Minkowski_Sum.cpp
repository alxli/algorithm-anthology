/*

Given two convex polygons, compute their Minkowski sum: the set of all points $a + b$ where $a$
comes from the first polygon and $b$ comes from the second. After rotating both polygons to start at
their lowest-leftmost vertex, the boundary edges of the sum are obtained by merging the two cyclic
edge-angle lists. Minkowski sums are useful for collision/translation problems, convex polygon
distance checks, and diameter computations via $P + (-P)$.

- `minkowski_sum(a, b)` returns the convex polygon representing the sum. Inputs must be convex
  polygons in boundary order, clockwise or counter-clockwise, with no repeated final vertex.
  Collinear consecutive output vertices are removed.

Time Complexity:
- O(n + m), where $n$ and $m$ are the polygon sizes.

Space Complexity:
- O(n + m) auxiliary.

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

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  friend bool EQ(const Point &a, const Point &b) { return EQ(a.x, b.x) && EQ(a.y, b.y); }
  Point operator+(const Point &p) const { return {x + p.x, y + p.y}; }
  Point operator-(const Point &p) const { return {x - p.x, y - p.y}; }
  double cross(const Point &p) const { return x * p.y - y * p.x; }
};

double polygon_area_2x(const std::vector<Point> &p) {
  double area = 0;
  for (int i = 0, j = static_cast<int>(p.size()) - 1; i < static_cast<int>(p.size()); j = i++) {
    area += p[j].cross(p[i]);
  }
  return area;
}

std::vector<Point> normalize(std::vector<Point> p) {
  while (p.size() > 1 && EQ(p.front(), p.back())) {
    p.pop_back();
  }
  if (p.size() <= 1) {
    return p;
  }
  if (polygon_area_2x(p) < 0) {
    std::reverse(p.begin(), p.end());
  }
  int start = 0;
  for (int i = 1; i < static_cast<int>(p.size()); i++) {
    if (p[i].y < p[start].y || (EQ(p[i].y, p[start].y) && p[i].x < p[start].x)) {
      start = i;
    }
  }
  std::rotate(p.begin(), p.begin() + start, p.end());
  return p;
}

std::vector<Point> remove_collinear(std::vector<Point> p) {
  std::vector<Point> res;
  for (const Point &q : p) {
    while (res.size() >= 2 && EQ((res.back() - res[res.size() - 2]).cross(q - res.back()), 0)) {
      res.pop_back();
    }
    if (res.empty() || !EQ(q, res.back())) {
      res.push_back(q);
    }
  }
  while (res.size() >= 3 && EQ((res.back() - res[res.size() - 2]).cross(res[0] - res.back()), 0)) {
    res.pop_back();
  }
  while (res.size() >= 3 && EQ((res[0] - res.back()).cross(res[1] - res[0]), 0)) {
    res.erase(res.begin());
  }
  return res;
}

std::vector<Point> minkowski_sum(std::vector<Point> a, std::vector<Point> b) {
  a = normalize(std::move(a));
  b = normalize(std::move(b));
  if (a.empty() || b.empty()) {
    return {};
  }
  if (a.size() == 1) {
    for (Point &p : b) {
      p = p + a[0];
    }
    return b;
  }
  if (b.size() == 1) {
    for (Point &p : a) {
      p = p + b[0];
    }
    return a;
  }
  int n = static_cast<int>(a.size()), m = static_cast<int>(b.size());
  a.push_back(a[0]);
  a.push_back(a[1]);
  b.push_back(b[0]);
  b.push_back(b[1]);
  std::vector<Point> res;
  int i = 0, j = 0;
  while (i < n || j < m) {
    res.push_back(a[i] + b[j]);
    if (i == n) {
      j++;
      continue;
    }
    if (j == m) {
      i++;
      continue;
    }
    double cr = (a[i + 1] - a[i]).cross(b[j + 1] - b[j]);
    if (cr > -EPS) {
      i++;
    }
    if (cr < EPS) {
      j++;
    }
  }
  return remove_collinear(res);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<Point> empty;
  assert(minkowski_sum(empty, {Point(1, 2)}).empty());

  vector<Point> square{{0, 0}, {1, 0}, {1, 1}, {0, 1}};
  vector<Point> tri{{0, 0}, {2, 0}, {0, 2}};
  vector<Point> sum = minkowski_sum(square, tri);
  vector<Point> expected{{0, 0}, {3, 0}, {3, 1}, {1, 3}, {0, 3}};
  assert(sum == expected);

  vector<Point> shifted = minkowski_sum(square, {Point(2, 3)});
  vector<Point> expected_shifted{{2, 3}, {3, 3}, {3, 4}, {2, 4}};
  assert(shifted == expected_shifted);

  vector<Point> point_sum = minkowski_sum({Point(1, 2)}, {Point(3, 4)});
  assert(point_sum.size() == 1 && point_sum[0] == Point(4, 6));
  return 0;
}
