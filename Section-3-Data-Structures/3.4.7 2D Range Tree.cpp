/*

3.4.7 - 2D Range Tree for Rectangular Queries

Description: A range tree is an ordered tree data structure to
hold a list of points. It allows all points within a given range
to be reported efficiently. Specifically, for a given query, a
range tree will report *all* points that lie in the given range.
Note that the initial array passed to construct the tree will be
sorted, and all resulting query reports will pertain to the
indices of points in the sorted array.

Time Complexity: A range tree can build() in O(N log^(d-1)(N))
and query() in O(log^d(n) + k), where N is the number of points
stored in the tree, d is the dimension of each point and k is the
number of points reported by a given query. Thus for this 2D case
build() is O(N log N) and query() is O(log^2(N) + k).

Space Complexity: O(N log^(d-1)(N)) for a d-dimensional range tree.
Thus for this 2D case, the space complexity is O(N log N).

*/

#include <algorithm> /* lower_bound(), merge(), sort() */
#include <utility>   /* std::pair */
#include <vector>

class range_tree_2D {
  typedef std::pair<int, int> point;
  std::vector<point> P;
  std::vector<std::vector<point> > seg;

  static inline bool comp1(const point & a, const point & b) {
    return a.second < b.second;
  }

  static inline bool comp2(const point & a, int v) {
    return a.second < v;
  }

  void build(int n, int lo, int hi) {
    if (P[lo].first == P[hi].first) {
      for (int i = lo; i <= hi; i++)
        seg[n].push_back(point(i, P[i].second));
      return;
    }
    int l = n * 2 + 1, r = n * 2 + 2;
    build(l, lo, (lo + hi)/2);
    build(r, (lo + hi)/2 + 1, hi);
    seg[n].resize(seg[l].size() + seg[r].size());
    std::merge(seg[l].begin(), seg[l].end(), seg[r].begin(), seg[r].end(),
               seg[n].begin(), comp1);
  }

  int xl, xh, yl, yh;

  template<class ReportFunction>
  void query(int n, int lo, int hi, ReportFunction f) {
    if (P[hi].first < xl || P[lo].first > xh) return;
    if (xl <= P[lo].first && P[hi].first <= xh) {
      if (!seg[n].empty() && yh >= yl) {
        std::vector<point>::iterator it;
        it = std::lower_bound(seg[n].begin(), seg[n].end(), yl, comp2);
        for (; it != seg[n].end(); ++it) {
          if (it->second > yh) break;
          f(it->first); //or report P[it->first], the actual point
        }
      }
    } else if (lo != hi) {
      query(n * 2 + 1, lo, (lo + hi) / 2, f);
      query(n * 2 + 2, (lo + hi) / 2 + 1, hi, f);
    }
  }

 public:
  range_tree_2D(int n, point init[]): seg(4 *n + 1) {
    std::sort(init, init + n);
    P = std::vector<point>(init, init + n);
    build(0, 0, n - 1);
  }

  template<class ReportFunction>
  void query(int x1, int y1, int x2, int y2, ReportFunction f) {
    xl = x1; xh = x2;
    yl = y1; yh = y2;
    query(0, 0, P.size() - 1, f);
  }
};

/*** Example Usage (wcipeg.com/problem/boxl) ***/

#include <bitset>
#include <cstdio>
using namespace std;

int N, M; bitset<200005> b;
pair<int, int> pts[200005];
int x1[200005], y1[200005];
int x2[200005], y2[200005];

void mark(int i) {
  b[i] = true;
}

int main() {
  scanf("%d%d", &N, &M);
  for (int i = 0; i < N; i++)
    scanf("%d%d%d%d", x1 + i, y1 + i, x2 + i, y2 + i);
  for (int i = 0; i < M; i++)
    scanf("%d%d", &pts[i].first, &pts[i].second);
  range_tree_2D T(M, pts);
  for (int i = 0; i < N; i++)
    T.query(x1[i], y1[i], x2[i], y2[i], mark);
  printf("%d\n", b.count());
  return 0;
}
