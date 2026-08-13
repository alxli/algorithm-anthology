/*

Maintain a set of two-dimensional points while supporting queries for all points that fall inside
given rectangular regions. This implementation uses `std::pair` to represent points, requiring
operators `<`, `<=`, and `==` to be defined on the numeric template type. A balanced tree over the
points sorted by $x$ stores at each node its subrange of points sorted by $y$, merged from its
children's lists as in a merge sort tree. A query decomposes the $x$-range into O(log n) nodes and
binary searches each node's list for the matching $y$-range.

Use this for static point-reporting queries when guaranteed worst-case bounds are more important
than memory. Compared with a range k-d tree, it uses more space but gives O(log^2 n + m) query time
regardless of point distribution; the k-d tree is lighter and often faster on typical inputs, but
its pruning is more distribution-dependent.

- `RangeTree<T>(lo, hi)` constructs a set of `std::pair` points from the half-open forward-iterator
  range $[`lo`, `hi`)$.
- `query(x1, y1, x2, y2)` returns (`i`, `x`, `y`) tuples for all points in the closed rectangle
  $[`x1`, `x2`] \times [`y1`, `y2`]$, where `i` is the point's 0-based index in the original range.

Time Complexity:
- O(n log n) per call to the constructor, where $n$ is the number of points.
- O(log^2 n + m) per call to `query()`, where $m$ is the number of points that are reported by the
  query.

Space Complexity:
- O(n log n) for storage of the points.
- O(log n) auxiliary stack space for `query()`.
- O(m) for the vector returned by `query()`, where $m$ is the number of reported points.

*/

#include <algorithm>
#include <cassert>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

template<typename T>
class RangeTree {
  struct IndexedPoint {
    T x, y;
    int original_index;
  };

  std::vector<IndexedPoint> points;
  std::vector<std::vector<std::pair<int, T>>> columns;

  void build(int n, int lo, int hi) {
    if (points[lo].x == points[hi].x) {
      for (int i = lo; i <= hi; i++) {
        columns[n].emplace_back(i, points[i].y);
      }
      return;
    }
    int l = n * 2 + 1, r = n * 2 + 2, mid = lo + (hi - lo) / 2;
    build(l, lo, mid);
    build(r, mid + 1, hi);
    columns[n].resize(columns[l].size() + columns[r].size());
    std::merge(
        columns[l].begin(), columns[l].end(), columns[r].begin(), columns[r].end(),
        columns[n].begin(), [](const auto &a, const auto &b) { return a.second < b.second; }
    );
  }

  void query(
      int n, int lo, int hi, const T &x1, const T &y1, const T &x2, const T &y2,
      std::vector<std::tuple<int, T, T>> &res
  ) {
    if (points[hi].x < x1 || x2 < points[lo].x) {
      return;
    }
    if (!(points[lo].x < x1 || x2 < points[hi].x)) {
      if (!columns[n].empty() && !(y2 < y1)) {
        auto it = std::lower_bound(
            columns[n].begin(), columns[n].end(), y1,
            [](const auto &a, const T &value) { return a.second < value; }
        );
        for (; it != columns[n].end() && it->second <= y2; ++it) {
          const IndexedPoint &p = points[it->first];
          res.emplace_back(p.original_index, p.x, p.y);
        }
      }
    } else if (lo != hi) {
      int mid = lo + (hi - lo) / 2;
      query(n * 2 + 1, lo, mid, x1, y1, x2, y2, res);
      query(n * 2 + 2, mid + 1, hi, x1, y1, x2, y2, res);
    }
  }

 public:
  template<typename It>
  RangeTree(It lo, It hi) {
    int n = std::distance(lo, hi);
    assert(n > 0);
    points.reserve(n);
    int index = 0;
    for (It it = lo; it != hi; ++it) {
      points.push_back({it->first, it->second, index++});
    }
    columns.resize(4 * n + 1);
    std::sort(points.begin(), points.end(), [](const IndexedPoint &a, const IndexedPoint &b) {
      return a.x != b.x ? a.x < b.x : a.y < b.y;
    });
    build(0, 0, n - 1);
  }

  std::vector<std::tuple<int, T, T>> query(const T &x1, const T &y1, const T &x2, const T &y2) {
    assert(!(x2 < x1) && !(y2 < y1));
    std::vector<std::tuple<int, T, T>> res;
    query(0, 0, static_cast<int>(points.size()) - 1, x1, y1, x2, y2, res);
    return res;
  }
};

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
using namespace std;

int main() {
  vector<pair<int, int>> v{{1, 4},  {5, 4},  {2, 2},   {3, 1},   {6, -5},
                           {5, -1}, {3, -3}, {-1, -2}, {-1, -1}, {2, -1}};
  RangeTree<int> t(v.begin(), v.end());
  auto got = t.query(-1, -1, 2, 5);
  sort(got.begin(), got.end());
  assert((got == vector<tuple<int, int, int>>{{0, 1, 4}, {2, 2, 2}, {8, -1, -1}, {9, 2, -1}}));
  got = t.query(1, 1, 4, 8);
  sort(got.begin(), got.end());
  assert((got == vector<tuple<int, int, int>>{{0, 1, 4}, {2, 2, 2}, {3, 3, 1}}));
  return 0;
}
