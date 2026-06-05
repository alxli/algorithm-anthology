/*

Given a list of points in two dimensions, computes the convex hull using the monotone chain
algorithm and the diametral pair using rotating calipers.

The functions are templated on the iterator type; the value type of the iterator is used as the
point type. The local `Point` struct (double coordinates) is the default; replace it with `PointD`/
`PointI` from 7.1.1 or any struct with numeric `.x` and `.y` fields, `<`, `==`, and `!=` operators.
`convex_hull` uses only cross product comparisons and is exact for integer-coordinate points.
`diametral_pair` uses squared-distance comparisons and is also exact for integer points.

- `convex_hull(lo, hi)` returns the convex hull in clockwise order. The input range is sorted
  lexicographically after the call. To produce CCW order, replace every `GE` with `LE`.
- `diametral_pair(lo, hi)` returns the maximum-distance pair of points.

Time Complexity:
- O(n log n) per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) auxiliary for storage of the convex hull.

*/

#include <algorithm>
#include <cmath>
#include <random>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define GE(a, b) ((a) >= (b) - EPS)

template<class Pt>
auto cross(const Pt &a, const Pt &b, const Pt &o) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

// Convex hull: exact for integer-coordinate points.
template<class It>
auto convex_hull(It lo, It hi) {
  using Pt = typename std::iterator_traits<It>::value_type;
  int k = 0;
  if (hi - lo <= 1) {
    return std::vector<Pt>(lo, hi);
  }
  std::vector<Pt> res(2 * (int)(hi - lo));
  std::sort(lo, hi);
  for (It it = lo; it != hi; ++it) {
    while (k >= 2 && GE(cross(res[k - 1], *it, res[k - 2]), 0)) {
      k--;
    }
    res[k++] = *it;
  }
  int t = k + 1;
  for (It it = hi - 2; it != lo - 1; --it) {
    while (k >= t && GE(cross(res[k - 1], *it, res[k - 2]), 0)) {
      k--;
    }
    res[k++] = *it;
  }
  res.resize(k - 1);
  return res;
}

// Diametral pair: squared-distance comparisons are exact for integer-coordinate points.
template<class It>
auto diametral_pair(It lo, It hi) {
  using Pt = typename std::iterator_traits<It>::value_type;
  auto h = convex_hull(lo, hi);
  int m = h.size();
  if (m == 1) return std::pair<Pt, Pt>{h[0], h[0]};
  if (m == 2) return std::pair<Pt, Pt>{h[0], h[1]};
  int k = 1;
  while (std::abs(cross(h[0], h[(k + 1) % m], h[m - 1])) > std::abs(cross(h[0], h[k], h[m - 1])))
    k++;
  auto sqdist = [](const Pt &a, const Pt &b) {
    auto dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
  };
  auto maxsq = sqdist(h[0], h[0]);
  std::pair<Pt, Pt> res{h[0], h[0]};
  for (int i = 0, j = k; i <= k && j < m; i++) {
    auto d = sqdist(h[i], h[j]);
    if (d > maxsq) {
      maxsq = d;
      res = {h[i], h[j]};
    }
    while (j < m && std::abs(cross(h[(i + 1) % m], h[(j + 1) % m], h[i])) >
                        std::abs(cross(h[(i + 1) % m], h[j], h[i]))) {
      d = sqdist(h[i], h[(j + 1) % m]);
      if (d > maxsq) {
        maxsq = d;
        res = {h[i], h[(j + 1) % m]};
      }
      j++;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

struct PointI {
  int x, y;
  bool operator==(const PointI &o) const { return x == o.x && y == o.y; }
  bool operator!=(const PointI &o) const { return !(*this == o); }
  bool operator<(const PointI &o) const { return x != o.x ? x < o.x : y < o.y; }
  bool operator>(const PointI &o) const { return o < *this; }
};

int main() {
  {
    vector<Point> v{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
    std::mt19937 rng(12345);
    std::shuffle(v.begin(), v.end(), rng);
    vector<Point> h{{-1, 3}, {1, 3}, {2, 1}, {0, 0}};
    assert(convex_hull(v.begin(), v.end()) == h);
  }
  {
    vector<Point> v{{0, 0}, {3, 0}, {0, 3}, {1, 1}, {4, 4}};
    auto [p1, p2] = diametral_pair(v.begin(), v.end());
    assert((p1 == Point{0, 0}));
    assert((p2 == Point{4, 4}));
  }

  // Integer-coordinate points: hull and diametral pair are exact.
  {
    vector<PointI> v{{0, 0}, {4, 0}, {4, 4}, {0, 4}, {2, 2}};
    auto h = convex_hull(v.begin(), v.end());
    assert(h.size() == 4);  // interior point (2,2) excluded
    auto [p1, p2] = diametral_pair(v.begin(), v.end());
    // diametral pair is exact
    assert(
        (p1 == PointI{0, 0} && p2 == PointI{4, 4}) || (p1 == PointI{4, 4} && p2 == PointI{0, 0}) ||
        (p1 == PointI{4, 0} && p2 == PointI{0, 4}) || (p1 == PointI{0, 4} && p2 == PointI{4, 0})
    );
  }
  return 0;
}
