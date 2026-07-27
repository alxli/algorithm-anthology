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
- `query(x1, y1, x2, y2, f)` calls the function `f(i, p)` on each point whose $x$-coordinate is in
  $[`x1`, `x2`]$ and whose $y$-coordinate is in $[`y1`, `y2`]$. The first argument to `f` is the
  0-based index of the point in the original range given to the constructor. The second argument is
  the point itself as an `std::pair`.

Time Complexity:
- O(n log n) per call to the constructor, where $n$ is the number of points.
- O(log^2 n + m) per call to `query()`, where $m$ is the number of points that are reported by the
  query.

Space Complexity:
- O(n log n) for storage of the points.
- O(log n) auxiliary stack space for `query()`.

*/

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

template<typename T>
class RangeTree {
  struct IndexedPoint {
    std::pair<T, T> value;
    int original_index;
  };

  std::vector<IndexedPoint> points;
  std::vector<std::vector<std::pair<int, T>>> columns;

  void build(int n, int lo, int hi) {
    if (points[lo].value.first == points[hi].value.first) {
      for (int i = lo; i <= hi; i++) {
        columns[n].emplace_back(i, points[i].value.second);
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

  // Helper variables for query().
  T x1, y1, x2, y2;

  template<typename Fn>
  void query(int n, int lo, int hi, Fn &f) {
    if (points[hi].value.first < x1 || x2 < points[lo].value.first) {
      return;
    }
    if (!(points[lo].value.first < x1 || x2 < points[hi].value.first)) {
      if (!columns[n].empty() && !(y2 < y1)) {
        auto it = std::lower_bound(
            columns[n].begin(), columns[n].end(), y1,
            [](const auto &a, const T &value) { return a.second < value; }
        );
        for (; it != columns[n].end() && it->second <= y2; ++it) {
          const IndexedPoint &p = points[it->first];
          f(p.original_index, p.value);
        }
      }
    } else if (lo != hi) {
      int mid = lo + (hi - lo) / 2;
      query(n * 2 + 1, lo, mid, f);
      query(n * 2 + 2, mid + 1, hi, f);
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
      points.push_back({*it, index++});
    }
    columns.resize(4 * n + 1);
    std::sort(points.begin(), points.end(), [](const IndexedPoint &a, const IndexedPoint &b) {
      return a.value < b.value;
    });
    build(0, 0, n - 1);
  }

  template<typename Fn>
  void query(const T &x1, const T &y1, const T &x2, const T &y2, Fn f) {
    assert(!(x2 < x1) && !(y2 < y1));
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    query(0, 0, static_cast<int>(points.size()) - 1, f);
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
  vector<pair<int, int>> got;
  vector<int> indices;
  auto collect = [&](int i, const pair<int, int> &p) {
    indices.push_back(i);
    got.push_back(p);
  };
  t.query(-1, -1, 2, 5, collect);
  sort(got.begin(), got.end());
  assert((got == vector<pair<int, int>>{{-1, -1}, {1, 4}, {2, -1}, {2, 2}}));
  sort(indices.begin(), indices.end());
  assert((indices == vector<int>{0, 2, 8, 9}));
  got.clear();
  indices.clear();
  t.query(1, 1, 4, 8, collect);
  sort(got.begin(), got.end());
  assert((got == vector<pair<int, int>>{{1, 4}, {2, 2}, {3, 1}}));
  return 0;
}
