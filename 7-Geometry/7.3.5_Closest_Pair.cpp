/*

Given a list of points in two dimensions, find the closest pair among them using a divide and
conquer algorithm.

- `closest_pair(lo, hi, &res)` returns the minimum Euclidean distance between any pair of points in
  the range `[lo, hi)`, where `lo` and `hi` must be random-access iterators. The input range will be
  sorted lexicographically (by $x$, then by $y$) after the function call. If there is an answer, the
  closest pair will be stored into pointer `res`.

Time Complexity:
- O(n log^2 n) per call to `closest_pair(lo, hi, &res)`, where $n$ is the distance between `lo` and
  `hi`.

Space Complexity:
- O(n log^2 n) auxiliary stack space for `closest_pair(lo, hi, &res)`, where $n$ is the distance
  between `lo` and `hi`.

*/

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

// clang-format off
double sqnorm(const Point &a) { return a.x * a.x + a.y * a.y; }
double norm(const Point &a) { return sqrt(sqnorm(a)); }
bool cmp_x(const Point &a, const Point &b) { return (a.x == b.x) ? (a.y < b.y) : (a.x < b.x); }
bool cmp_y(const Point &a, const Point &b) { return (a.y == b.y) ? (a.x < b.x) : (a.y < b.y); }
// clang-format on

template<class It>
double closest_pair(
    It lo, It hi, std::pair<Point, Point> *res = nullptr,
    double mindist = DBL_MAX, bool sort_x = true
) {
  if (lo == hi) {
    return DBL_MAX;
  }
  if (sort_x) {
    std::sort(lo, hi, cmp_x);
  }
  It mid = lo + (hi - lo) / 2;
  double midx = mid->x;
  double d1 = closest_pair(lo, mid, res, mindist, false);
  mindist = std::min(mindist, d1);
  double d2 = closest_pair(mid + 1, hi, res, mindist, false);
  mindist = std::min(mindist, d2);
  std::sort(lo, hi, cmp_y);
  std::vector<It> t;
  for (It it = lo; it != hi; ++it) {
    if (fabs(it->x - midx) < mindist) {
      t.push_back(it);
    }
  }
  for (size_t i = 0; i < t.size(); i++) {
    for (size_t j = i + 1; j < t.size(); j++) {
      Point a(*t[i]), b(*t[j]);
      if (b.y - a.y >= mindist) {
        break;
      }
      double dist = norm(Point(a.x - b.x, a.y - b.y));
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

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  vector<Point> v{{2, 3}, {12, 30}, {40, 50}, {5, 1}, {12, 10}, {3, 4}};
  std::pair<Point, Point> res;
  assert(EQ(closest_pair(v.begin(), v.end(), &res), sqrt(2)));
  auto [p1, p2] = res;
  assert(p1 == Point(2, 3));
  assert(p2 == Point(3, 4));
  return 0;
}
