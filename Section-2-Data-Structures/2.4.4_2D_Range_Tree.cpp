/*

Maintain a set of two-dimensional points while supporting queries for all points
that fall inside given rectangular regions. This implementation uses std::pair
to represent points, requiring operators < and == to be defined on the numeric
template type.

- range_tree(lo, hi) constructs a set from two RandomAccessIterators to
  std::pair as a range [lo, hi) of points.
- query(x1, y1, x2, y2, f) calls the function f(i, p) on each point in the set
  that falls into the rectangular region consisting of rows from x1 to x2,
  inclusive, and columns from y1 to y2, inclusive. The first argument to f is
  the zero-based index of the point in the original range given to the
  constructor. The second argument is the point itself as an std::pair.

Time Complexity:
- O(n log n) per call to the constructor, where n is the number of points.
- O(log^2(n) + m) per call to query(), where m is the number of points that are
  reported by the query.

Space Complexity:
- O(n log n) for storage of the points.
- O(log^2(n)) auxiliary stack space for query().

*/

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

template<class T>
class range_tree {
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
    int l = n*2 + 1, r = n*2 + 2, mid = lo + (hi - lo)/2;
    build(l, lo, mid);
    build(r, mid + 1, hi);
    columns[n].resize(columns[l].size() + columns[r].size());
    std::merge(columns[l].begin(), columns[l].end(),
               columns[r].begin(), columns[r].end(),
               columns[n].begin(), comp1);
  }

  // Helper variables for query().
  T x1, y1, x2, y2;

  template<class ReportFunction>
  void query(int n, int lo, int hi, ReportFunction f) {
    if (points[hi].first < x1 || x2 < points[lo].first) {
      return;
    }
    if (!(points[lo].first < x1 || x2 < points[hi].first)) {
      if (!columns[n].empty() && !(y2 < y1)) {
        typename std::vector<point>::iterator it;
        it = std::lower_bound(columns[n].begin(), columns[n].end(), y1, comp2);
        for (; it != columns[n].end() && it->second <= y2; ++it) {
          f(it->first, points[it->first]);
        }
      }
    } else if (lo != hi) {
      int mid = lo + (hi - lo)/2;
      query(n*2 + 1, lo, mid, f);
      query(n*2 + 2, mid + 1, hi, f);
    }
  }

 public:
  template<class It>
  range_tree(It lo, It hi) : points(lo, hi) {
    int n = std::distance(lo, hi);
    columns.resize(4*n + 1);
    std::sort(points.begin(), points.end());
    build(0, 0, n - 1);
  }

  template<class ReportFunction>
  void query(const T &x1, const T &y1, const T &x2, const T &y2,
             ReportFunction f) {
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
  const int n = 10;
  int points[n][2] = {{1, 4}, {5, 4}, {2, 2}, {3, 1}, {6, -5}, {5, -1},
                      {3, -3}, {-1, -2}, {-1, -1}, {2, -1}};
  vector<pair<int, int> > v;
  for (int i = 0; i < n; i++) {
    v.push_back(make_pair(points[i][0], points[i][1]));
  }
  range_tree<int> t(v.begin(), v.end());
  t.query(-1, -1, 2, 5, print);
  cout << endl;
  t.query(1, 1, 4, 8, print);
  cout << endl;
  return 0;
}
