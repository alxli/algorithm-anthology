#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

template<class T> class kd_tree {
  typedef std::pair<T, T> point;

  static inline bool comp1(const point &a, const point &b) {
    return a.first < b.first;
  }

  static inline bool comp2(const point &a, const point &b) {
    return a.second < b.second;
  }

  std::vector<point> tree, minp, maxp;
  std::vector<int> count;

  template<class It>
  void build(int lo, int hi, bool row, It points) {
    if (lo >= hi) {
      return;
    }
    int mid = (lo + hi)/2;
    std::nth_element(points + lo, points + mid, points + hi,
                     row ? comp1 : comp2);
    tree[mid] = *(points + mid);
    count[mid] = hi - lo;
    minp[mid].first = minp[mid].second = std::numeric_limits<T>::max();
    maxp[mid].first = maxp[mid].second = std::numeric_limits<T>::min();
    for (int i = lo; i < hi; i++) {
      minp[mid].first = std::min(minp[mid].first, (points + i)->first);
      minp[mid].second = std::min(minp[mid].second, (points + i)->second);
      maxp[mid].first = std::max(maxp[mid].first, (points + i)->first);
      maxp[mid].second = std::max(maxp[mid].second, (points + i)->second);
    }
    build(lo, mid, !row, points);
    build(mid + 1, hi, !row, points);
  }

  T r1, c1, r2, c2;  // Helper variables for count().

  int query(int lo, int hi) {
    if (lo >= hi) {
      return 0;
    }
    int mid = (lo + hi)/2;
    T ar = minp[mid].first, ac = minp[mid].second;
    T br = maxp[mid].first, bc = maxp[mid].second;
    if (r2 < ar || br < r1 || c2 < ac || bc < c1) {
      return 0;
    }
    if (!(ar < r1 || r2 < br || ac < c1 || c2 < bc)) {
      return count[mid];
    }
    int res = query(lo, mid) + query(mid + 1, hi);
    if (tree[mid].first < r1 || r2 < tree[mid].first ||
        tree[mid].second < c1 || c2 < tree[mid].second) {
      return res;
    }
    return res + 1;
  }

 public:
  template<class It> kd_tree(It lo, It hi) {
    int n = std::distance(lo, hi);
    tree.resize(n);
    count.resize(n);
    minp.resize(n);
    maxp.resize(n);
    build(0, n, true, lo);
  }

  int query(const T &r1, const T &c1, const T &r2, const T &c2) {
    this->r1 = r1;
    this->c1 = c1;
    this->r2 = r2;
    this->c2 = c2;
    return query(0, tree.size());
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  const int n = 10;
  int points[n][2] = {{1, 4}, {5, 4}, {2, 2}, {3, 1}, {6, -5}, {5, -1},
                      {3, -3}, {-1, -2}, {-1, -1}, {2, -1}};
  vector<pair<int, int> > v;
  for (int i = 0; i < n; i++) {
    v.push_back(make_pair(points[i][0], points[i][1]));
  }
  kd_tree<int> t(v.begin(), v.end());
  assert(t.query(-1, -1, 2, 5) == 4);
  assert(t.query(1, 1, 4, 8) == 3);
  return 0;
}
