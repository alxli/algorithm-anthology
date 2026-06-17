/*

Given a list of points in two dimensions, computes the convex hull using the monotone chain
algorithm and the diametral pair using rotating calipers. Monotone chain sorts the points
lexicographically and builds the lower and upper hulls in one pass each, popping any point that
would create a clockwise turn. Rotating calipers then walks two antipodal pointers around the hull,
advancing whichever increases the separation, visiting every candidate diametral pair in linear
time. The functions are templated on the iterator type; the value type of the iterator is used as
the point type. Both functions accept either floating-point or integral coordinates, but since they
use only cross product comparisons, they happen to be exact for integral points.

- `convex_hull(lo, hi)` returns the convex hull in clockwise order for a range [`lo`, `hi`) of
  points. The input range is sorted lexicographically after the call. To instead return the hull
  points counter-clockwise order, replace every `>= 0` with `<= 0`.
- `diametral_pair(lo, hi)` returns the maximum-distance pair of points.

Overflow warning: `cross()` and the squared distance in `diametral_pair()` grow like the squared
coordinate magnitude. With 32-bit `int` coordinates they overflow once coordinates exceed a few tens
of thousands; use a 64-bit (`int64_t`) coordinate type for larger integer inputs.

Time Complexity:
- O(n log n) per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) auxiliary for storage of the convex hull.

*/

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

template<class Pt>
auto cross(const Pt &a, const Pt &b, const Pt &o) {
  // Overflow risk for integer Pt: these products are ~O(max_coord^2); use int64_t if necessary.
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
  std::vector<Pt> res(2 * static_cast<int>(hi - lo));
  std::sort(lo, hi);
  for (It it = lo; it != hi; ++it) {
    while (k >= 2 && cross(res[k - 1], *it, res[k - 2]) >= 0) {
      k--;
    }
    res[k++] = *it;
  }
  int t = k + 1;
  for (It it = hi - 1; it != lo;) {
    --it;
    while (k >= t && cross(res[k - 1], *it, res[k - 2]) >= 0) {
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
  int m = static_cast<int>(h.size());
  if (m == 0) {
    return std::pair<Pt, Pt>{};
  }
  if (m == 1) {
    return std::pair<Pt, Pt>{h[0], h[0]};
  }
  if (m == 2) {
    return std::pair<Pt, Pt>{h[0], h[1]};
  }
  int k = 1;
  while (std::abs(cross(h[0], h[(k + 1) % m], h[m - 1])) > std::abs(cross(h[0], h[k], h[m - 1]))) {
    k++;
  }
  auto sqdist = [](const Pt &a, const Pt &b) {
    // Overflow risk for integer Pt: ~O(max_coord^2); use int64_t if necessary.
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

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
  bool operator==(const PointI &o) const { return x == o.x && y == o.y; }
  bool operator!=(const PointI &o) const { return !(*this == o); }
  bool operator<(const PointI &o) const { return x != o.x ? x < o.x : y < o.y; }
  bool operator>(const PointI &o) const { return o < *this; }
};

int main() {
  {
    vector<PointI> v{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
    std::mt19937 rng(12345);
    std::shuffle(v.begin(), v.end(), rng);
    vector<PointI> h{{-1, 3}, {1, 3}, {2, 1}, {0, 0}};
    assert(convex_hull(v.begin(), v.end()) == h);
  }
  {
    vector<PointI> v{{0, 0}, {3, 0}, {0, 3}, {1, 1}, {4, 4}};
    auto [p1, p2] = diametral_pair(v.begin(), v.end());
    assert(p1 == PointI(0, 0));
    assert(p2 == PointI(4, 4));
  }
  {
    vector<PointI> v{{0, 0}, {4, 0}, {4, 4}, {0, 4}, {2, 2}};
    auto h = convex_hull(v.begin(), v.end());
    assert(h.size() == 4);  // interior point (2,2) excluded
    auto [p1, p2] = diametral_pair(v.begin(), v.end());
    // diametral pair is exact
    assert(
        (p1 == PointI(0, 0) && p2 == PointI(4, 4)) || (p1 == PointI(4, 4) && p2 == PointI(0, 0)) ||
        (p1 == PointI(4, 0) && p2 == PointI(0, 4)) || (p1 == PointI(0, 4) && p2 == PointI(4, 0))
    );
  }
  return 0;
}
