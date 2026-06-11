/*

Given a list of points in two dimensions, finds the closest pair using a divide-and-conquer
algorithm. The points are split in half by $x$-coordinate and each half is solved recursively; the
combining step then only needs to examine points within the best distance so far of the dividing
line, where each point in this strip is compared to a constant number of $y$-ordered neighbors.

- `closest_pair(lo, hi, &res)` returns the minimum squared distance between any two points in the
  range `[lo, hi)`, where `lo` and `hi` must be random-access iterators. The input range is
  reordered during the computation and is sorted by $y$-coordinate on return. If `res` is non-null,
  one closest pair is stored there in lexicographic order. The function is templated on the point
  type and works with any type exposing numeric `.x` and `.y` members.

The returned distance preserves the coordinate arithmetic type. For integer-coordinate inputs, the
result is therefore an exact squared distance provided intermediate products do not overflow. The
returned pair contains the original point type.

Overflow warning: squared distances grow like the square of the coordinate magnitude. For integer
point types, use a 64-bit coordinate type (e.g. `PointL` from 7.1.1) when coordinates may exceed a
few tens of thousands.

Time Complexity:
- O(n log^2 n) per call to `closest_pair(lo, hi, &res)`, where n is the distance between `lo` and
  `hi`.

Space Complexity:
- O(n) auxiliary heap space.
- O(log n) recursion stack space.

*/

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

template<class Pt>
auto sqdist(const Pt &a, const Pt &b) {
  auto dx = a.x - b.x, dy = a.y - b.y;
  return dx * dx + dy * dy;  // Overflow warning!
}

template<class It, class T>
T closest_pair_rec(
    It lo, It hi,
    std::pair<
        typename std::iterator_traits<It>::value_type,
        typename std::iterator_traits<It>::value_type> *res
) {
  using Pt = typename std::iterator_traits<It>::value_type;
  int n = hi - lo;
  if (n < 2) {
    return std::numeric_limits<T>::max();
  }
  std::sort(lo, hi, [](const Pt &a, const Pt &b) { return a.x != b.x ? a.x < b.x : a.y < b.y; });
  if (n <= 3) {
    T best = std::numeric_limits<T>::max();
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
    return best;
  }
  It mid = lo + n / 2;
  auto midx = mid->x;
  std::pair<Pt, Pt> lres, rres;
  T dl = closest_pair_rec<It, T>(lo, mid, &lres);
  T dr = closest_pair_rec<It, T>(mid, hi, &rres);
  T best;
  if (dl <= dr) {
    best = dl;
    if (res && dl != std::numeric_limits<T>::max()) {
      *res = lres;
    }
  } else {
    best = dr;
    if (res && dr != std::numeric_limits<T>::max()) {
      *res = rres;
    }
  }
  std::sort(lo, hi, [](const Pt &a, const Pt &b) { return a.y != b.y ? a.y < b.y : a.x < b.x; });
  std::vector<It> strip;
  for (It it = lo; it != hi; ++it) {
    auto dx = it->x - midx;
    if (dx * dx < best) {
      strip.push_back(it);
    }
  }
  for (int i = 0; i < (int)strip.size(); i++) {
    for (int j = i + 1; j < (int)strip.size(); j++) {
      auto dy = strip[j]->y - strip[i]->y;
      if (dy * dy >= best) break;

      T d = sqdist(*strip[i], *strip[j]);
      if (d < best) {
        best = d;
        if (res != nullptr) {
          *res = std::minmax(*strip[i], *strip[j]);
        }
      }
    }
  }
  return best;
}

template<class It>
auto closest_pair(
    It lo, It hi,
    std::pair<
        typename std::iterator_traits<It>::value_type,
        typename std::iterator_traits<It>::value_type> *res = nullptr
) {
  using T = decltype(sqdist(*lo, *lo));
  return closest_pair_rec<It, T>(lo, hi, res);
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

const double EPS = 1e-9;
#define EQ(a, b) (fabs((a) - (b)) <= EPS)

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
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
