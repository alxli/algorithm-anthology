/*

Maintain a set of two-dimensional points while supporting queries for all points
that fall inside given rectangular regions. This implementation uses std::pair
to represent points, requiring operators < and == to be defined on the numeric
template type.

- kd_tree(lo, hi) constructs a set from two RandomAccessIterators to std::pair
  as a range [lo, hi) of points.
- query(x1, y1, x2, y2, f) calls the function f(i, p) on each point in the set
  that falls into the rectangular region consisting of rows from x1 to x2,
  inclusive, and columns from y1 to y2, inclusive. The first argument to f is
  the zero-based index of the point in the original range given to the
  constructor. The second argument is the point itself as an std::pair.

Time Complexity:
- O(n log n) per call to the constructor, where n is the number of points.
- O(log(n) + m) on average per call to query(), where m is the number of points
  that are reported by the query.

Space Complexity:
- O(n) for storage of the points.
- O(log n) auxiliary stack space for query().

*/

#include <algorithm>
#include <utility>
#include <vector>

template<class T>
class kd_tree {
  typedef std::pair<T, T> point;

  static inline bool comp1(const point &a, const point &b) {
    return a.first < b.first;
  }

  static inline bool comp2(const point &a, const point &b) {
    return a.second < b.second;
  }

  std::vector<point> tree, minp, maxp;
  std::vector<int> l_index, h_index;

  void build(int lo, int hi, bool div_x) {
    if (lo >= hi) {
      return;
    }
    int mid = lo + (hi - lo)/2;
    std::nth_element(tree.begin() + lo, tree.begin() + mid, tree.begin() + hi,
                     div_x ? comp1 : comp2);
    l_index[mid] = lo;
    h_index[mid] = hi;
    minp[mid].first = maxp[mid].first = tree[lo].first;
    minp[mid].second = maxp[mid].second = tree[lo].second;
    for (int i = lo + 1; i < hi; i++) {
      minp[mid].first = std::min(minp[mid].first, tree[i].first);
      minp[mid].second = std::min(minp[mid].second, tree[i].second);
      maxp[mid].first = std::max(maxp[mid].first, tree[i].first);
      maxp[mid].second = std::max(maxp[mid].second, tree[i].second);
    }
    build(lo, mid, !div_x);
    build(mid + 1, hi, !div_x);
  }

  // Helper variables for query().
  T x1, y1, x2, y2;

  template<class ReportFunction>
  void query(int lo, int hi, ReportFunction f) {
    if (lo >= hi) {
      return;
    }
    int mid = lo + (hi - lo)/2;
    T ax = minp[mid].first, ay = minp[mid].second;
    T bx = maxp[mid].first, by = maxp[mid].second;
    if (x2 < ax || bx < x1 || y2 < ay || by < y1) {
      return;
    }
    if (!(ax < x1 || x2 < bx || ay < y1 || y2 < by)) {
      for (int i = l_index[mid]; i < h_index[mid]; i++) {
        f(tree[i]);
      }
      return;
    }
    query(lo, mid, f);
    query(mid + 1, hi, f);
    if (tree[mid].first < x1 || x2 < tree[mid].first ||
        tree[mid].second < y1 || y2 < tree[mid].second) {
      return;
    }
    f(tree[mid]);
  }

 public:
  template<class It>
  kd_tree(It lo, It hi) : tree(lo, hi) {
    int n = std::distance(lo, hi);
    l_index.resize(n);
    h_index.resize(n);
    minp.resize(n);
    maxp.resize(n);
    build(0, n, true);
  }

  template<class ReportFunction>
  void query(const T &x1, const T &y1, const T &x2, const T &y2,
             ReportFunction f) {
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    query(0, tree.size(), f);
  }
};

/*** Example Usage and Output:

(2, -1) (1, 4) (2, 2) (-1, -1)
(1, 4) (2, 2) (3, 1)

***/

#include <iostream>
using namespace std;

void print(const pair<int, int> &p) {
  cout << "(" << p.first << ", " << p.second << ") ";
}

int main() {
  const int n = 10;
  int points[n][2] = {{1, 4}, {5, 4}, {2, 2}, {3, 1}, {6, -5}, {5, -1},
                      {3, -3}, {-1, -2}, {-1, -1}, {2, -1}};
  vector<pair<int, int> > v;
  for (int i = 0; i < n; i++) {
    v.push_back(make_pair(points[i][0], points[i][1]));
  }
  kd_tree<int> t(v.begin(), v.end());
  t.query(-1, -1, 2, 5, print);
  cout << endl;
  t.query(1, 1, 4, 8, print);
  cout << endl;
  return 0;
}
