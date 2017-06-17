/*

Description: k-d tree (short for k-dimensional tree) is a space-
partitioning data structure for organizing points in a k-
dimensional space. The following implementation supports
counting the number of points in rectangular ranges after the
tree has been build.

Time Complexity: O(N log N) for build(), where N is the number
of points in the tree. count() is O(sqrt N).

Space Complexity: O(N) on the number of points.

*/

#include <algorithm>
#include <climits>
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

  std::vector<int> tx, ty, cnt, minx, miny, maxx, maxy;
  int x1, y1, x2, y2; //temporary values to speed up recursion

  void build(int lo, int hi, bool div_x, point P[]) {
    if (lo >= hi) return;
    int mid = (lo + hi) >> 1;
    std::nth_element(P + lo, P + mid, P + hi, div_x ? cmp_x : cmp_y);
    tx[mid] = P[mid].first;
    ty[mid] = P[mid].second;
    cnt[mid] = hi - lo;
    minx[mid] = INT_MAX; miny[mid] = INT_MAX;
    maxx[mid] = INT_MIN; maxy[mid] = INT_MIN;
    for (int i = lo; i < hi; i++) {
      minx[mid] = std::min(minx[mid], P[i].first);
      maxx[mid] = std::max(maxx[mid], P[i].first);
      miny[mid] = std::min(miny[mid], P[i].second);
      maxy[mid] = std::max(maxy[mid], P[i].second);
    }
    build(lo, mid, !div_x, P);
    build(mid + 1, hi, !div_x, P);
  }

  int count(int lo, int hi) {
    if (lo >= hi) return 0;
    int mid = (lo + hi) >> 1;
    int ax = minx[mid], ay = miny[mid];
    int bx = maxx[mid], by = maxy[mid];
    if (ax > x2 || x1 > bx || ay > y2 || y1 > by) return 0;
    if (x1 <= ax && bx <= x2 && y1 <= ay && by <= y2) return cnt[mid];
    int res = count(lo, mid) + count(mid + 1, hi);
    res += (x1 <= tx[mid] && tx[mid] <= x2 && y1 <= ty[mid] && ty[mid] <= y2);
    return res;
  }

 public:
  kd_tree(int n, point P[]): tx(n), ty(n), cnt(n),
    minx(n), miny(n), maxx(n), maxy(n) {
     build(0, n, true, P);
  }

  int count(int x1, int y1, int x2, int y2) {
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    return count(0, tx.size());
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  pair<int, int> P[4];
  P[0] = make_pair(0, 0);
  P[1] = make_pair(10, 10);
  P[2] = make_pair(0, 10);
  P[3] = make_pair(10, 0);
  kd_tree t(4, P);
  assert(t.count(0, 0, 10, 9) == 2);
  assert(t.count(0, 0, 10, 10) == 4);
  return 0;
}
