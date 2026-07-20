/*

Maintain a static set of two-dimensional points while supporting nearest-neighbor queries. A k-d
tree recursively splits points by the coordinate with larger spread, searches the more promising
side first, and only explores the other side when its splitting plane can still improve the answer.

This implementation uses `std::pair` to represent points, requiring operator `<` and `long double`
casting to be defined on the numeric template type.

Use this for static nearest-neighbor queries on point sets in low dimensions. It is a geometric
search tree rather than an aggregate structure: for rectangle reporting use the range k-d tree or 2D
range tree, and for grid cell updates or rectangle sums/minima use a Fenwick tree or segment tree.

- `NearestKDTree<T>(lo, hi)` constructs a set of `std::pair` points from the half-open
  forward-iterator range $[`lo`, `hi`)$.
- `nearest(x, y, can_equal = true)` returns a point in the set that is closest to $(`x`, `y`)$ by
  Euclidean distance. This may be equal to $(`x`, `y`)$ only if `can_equal` is `true`; at least one
  eligible point must exist.

Time Complexity:
- O(n log n) per call to the constructor, where $n$ is the number of points.
- O(log n) on average per call to `nearest()`.

Space Complexity:
- O(n) for storage of the points.
- O(log n) auxiliary stack space for `nearest()`.

*/

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <climits>
#include <iterator>
#include <utility>
#include <vector>

template<typename T>
class NearestKDTree {
  std::vector<std::pair<T, T>> tree;
  std::vector<char> div_x;

  void build(int lo, int hi) {
    if (lo >= hi) {
      return;
    }
    int mid = lo + (hi - lo) / 2;
    T minx, maxx, miny, maxy;
    minx = maxx = tree[lo].first;
    miny = maxy = tree[lo].second;
    for (int i = lo + 1; i < hi; i++) {
      minx = std::min(minx, tree[i].first);
      miny = std::min(miny, tree[i].second);
      maxx = std::max(maxx, tree[i].first);
      maxy = std::max(maxy, tree[i].second);
    }
    long double x_span = static_cast<long double>(maxx) - minx;
    long double y_span = static_cast<long double>(maxy) - miny;
    div_x[mid] = !(x_span < y_span);
    std::nth_element(
        tree.begin() + lo, tree.begin() + mid, tree.begin() + hi,
        [&](const auto &a, const auto &b) {
          return div_x[mid] ? a.first < b.first : a.second < b.second;
        }
    );
    if (lo + 1 == hi) {
      return;
    }
    build(lo, mid);
    build(mid + 1, hi);
  }

  // Helper variables for nearest().
  long double min_dist;
  int id;

  void nearest(int lo, int hi, const T &x, const T &y, bool can_equal) {
    if (lo >= hi) {
      return;
    }
    int mid = lo + (hi - lo) / 2;
    long double dx = static_cast<long double>(x) - tree[mid].first;
    long double dy = static_cast<long double>(y) - tree[mid].second;
    long double d = dx * dx + dy * dy;
    if (d < min_dist && (can_equal || d != 0)) {
      min_dist = d;
      id = mid;
    }
    if (lo + 1 == hi) {
      return;
    }
    d = div_x[mid] ? dx : dy;
    int l1 = lo, r1 = mid, l2 = mid + 1, r2 = hi;
    if (d > 0) {
      std::swap(l1, l2);
      std::swap(r1, r2);
    }
    nearest(l1, r1, x, y, can_equal);
    if (d * static_cast<long double>(d) < min_dist) {
      nearest(l2, r2, x, y, can_equal);
    }
  }

 public:
  template<typename It>
  NearestKDTree(It lo, It hi) : tree(lo, hi) {
    int n = std::distance(lo, hi);
    assert(n > 0);
    div_x.resize(n);
    build(0, n);
  }

  std::pair<T, T> nearest(const T &x, const T &y, bool can_equal = true) {
    min_dist = LDBL_MAX;
    id = -1;
    nearest(0, static_cast<int>(tree.size()), x, y, can_equal);
    assert(id != -1);
    return tree[id];
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  vector<pair<int, int>> p{{0, 2}, {0, 3}, {-1, 0}};
  NearestKDTree<int> t(p.begin(), p.end());
  assert(t.nearest(0, 2, true) == (pair<int, int>{0, 2}));
  assert(t.nearest(0, 2, false) == (pair<int, int>{0, 3}));
  assert(t.nearest(0, 0) == (pair<int, int>{-1, 0}));
  assert(t.nearest(-10000, 0) == (pair<int, int>{-1, 0}));

  vector<pair<int, int>> extremes{{INT_MIN, 0}, {INT_MAX, 0}};
  NearestKDTree<int> extreme_tree(extremes.begin(), extremes.end());
  assert(extreme_tree.nearest(0, 0) == (pair<int, int>{INT_MAX, 0}));
  return 0;
}
