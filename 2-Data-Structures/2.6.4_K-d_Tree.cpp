/*

Maintain a static collection of two-dimensional points while supporting rectangle reporting and
nearest-neighbor queries. A k-d tree recursively splits points by alternating coordinates and stores
a bounding box for each subtree. Rectangle queries accept or prune entire bounding boxes, while
nearest-neighbor queries visit the boxes with the smallest possible distance first and prune those
that cannot improve the answer.

This implementation uses `std::pair` to represent points. The numeric template type must support
`operator<` and conversion to `int64_t` for integral types or `long double` otherwise.

Use this when O(n) storage and nearest-neighbor queries are more important than the 2D range tree's
faster O(log^2 n + m) rectangle reporting. This tree reports a rectangle in O(sqrt(n) + m) time but
supports both query types with the same index.

- `KDTree<T>(lo, hi)` constructs a collection of `std::pair` points from the half-open
  forward-iterator range $[`lo`, `hi`)$.
- `query(x1, y1, x2, y2)` returns all points in the closed rectangle
  $[`x1`, `x2`] \times [`y1`, `y2`]$ as `std::pair` values.
- `nearest(x, y, can_equal = true)` returns a point in the collection that is closest to (`x`, `y`)
  by Euclidean distance. This may be equal to (`x`, `y`) only if `can_equal` is `true`; at least one
  eligible point must exist.

Overflow warning: For integral coordinate types, every coordinate difference, squared difference,
and sum of squared differences must fit in `int64_t`.

Time Complexity:
- O(n log n) per call to the constructor, where $n$ is the number of points.
- O(sqrt(n) + m) per call to `query()`, where $m$ is the number of points that are reported.
- O(log n) on average and O(n) in the worst case per call to `nearest()`.

Space Complexity:
- O(n) for storage of the points and bounding boxes.
- O(log n) auxiliary stack space for `query()` and `nearest()`.
- O(m) for the vector returned by `query()`, where $m$ is the number of reported points.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template<typename T>
class KDTree {
  using Dist = std::conditional_t<std::is_integral_v<T>, int64_t, long double>;
  using Point = std::pair<T, T>;

  std::vector<Point> tree, minp, maxp;

  static int root_index(int lo, int hi) { return lo + (hi - lo) / 2; }

  void build(int lo, int hi, bool div_x) {
    if (lo >= hi) {
      return;
    }
    int mid = root_index(lo, hi);
    std::nth_element(
        tree.begin() + lo, tree.begin() + mid, tree.begin() + hi,
        [div_x](const Point &a, const Point &b) {
          return div_x ? a.first < b.first : a.second < b.second;
        }
    );
    minp[mid] = maxp[mid] = tree[mid];
    build(lo, mid, !div_x);
    build(mid + 1, hi, !div_x);
    for (int i = lo; i < hi; i++) {
      minp[mid].first = std::min(minp[mid].first, tree[i].first);
      minp[mid].second = std::min(minp[mid].second, tree[i].second);
      maxp[mid].first = std::max(maxp[mid].first, tree[i].first);
      maxp[mid].second = std::max(maxp[mid].second, tree[i].second);
    }
  }

  void query(
      int lo, int hi, const T &x1, const T &y1, const T &x2, const T &y2, std::vector<Point> &res
  ) const {
    if (lo >= hi) {
      return;
    }
    int mid = root_index(lo, hi);
    T ax = minp[mid].first, ay = minp[mid].second;
    T bx = maxp[mid].first, by = maxp[mid].second;
    if (x2 < ax || bx < x1 || y2 < ay || by < y1) {
      return;
    }
    if (!(ax < x1 || x2 < bx || ay < y1 || y2 < by)) {
      res.insert(res.end(), tree.begin() + lo, tree.begin() + hi);
      return;
    }
    query(lo, mid, x1, y1, x2, y2, res);
    query(mid + 1, hi, x1, y1, x2, y2, res);
    if (!(tree[mid].first < x1 || x2 < tree[mid].first || tree[mid].second < y1 ||
          y2 < tree[mid].second)) {
      res.push_back(tree[mid]);
    }
  }

  Dist box_dist(int root, const T &x, const T &y) const {
    Dist dx = 0, dy = 0;
    if (x < minp[root].first) {
      dx = Dist(minp[root].first) - Dist(x);
    } else if (maxp[root].first < x) {
      dx = Dist(x) - Dist(maxp[root].first);
    }
    if (y < minp[root].second) {
      dy = Dist(minp[root].second) - Dist(y);
    } else if (maxp[root].second < y) {
      dy = Dist(y) - Dist(maxp[root].second);
    }
    return dx * dx + dy * dy;  // Overflow warning.
  }

  void nearest(
      int lo, int hi, const T &x, const T &y, bool can_equal, Dist &best_dist, int &best
  ) const {
    if (lo >= hi) {
      return;
    }
    int mid = root_index(lo, hi);
    if (best != -1 && best_dist <= box_dist(mid, x, y)) {
      return;
    }
    Dist dx = Dist(x) - Dist(tree[mid].first);
    Dist dy = Dist(y) - Dist(tree[mid].second);
    Dist dist = dx * dx + dy * dy;  // Overflow warning.
    if ((best == -1 || dist < best_dist) && (can_equal || dist != 0)) {
      best_dist = dist;
      best = mid;
    }

    int llo = lo, lhi = mid, rlo = mid + 1, rhi = hi;
    if (llo < lhi && rlo < rhi &&
        box_dist(root_index(rlo, rhi), x, y) < box_dist(root_index(llo, lhi), x, y)) {
      std::swap(llo, rlo);
      std::swap(lhi, rhi);
    }
    nearest(llo, lhi, x, y, can_equal, best_dist, best);
    nearest(rlo, rhi, x, y, can_equal, best_dist, best);
  }

 public:
  template<typename It>
  KDTree(It lo, It hi) : tree(lo, hi), minp(tree.size()), maxp(tree.size()) {
    build(0, static_cast<int>(tree.size()), true);
  }

  std::vector<Point> query(const T &x1, const T &y1, const T &x2, const T &y2) const {
    assert(!(x2 < x1) && !(y2 < y1));
    std::vector<Point> res;
    query(0, static_cast<int>(tree.size()), x1, y1, x2, y2, res);
    return res;
  }

  Point nearest(const T &x, const T &y, bool can_equal = true) const {
    Dist best_dist = std::numeric_limits<Dist>::max();
    int best = -1;
    nearest(0, static_cast<int>(tree.size()), x, y, can_equal, best_dist, best);
    assert(best != -1);
    return tree[best];
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  vector<pair<int, int>> points{{1, 4},  {5, 4},  {2, 2},   {3, 1},   {6, -5},
                                {5, -1}, {3, -3}, {-1, -2}, {-1, -1}, {2, -1}};
  KDTree<int> t(points.begin(), points.end());
  auto got = t.query(-1, -1, 2, 5);
  sort(got.begin(), got.end());
  assert((got == vector<pair<int, int>>{{-1, -1}, {1, 4}, {2, -1}, {2, 2}}));
  got = t.query(1, 1, 4, 8);
  sort(got.begin(), got.end());
  assert((got == vector<pair<int, int>>{{1, 4}, {2, 2}, {3, 1}}));

  assert(t.nearest(0, 2) == (pair<int, int>{2, 2}));
  assert(t.nearest(2, 2, false) == (pair<int, int>{3, 1}));

  vector<pair<int, int>> large{{-1000000000, 1}, {1000000000, 0}};
  KDTree<int> large_tree(large.begin(), large.end());
  assert(large_tree.nearest(0, 0) == (pair<int, int>{1000000000, 0}));

  KDTree<int> empty(points.end(), points.end());
  assert(empty.query(0, 0, 1, 1).empty());
  return 0;
}
