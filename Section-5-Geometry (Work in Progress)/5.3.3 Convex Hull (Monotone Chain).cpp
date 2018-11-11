/*

Given a list of points in two dimensions, determine its convex hull using the
monotone chain algorithm. The convex hull is the smallest convex polygon (a
polygon such that every line crossing through it will only do so once) that
contains all of its points.

- convex_hull(lo, hi) returns the convex hull as a vector of polygon vertices in
  clockwise order, given a range [lo, hi) of points where lo and hi must be
  RandomAccessIterators. The input range will be sorted lexicographically (by x,
  then by y for equal x) after the function call. Note that to produce the hull
  points in counter-clockwise order, replace every GE() comparison with LE(). To
  have the first point on the hull repeated as the last in the resulting vector,
  the final res.resize(k - 1) may be changed to res.resize(k).

Time Complexity:
- O(n log n) per call to convex_hull(lo, hi), where n is the distance between lo
  and hi.

Space Complexity:
- O(n) auxiliary for storage of the convex hull.

*/

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define GE(a, b) ((a) >= (b) - EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double cross(const point &a, const point &b, const point &o = point(0, 0)) {
  return (a.x - o.x)*(b.y - o.y) - (a.y - o.y)*(b.x - o.x);
}

template<class It>
std::vector<point> convex_hull(It lo, It hi) {
  int k = 0;
  if (hi - lo <= 1) {
    return std::vector<point>(lo, hi);
  }
  std::vector<point> res(2*(int)(hi - lo));
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

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Irregular pentagon with only the vertex (1, 2) not on the hull.
  vector<point> v;
  v.push_back(point(1, 3));
  v.push_back(point(1, 2));
  v.push_back(point(2, 1));
  v.push_back(point(0, 0));
  v.push_back(point(-1, 3));
  std::random_shuffle(v.begin(), v.end());
  vector<point> h;
  h.push_back(point(-1, 3));
  h.push_back(point(1, 3));
  h.push_back(point(2, 1));
  h.push_back(point(0, 0));
  assert(convex_hull(v.begin(), v.end()) == h);
  return 0;
}
