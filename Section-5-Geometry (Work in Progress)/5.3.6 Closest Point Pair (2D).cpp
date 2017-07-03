/*

Given a range containing distinct points on the Cartesian plane,
determine two points which have the closest possible distance.
A divide and conquer algorithm is used. Note that the ordering
of points in the input range may be changed by the function.

Time Complexity: O(n log^2 n) where n is the number of points.

*/

#include <algorithm> /* std::min, std::sort */
#include <cfloat>    /* DBL_MAX */
#include <cmath>     /* fabs */
#include <utility>   /* std::pair */

typedef std::pair<double, double> point;
#define x first
#define y second

double sqdist(const point & a, const point & b) {
  double dx = a.x - b.x, dy = a.y - b.y;
  return dx * dx + dy * dy;
}

bool cmp_x(const point & a, const point & b) { return a.x < b.x; }
bool cmp_y(const point & a, const point & b) { return a.y < b.y; }

template<class It>
double rec(It lo, It hi, std::pair<point, point> & res, double mindist) {
  if (lo == hi) return DBL_MAX;
  It mid = lo + (hi - lo) / 2;
  double midx = mid->x;
  double d1 = rec(lo, mid, res, mindist);
  mindist = std::min(mindist, d1);
  double d2 = rec(mid + 1, hi, res, mindist);
  mindist = std::min(mindist, d2);
  std::sort(lo, hi, cmp_y);
  int size = 0;
  It t[hi - lo];
  for (It it = lo; it != hi; ++it)
    if (fabs(it->x - midx) < mindist)
      t[size++] = it;
  for (int i = 0; i < size; i++) {
    for (int j = i + 1; j < size; j++) {
      point a = *t[i], b = *t[j];
      if (b.y - a.y >= mindist) break;
      double dist = sqdist(a, b);
      if (mindist > dist) {
        mindist = dist;
        res = std::make_pair(a, b);
      }
    }
  }
  return mindist;
}

template<class It> std::pair<point, point> closest_pair(It lo, It hi) {
  std::pair<point, point> res;
  std::sort(lo, hi, cmp_x);
  rec(lo, hi, res, DBL_MAX);
  return res;
}

/*** Example Usage ***/

#include <iostream>
#include <vector>
using namespace std;

int main() {
  vector<point> v;
  v.push_back(point(2, 3));
  v.push_back(point(12, 30));
  v.push_back(point(40, 50));
  v.push_back(point(5, 1));
  v.push_back(point(12, 10));
  v.push_back(point(3, 4));
  pair<point, point> res = closest_pair(v.begin(), v.end());
  cout << "closest pair: (" << res.first.x << "," << res.first.y << ") ";
  cout << "(" << res.second.x << "," << res.second.y << ")\n";
  cout << "dist: " << sqrt(sqdist(res.first, res.second)) << "\n"; //1.41421
  return 0;
}
