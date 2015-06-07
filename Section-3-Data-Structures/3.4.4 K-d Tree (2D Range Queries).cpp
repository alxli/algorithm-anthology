/*

3.4.4 - K-d Tree for 2D Rectangular Queries

Description: k-d tree (short for k-dimensional tree) is a space-
partitioning data structure for organizing points in a k-
dimensional space. The following implementation supports
counting the number of points in rectangular ranges after the tree
has been build.

Time Complexity: O(N log N) for build(), where N is the number of
points in the tree. count() is O(sqrt(N)).

Space Complexity: O(N) on the number of points.

*/

#include <algorithm> /* nth_element(), max(), min() */
#include <climits>   /* INT_MIN, INT_MAX */
#include <utility>
#include <vector>

class kd_tree {

  typedef std::pair<int, int> point;

  static inline bool cmp_x(const point &a, const point &b) {
    return a.first < b.first;
  }

  static inline bool cmp_y(const point &a, const point &b) {
    return a.second < b.second;
  }

  std::vector<int> tx, ty, cnt, minx, miny, maxx, maxy;
  int xl, yl, xh, yh; //temporary values to speed up recursion

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
    if (ax > xh || xl > bx || ay > yh || yl > by) return 0;
    if (xl <= ax && bx <= xh && yl <= ay && by <= yh) return cnt[mid];
    int res = count(lo, mid) + count(mid + 1, hi);
    res += (xl <= tx[mid] && tx[mid] <= xh && yl <= ty[mid] && ty[mid] <= yh);
    return res;
  }

 public:
  kd_tree(int N, point P[]) {
    tx.resize(N);
    ty.resize(N);
    cnt.resize(N);
    minx.resize(N);
    maxx.resize(N);
    miny.resize(N);
    maxy.resize(N);
    build(0, N, true, P);
  }

  int count(int xl, int yl, int xh, int yh) {
    this->xl = xl; this->yl = yl;
    this->xh = xh; this->yh = yh;
    return count(0, tx.size());
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  pair<int, int> P[4];
  P[0] = make_pair(0, 0);
  P[1] = make_pair(10, 10);
  P[2] = make_pair(0, 10);
  P[3] = make_pair(10, 0);
  kd_tree T(4, P);
  cout << T.count(0, 0, 10, 9) << "\n";  //2
  cout << T.count(0, 0, 10, 10) << "\n"; //4
  return 0;
}
