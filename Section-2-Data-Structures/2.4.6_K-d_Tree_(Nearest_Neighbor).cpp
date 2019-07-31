/*

Maintain a set of two-dimensional points while supporting queries for the
closest point in the set to a given query point. This implementation uses
std::pair to represent points, requiring operators <, ==, -, and long double
casting to be defined on the numeric template type.

- kd_tree(lo, hi) constructs a set from two RandomAccessIterators to std::pair
  as a range [lo, hi) of points.
- nearest(x, y, can_equal) returns a point in the set that is closest to (x, y)
  by Euclidean distance. This may be equal to (x, y) only if can_equal is true.

Time Complexity:
- O(n log n) per call to the constructor, where n is the number of points.
- O(log n) on average per call to nearest().

Space Complexity:
- O(n) for storage of the points.
- O(log n) auxiliary stack space for nearest().

*/

#include <algorithm>
#include <limits>
#include <stdexcept>
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

  std::vector<point> tree;
  std::vector<bool> div_x;

  void build(int lo, int hi) {
    if (lo >= hi) {
      return;
    }
    int mid = lo + (hi - lo)/2;
    T minx, maxx, miny, maxy;
    minx = maxx = tree[lo].first;
    miny = maxy = tree[lo].second;
    for (int i = lo + 1; i < hi; i++) {
      minx = std::min(minx, tree[i].first);
      miny = std::min(miny, tree[i].second);
      maxx = std::max(maxx, tree[i].first);
      maxy = std::max(maxy, tree[i].second);
    }
    div_x[mid] = !((maxx - minx) < (maxy - miny));
    std::nth_element(tree.begin() + lo, tree.begin() + mid, tree.begin() + hi,
                     div_x[mid] ? comp1 : comp2);
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
    int mid = lo + (hi - lo)/2;
    T dx = x - tree[mid].first, dy = y - tree[mid].second;
    long double d = dx*(long double)dx + dy*(long double)dy;
    if (d < min_dist && (can_equal || d != 0)) {
      min_dist = d;
      id = mid;
    }
    if (lo + 1 == hi) {
      return;
    }
    d = (long double)(div_x[mid] ? dx : dy);
    int l1 = lo, r1 = mid, l2 = mid + 1, r2 = hi;
    if (d > 0) {
      std::swap(l1, l2);
      std::swap(r1, r2);
    }
    nearest(l1, r1, x, y, can_equal);
    if (d*(long double)d < min_dist) {
      nearest(l2, r2, x, y, can_equal);
    }
  }

 public:
  template<class It>
  kd_tree(It lo, It hi) : tree(lo, hi) {
    int n = std::distance(lo, hi);
    if (n <= 1) {
      throw std::runtime_error("K-d tree must be have at least 2 points.");
    }
    div_x.resize(n);
    build(0, n);
  }

  point nearest(const T &x, const T &y, bool can_equal = true) {
    min_dist = std::numeric_limits<long double>::max();
    nearest(0, tree.size(), x, y, can_equal);
    return tree[id];
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  pair<int, int> p[3];
  p[0] = make_pair(0, 2);
  p[1] = make_pair(0, 3);
  p[2] = make_pair(-1, 0);
  kd_tree<int> t(p, p + 3);
  assert(t.nearest(0, 2, true) == make_pair(0, 2));
  assert(t.nearest(0, 2, false) == make_pair(0, 3));
  assert(t.nearest(0, 0) == make_pair(-1, 0));
  assert(t.nearest(-10000, 0) == make_pair(-1, 0));
  return 0;
}
