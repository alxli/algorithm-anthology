/*

Maintain a set of two-dimensional points while supporting queries for all points
that fall inside given rectangular regions. This implementation uses std::pair
to represent points, requiring operators < and == to be defined on the numeric
template type.

- range_tree_2d(lo, hi) constructs a set from two RandomAccessIterators to
  std::pair as a range [lo, hi) of points.
- query(r1, c1, r2, c2, f) calls the function f(i, p) on each point in the set
  that falls into the rectangular region consisting of rows from r1 to r2,
  inclusive, and columns from c1 to c2, inclusive. The first argument to f is
  the zero-based index of the point in the original range given to the constructor. The second argument is the point itself as an std::pair.

Time Complexity:
- O(n log n) per call to the constructor, where n is the number of points.
- O(log^2(n) + m) per call to query(), where m is the number of points that are
  reported by the query.

Space Complexity:
- O(n log n) for storage of the points.
- O(log^2(n)) auxiliary stack space per call to query().

*/

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

template<class T> class range_tree_2d {
  typedef std::pair<T, T> point;
  typedef std::pair<int, T> colindex;

  std::vector<point> points;
  std::vector<std::vector<colindex> > columns;

  static inline bool comp1(const colindex &a, const colindex &b) {
    return a.second < b.second;
  }

  static inline bool comp2(const colindex &a, const T &v) {
    return a.second < v;
  }

  void build(int n, int lo, int hi) {
    if (points[lo].first == points[hi].first) {
      for (int i = lo; i <= hi; i++) {
        columns[n].push_back(point(i, points[i].second));
      }
      return;
    }
    int l = n*2 + 1, r = n*2 + 2;
    build(l, lo, (lo + hi)/2);
    build(r, (lo + hi)/2 + 1, hi);
    columns[n].resize(columns[l].size() + columns[r].size());
    std::merge(columns[l].begin(), columns[l].end(),
               columns[r].begin(), columns[r].end(),
               columns[n].begin(), comp1);
  }

  T r1, c1, r2, c2;  // Helper variables for query().

  template<class ReportFunction>
  void query(int n, int lo, int hi, ReportFunction f) {
    if (points[hi].first < r1 || r2 < points[lo].first) {
      return;
    }
    if (!(points[lo].first < r1 || r2 < points[hi].first)) {
      if (!columns[n].empty() && !(c2 < c1)) {
        typename std::vector<point>::iterator it;
        it = std::lower_bound(columns[n].begin(), columns[n].end(), c1, comp2);
        for (; it != columns[n].end() && it->second <= c2; ++it) {
          f(it->first, points[it->first]);
        }
      }
    } else if (lo != hi) {
      query(n*2 + 1, lo, (lo + hi)/2, f);
      query(n*2 + 2, (lo + hi)/2 + 1, hi, f);
    }
  }

 public:
  template<class It> range_tree_2d(It lo, It hi) {
    int n = std::distance(lo, hi);
    columns.resize(4*n + 1);
    points.assign(lo, hi);
    std::sort(points.begin(), points.end());
    build(0, 0, n - 1);
  }

  template<class ReportFunction>
  void query(const T &r1, const T &c1, const T &r2, const T &c2,
             ReportFunction f) {
    this->r1 = r1;
    this->c1 = c1;
    this->r2 = r2;
    this->c2 = c2;
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
  const int n = 10;
  int points[n][2] = {{1, 4}, {5, 4}, {2, 2}, {3, 1}, {6, -5}, {5, -1},
                      {3, -3}, {-1, -2}, {-1, -1}, {2, -1}};
  vector<pair<int, int> > v;
  for (int i = 0; i < n; i++) {
    v.push_back(make_pair(points[i][0], points[i][1]));
  }
  range_tree_2d<int> t(v.begin(), v.end());
  t.query(-1, -1, 2, 5, print);
  cout << endl;
  t.query(1, 1, 4, 8, print);
  cout << endl;
  return 0;
}
