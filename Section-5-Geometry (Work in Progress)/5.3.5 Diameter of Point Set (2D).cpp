/*

Determines the diametral pair of a range of points. The diamter
of a set of points is the largest distance between any two
points in the set. A diametral pair is a pair of points in the
set whose distance is equal to the set's diameter. The following
program uses rotating calipers method to find a solution.

Time Complexity: O(n log n) on the number of points in the set.

*/

#include <algorithm> /* std::sort() */
#include <cmath>     /* fabs(), sqrt() */
#include <utility>   /* std::pair */
#include <vector>

typedef std::pair<double, double> point;
#define x first
#define y second

double sqdist(const point & a, const point & b) {
  double dx = a.x - b.x, dy = a.y - b.y;
  return sqrt(dx * dx + dy * dy);
}

double cross(const point & o, const point & a, const point & b) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

bool cw(const point & o, const point & a, const point & b) {
  return cross(o, a, b) < 0;
}

double area(const point & o, const point & a, const point & b) {
  return fabs(cross(o, a, b));
}

template<class It> std::vector<point> convex_hull(It lo, It hi) {
  int k = 0;
  if (hi - lo <= 1) return std::vector<point>(lo, hi);
  std::vector<point> res(2 * (int)(hi - lo));
  std::sort(lo, hi); //compare by x, then by y if x-values are equal
  for (It it = lo; it != hi; ++it) {
    while (k >= 2 && !cw(res[k - 2], res[k - 1], *it)) k--;
    res[k++] = *it;
  }
  int t = k + 1;
  for (It it = hi - 2; it != lo - 1; --it) {
    while (k >= t && !cw(res[k - 2], res[k - 1], *it)) k--;
    res[k++] = *it;
  }
  res.resize(k - 1);
  return res;
}

template<class It> std::pair<point, point> diametral_pair(It lo, It hi) {
  std::vector<point> h = convex_hull(lo, hi);
  int m = h.size();
  if (m == 1) return std::make_pair(h[0], h[0]);
  if (m == 2) return std::make_pair(h[0], h[1]);
  int k = 1;
  while (area(h[m - 1], h[0], h[(k + 1) % m]) > area(h[m - 1], h[0], h[k]))
    k++;
  double maxdist = 0, d;
  std::pair<point, point> res;
  for (int i = 0, j = k; i <= k && j < m; i++) {
    d = sqdist(h[i], h[j]);
    if (d > maxdist) {
      maxdist = d;
      res = std::make_pair(h[i], h[j]);
    }
    while (j < m && area(h[i], h[(i + 1) % m], h[(j + 1) % m]) >
                    area(h[i], h[(i + 1) % m], h[j])) {
      d = sqdist(h[i], h[(j + 1) % m]);
      if (d > maxdist) {
        maxdist = d;
        res = std::make_pair(h[i], h[(j + 1) % m]);
      }
      j++;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  vector<point> v;
  v.push_back(point(0, 0));
  v.push_back(point(3, 0));
  v.push_back(point(0, 3));
  v.push_back(point(1, 1));
  v.push_back(point(4, 4));
  pair<point, point> res = diametral_pair(v.begin(), v.end());
  cout << "diametral pair: (" << res.first.x << "," << res.first.y << ") ";
  cout << "(" << res.second.x << "," << res.second.y << ")\n";
  cout << "diameter: " << sqrt(sqdist(res.first, res.second)) << "\n";
  return 0;
}
