/*

Description: k-d tree (short for k-dimensional tree) is a space-
partitioning data structure for organizing points in a k-
dimensional space. The following implementation supports
querying the nearest neighboring point to (x, y) in terms of
Euclidean distance after the tree has been build. Note that
a point is not considered its own neighbour if it already exists
in the tree.

Time Complexity: O(N log N) for build(), where N is the number of
points in the tree. nearest_neighbor_id() is O(log(N)) on average.

Space Complexity: O(N) on the number of points.

*/

#include <algorithm> /* nth_element(), max(), min(), swap() */
#include <climits>   /* INT_MIN, INT_MAX */
#include <utility>
#include <vector>

class kd_tree {
  typedef std::pair<int, int> point;

  static inline bool cmp_x(const point & a, const point & b) {
    return a.first < b.first;
  }

  static inline bool cmp_y(const point & a, const point & b) {
    return a.second < b.second;
  }

  std::vector<int> tx, ty;
  std::vector<bool> div_x;

  void build(int lo, int hi, point P[]) {
    if (lo >= hi) return;
    int mid = (lo + hi) >> 1;
    int minx = INT_MAX, maxx = INT_MIN;
    int miny = INT_MAX, maxy = INT_MIN;
    for (int i = lo; i < hi; i++) {
      minx = std::min(minx, P[i].first);
      maxx = std::max(maxx, P[i].first);
      miny = std::min(miny, P[i].second);
      maxy = std::max(maxy, P[i].second);
    }
    div_x[mid] = (maxx - minx) >= (maxy - miny);
    std::nth_element(P + lo, P + mid, P + hi, div_x[mid] ? cmp_x : cmp_y);
    tx[mid] = P[mid].first;
    ty[mid] = P[mid].second;
    if (lo + 1 == hi) return;
    build(lo, mid, P);
    build(mid + 1, hi, P);
  }

  long long min_dist;
  int min_dist_id, x, y;

  void nearest_neighbor(int lo, int hi) {
    if (lo >= hi) return;
    int mid = (lo + hi) >> 1;
    int dx = x - tx[mid], dy = y - ty[mid];
    long long d = dx*(long long)dx + dy*(long long)dy;
    if (min_dist > d && d) {
      min_dist = d;
      min_dist_id = mid;
    }
    if (lo + 1 == hi) return;
    int delta = div_x[mid] ? dx : dy;
    long long delta2 = delta*(long long)delta;
    int l1 = lo, r1 = mid, l2 = mid + 1, r2 = hi;
    if (delta > 0) std::swap(l1, l2), std::swap(r1, r2);
    nearest_neighbor(l1, r1);
    if (delta2 < min_dist) nearest_neighbor(l2, r2);
  }

 public:
  kd_tree(int N, point P[]) {
    tx.resize(N);
    ty.resize(N);
    div_x.resize(N);
    build(0, N, P);
  }

  int nearest_neighbor_id(int x, int y) {
    this->x = x; this->y = y;
    min_dist = LLONG_MAX;
    nearest_neighbor(0, tx.size());
    return min_dist_id;
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  pair<int, int> P[3];
  P[0] = make_pair(0, 2);
  P[1] = make_pair(0, 3);
  P[2] = make_pair(-1, 0);
  kd_tree T(3, P);
  int res = T.nearest_neighbor_id(0, 0);
  cout << P[res].first << " " << P[res].second << "\n"; //-1, 0
  return 0;
}
