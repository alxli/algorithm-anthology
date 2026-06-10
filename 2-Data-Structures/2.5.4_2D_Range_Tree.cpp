/*

Maintain a set of two-dimensional points while supporting queries for all points that fall inside
given rectangular regions. This implementation uses `std::pair` to represent points, requiring
operators `<` and `==` to be defined on the numeric template type.

Use this for static point-reporting queries when guaranteed worst-case bounds are more important
than memory. Compared with a range k-d tree, it uses more space but gives O(log^2(n) + m) query time
regardless of point distribution; the k-d tree is lighter and often faster on typical inputs, but
its pruning is more distribution-dependent.

- `RangeTree(lo, hi)` constructs a set from two random-access iterators to `std::pair` as a range
  `[lo, hi)` of points.
- `query(x1, y1, x2, y2, f)` calls the function `f(i, p)` on each point in the set that falls into
  the rectangular region consisting of rows from `x1` to `x2`, inclusive, and columns from `y1` to
  `y2`, inclusive. The first argument to `f` is the 0-based index of the point in the original
  range given to the constructor. The second argument is the point itself as an `std::pair`.

Time Complexity:
- O(n log n) per call to the constructor, where $n$ is the number of points.
- O(log^2(n) + m) per call to `query()`, where $m$ is the number of points that are reported by the
  query.

Space Complexity:
- O(n log n) for storage of the points.
- O(log^2(n)) auxiliary stack space for `query()`.

*/

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

template<class T>
class RangeTree {
  using point = std::pair<T, T>;
  using colindex = std::pair<int, T>;

  std::vector<point> points;
  std::vector<std::vector<colindex>> columns;

  static inline bool comp1(const colindex &a, const colindex &b) { return a.second < b.second; }
  static inline bool comp2(const colindex &a, const T &v) { return a.second < v; }

  void build(int n, int lo, int hi) {
    if (points[lo].first == points[hi].first) {
      for (int i = lo; i <= hi; i++) {
        columns[n].emplace_back(i, points[i].second);
      }
      return;
    }
    int l = n * 2 + 1, r = n * 2 + 2, mid = lo + (hi - lo) / 2;
    build(l, lo, mid);
    build(r, mid + 1, hi);
    columns[n].resize(columns[l].size() + columns[r].size());
    std::merge(
        columns[l].begin(), columns[l].end(), columns[r].begin(), columns[r].end(),
        columns[n].begin(), comp1
    );
  }

  // Helper variables for query().
  T x1, y1, x2, y2;

  template<class Fn>
  void query(int n, int lo, int hi, Fn f) {
    if (points[hi].first < x1 || x2 < points[lo].first) {
      return;
    }
    if (!(points[lo].first < x1 || x2 < points[hi].first)) {
      if (!columns[n].empty() && !(y2 < y1)) {
        auto it = std::lower_bound(columns[n].begin(), columns[n].end(), y1, comp2);
        for (; it != columns[n].end() && it->second <= y2; ++it) {
          f(it->first, points[it->first]);
        }
      }
    } else if (lo != hi) {
      int mid = lo + (hi - lo) / 2;
      query(n * 2 + 1, lo, mid, f);
      query(n * 2 + 2, mid + 1, hi, f);
    }
  }

 public:
  template<class It>
  RangeTree(It lo, It hi) : points(lo, hi) {
    int n = std::distance(lo, hi);
    columns.resize(4 * n + 1);
    std::sort(points.begin(), points.end());
    build(0, 0, n - 1);
  }

  template<class Fn>
  void query(const T &x1, const T &y1, const T &x2, const T &y2, Fn f) {
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    query(0, 0, points.size() - 1, f);
  }
};

/*** Example Usage and Output:

(-1, -1) (2, -1) (2, 2) (1, 4)
(1, 4) (2, 2) (3, 1)

***/

#include <iostream>
using namespace std;

void print(int i, const pair<int, int> &p) {
  cout << "(" << p.first << ", " << p.second << ") ";
}

int main() {
  vector<pair<int, int>> v{{1, 4},  {5, 4},  {2, 2},   {3, 1},   {6, -5},
                           {5, -1}, {3, -3}, {-1, -2}, {-1, -1}, {2, -1}};
  RangeTree<int> t(v.begin(), v.end());
  t.query(-1, -1, 2, 5, print);
  cout << endl;
  t.query(1, 1, 4, 8, print);
  cout << endl;
  return 0;
}
