/*

Given a list of points, finds the closest pair using divide and conquer.

- `closest_pair(lo, hi, &res)` returns the minimum squared distance between any two points in
  `[lo, hi)`. The input range is sorted after the call. The closest pair is stored into `res` if
  non-null. The function is templated on the iterator type. The returned minimum squared distance
  preserves the coordinate arithmetic type, so integer-coordinate inputs stay exact. The result pair
  contains the original point type.

Overflow warning: the squared distance grows like the squared coordinate magnitude. For integer
inputs use a 64-bit coordinate type (e.g. `PointL` from 7.1.1) once coordinates exceed a few tens of
thousands, or the 32-bit products overflow.

Time Complexity:
- O(n log^2 n) per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n log^2 n) auxiliary stack space.

*/

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)

template<class It, class T>
T closest_pair(
    It lo, It hi,
    std::pair<
        typename std::iterator_traits<It>::value_type,
        typename std::iterator_traits<It>::value_type> *res = nullptr,
    T mindist = std::numeric_limits<T>::max(), bool sort_x = true
) {
  using Pt = typename std::iterator_traits<It>::value_type;
  if (lo == hi) {
    return mindist;
  }
  if (sort_x) {
    std::sort(lo, hi, [](const Pt &a, const Pt &b) { return a.x != b.x ? a.x < b.x : a.y < b.y; });
  }
  It mid = lo + (hi - lo) / 2;
  auto midx = mid->x;
  T d1 = closest_pair(lo, mid, res, mindist, false);
  mindist = std::min(mindist, d1);
  T d2 = closest_pair(mid + 1, hi, res, mindist, false);
  mindist = std::min(mindist, d2);
  std::sort(lo, hi, [](const Pt &a, const Pt &b) { return a.y != b.y ? a.y < b.y : a.x < b.x; });
  std::vector<It> t;
  for (It it = lo; it != hi; ++it) {
    auto dx = it->x - midx;
    if (dx * dx < mindist) {
      t.push_back(it);
    }
  }
  for (int i = 0; i < static_cast<int>(t.size()); i++) {
    for (int j = i + 1; j < static_cast<int>(t.size()); j++) {
      const auto &a = *t[i], &b = *t[j];
      auto dy = b.y - a.y;
      if (dy * dy >= mindist) {
        break;
      }
      auto dx = a.x - b.x;
      auto dist = dx * dx + dy * dy;
      if (mindist > dist) {
        mindist = dist;
        if (res) {
          *res = {a, b};
        }
      }
    }
  }
  return mindist;
}

template<class It>
auto closest_pair(
    It lo, It hi,
    std::pair<
        typename std::iterator_traits<It>::value_type,
        typename std::iterator_traits<It>::value_type> *res = nullptr
) {
  using T = decltype((lo->x - lo->x) * (lo->x - lo->x) + (lo->y - lo->y) * (lo->y - lo->y));
  return closest_pair(lo, hi, res, std::numeric_limits<T>::max(), true);
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
  bool operator==(const PointI &p) const { return x == p.x && y == p.y; }
};

int main() {
  vector<Point> v{{2, 3}, {12, 30}, {40, 50}, {5, 1}, {12, 10}, {3, 4}};
  pair<Point, Point> res;
  assert(EQ(closest_pair(v.begin(), v.end(), &res), 2));
  auto [p1, p2] = res;
  assert(p1 == Point(2, 3) && p2 == Point(3, 4));

  // Integer-coordinate input: exact pair selection, int squared distance returned.
  vector<PointI> iv{{0, 0}, {10, 10}, {3, 4}};
  pair<PointI, PointI> ires;
  assert(closest_pair(iv.begin(), iv.end(), &ires) == 25);  // (0,0)-(3,4) or (3,4)-(0,0)
  auto [i1, i2] = ires;
  assert((i1 == PointI(0, 0) && i2 == PointI(3, 4)) || (i1 == PointI(3, 4) && i2 == PointI(0, 0)));

  return 0;
}
