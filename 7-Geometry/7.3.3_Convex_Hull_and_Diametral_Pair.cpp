/*

Given a list of points in two dimensions, determine the convex hull using the monotone chain
algorithm, and the diameter of the points using the method of rotating calipers. The convex hull is
the smallest convex polygon (a polygon such that every line crossing through it will only do so
once) that contains all of its points.

- `convex_hull(lo, hi)` returns the convex hull as a vector of polygon vertices in clockwise order,
  given a range `[lo, hi)` of points where `lo` and `hi` must be random-access iterators. The input
  range will be sorted lexicographically (by $x$, then by $y$) after the function call. Note that to
  produce the hull points in counter-clockwise order, replace every `GE()` comparison with `LE()`.
  To have the first point on the hull repeated as the last in the resulting vector, the final
  `res.resize(k - 1)` may be changed to `res.resize(k)`.
- `diametral_pair(lo, hi)` returns a maximum diametral pair given a range `[lo, hi)` of points where
  `lo` and `hi` must be random-access iterators. The input range will be sorted lexicographically
  (by $x$, then by $y$) after the function call.

Time Complexity:
- O(n log n) per call to `convex_hull(lo, hi)` and `diametral_pair(lo, hi)`, where $n$ is the
  distance between `lo` and `hi`.

Space Complexity:
- O(n) auxiliary for storage of the convex hull in both operations.

*/

#include <algorithm>
#include <cmath>
#include <random>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define GE(a, b) ((a) >= (b) - EPS)

using point = std::pair<double, double>;
#define x first
#define y second

double sqnorm(const point &a) {
  return a.x * a.x + a.y * a.y;
}

double cross(const point &a, const point &b, const point &o = point(0, 0)) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

template<class It>
std::vector<point> convex_hull(It lo, It hi) {
  int k = 0;
  if (hi - lo <= 1) {
    return std::vector<point>(lo, hi);
  }
  std::vector<point> res(2 * static_cast<int>((hi - lo)));
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

template<class It>
std::pair<point, point> diametral_pair(It lo, It hi) {
  std::vector<point> h = convex_hull(lo, hi);
  int m = h.size();
  if (m == 1) {
    return {h[0], h[0]};
  }
  if (m == 2) {
    return {h[0], h[1]};
  }
  int k = 1;
  while (fabs(cross(h[0], h[(k + 1) % m], h[m - 1])) > fabs(cross(h[0], h[k], h[m - 1]))) {
    k++;
  }
  double maxdist = 0, d;
  std::pair<point, point> res;
  for (int i = 0, j = k; i <= k && j < m; i++) {
    d = sqnorm(point(h[i].x - h[j].x, h[i].y - h[j].y));
    if (d > maxdist) {
      maxdist = d;
      res = {h[i], h[j]};
    }
    while (j < m && fabs(cross(h[(i + 1) % m], h[(j + 1) % m], h[i])) >
                        fabs(cross(h[(i + 1) % m], h[j], h[i]))) {
      d = sqnorm(point(h[i].x - h[(j + 1) % m].x, h[i].y - h[(j + 1) % m].y));
      if (d > maxdist) {
        maxdist = d;
        res = {h[i], h[(j + 1) % m]};
      }
      j++;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  {  // Irregular pentagon with only the vertex (1, 2) not on the hull.
    vector<point> v;
    v.emplace_back(1, 3);
    v.emplace_back(1, 2);
    v.emplace_back(2, 1);
    v.emplace_back(0, 0);
    v.emplace_back(-1, 3);
    std::mt19937 rng(12345);
    std::shuffle(v.begin(), v.end(), rng);
    vector<point> h;
    h.emplace_back(-1, 3);
    h.emplace_back(1, 3);
    h.emplace_back(2, 1);
    h.emplace_back(0, 0);
    assert(convex_hull(v.begin(), v.end()) == h);
  }
  {
    vector<point> v;
    v.emplace_back(0, 0);
    v.emplace_back(3, 0);
    v.emplace_back(0, 3);
    v.emplace_back(1, 1);
    v.emplace_back(4, 4);
    auto [p1, p2] = diametral_pair(v.begin(), v.end());
    assert(p1 == point(0, 0));
    assert(p2 == point(4, 4));
  }
  return 0;
}
