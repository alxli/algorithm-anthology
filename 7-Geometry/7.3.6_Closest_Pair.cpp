/*

Given a list of points in two dimensions, finds the closest pair using a divide-and-conquer
algorithm. The points are split in half by $x$-coordinate and each half is solved recursively; the
combining step then only needs to examine points within the best distance so far of the dividing
line, where each point in this strip is compared to a constant number of $y$-ordered neighbors.

- `closest_pair(p, &res)` returns the minimum squared distance between any two points in `p`. If
  `res` is non-null, one closest pair is stored there in lexicographic order. With fewer than two
  points, the maximum value of the squared-distance type is returned and `res` is unchanged. The
  point type may be any type exposing numeric `.x` and `.y` members and a lexicographic `operator<`.
  The distance preserves the coordinate arithmetic type, and the returned pair preserves the point
  type. For integer coordinates, the distance is exact provided intermediate products do not
  overflow.

Overflow warning: squared distances grow like the square of the coordinate magnitude. For integer
point types, use a 64-bit coordinate type (e.g. `PointL` from 7.1.1) when coordinates may exceed a
few tens of thousands.

Time Complexity:
- O(n log n) per call, where $n$ is the number of points.

Space Complexity:
- O(n) auxiliary heap space.
- O(log n) auxiliary stack space.

*/

#include <algorithm>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

template<typename Pt>
auto sqdist(const Pt &a, const Pt &b) {
  auto dx = b.x - a.x, dy = b.y - a.y;
  return dx * dx + dy * dy;  // Overflow warning.
}

template<typename It, typename T, typename Pt = typename std::iterator_traits<It>::value_type>
void closest_pair_rec(It lo, It hi, std::vector<Pt> &tmp, T &best, std::pair<Pt, Pt> *res) {
  int n = hi - lo;
  auto by_y = [](const Pt &a, const Pt &b) { return a.y != b.y ? a.y < b.y : a.x < b.x; };
  if (n <= 3) {
    for (It i = lo; i != hi; ++i) {
      for (It j = i + 1; j != hi; ++j) {
        T d = sqdist(*i, *j);
        if (d < best) {
          best = d;
          if (res != nullptr) {
            *res = std::minmax(*i, *j);
          }
        }
      }
    }
    std::sort(lo, hi, by_y);
    return;
  }
  It mid = lo + n / 2;
  auto midx = mid->x;
  closest_pair_rec(lo, mid, tmp, best, res);
  closest_pair_rec(mid, hi, tmp, best, res);
  // Each half is now y-sorted, so merge them before examining the center strip.
  tmp.clear();
  std::merge(lo, mid, mid, hi, std::back_inserter(tmp), by_y);
  std::move(tmp.begin(), tmp.end(), lo);
  tmp.clear();
  for (It it = lo; it != hi; ++it) {
    auto dx = it->x - midx;
    if (dx * dx < best) {
      tmp.push_back(*it);
    }
  }
  for (int i = 0; i < static_cast<int>(tmp.size()); i++) {
    for (int j = i + 1; j < static_cast<int>(tmp.size()); j++) {
      auto dy = tmp[j].y - tmp[i].y;
      if (dy * dy >= best) {
        break;
      }
      T d = sqdist(tmp[i], tmp[j]);
      if (d < best) {
        best = d;
        if (res != nullptr) {
          *res = std::minmax(tmp[i], tmp[j]);
        }
      }
    }
  }
  tmp.clear();
}

template<typename Pt>
auto closest_pair(std::vector<Pt> p, std::pair<Pt, Pt> *res = nullptr) {
  using T = decltype(sqdist(p[0], p[0]));
  T best = std::numeric_limits<T>::max();
  std::sort(p.begin(), p.end(), [](const Pt &a, const Pt &b) {
    return a.x != b.x ? a.x < b.x : a.y < b.y;
  });
  std::vector<Pt> tmp;
  tmp.reserve(p.size());
  closest_pair_rec(p.begin(), p.end(), tmp, best, res);
  return best;
}

/*** Example Usage ***/

#include <cassert>
#include <cmath>
#include <vector>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
  bool operator==(const PointI &p) const { return x == p.x && y == p.y; }
  bool operator<(const PointI &o) const { return x != o.x ? x < o.x : y < o.y; }
};

int main() {
  vector<Point> v{{2, 3}, {12, 30}, {40, 50}, {5, 1}, {12, 10}, {3, 4}};
  pair<Point, Point> res;
  assert(EQ(closest_pair(v, &res), 2));
  auto [p1, p2] = res;
  assert(p1 == Point(2, 3) && p2 == Point(3, 4));

  // Integer-coordinate input: exact pair selection, int squared distance returned.
  vector<PointI> iv{{0, 0}, {10, 10}, {3, 4}};
  pair<PointI, PointI> ires;
  assert(closest_pair(iv, &ires) == 25);
  auto [i1, i2] = ires;
  assert(i1 == PointI(0, 0) && i2 == PointI(3, 4));
  return 0;
}
