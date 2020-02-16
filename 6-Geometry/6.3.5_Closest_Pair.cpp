/*

Given a list of points in two dimensions, find the closest pair among them using
a divide and conquer algorithm.

- closest_pair(lo, hi, &res) returns the minimum Euclidean distance between any
  two pair of points in the range [lo, hi), where lo and hi must be
  random-access iterators. The input range will be sorted lexicographically (by
  x, then by y) after the function call. If there is an answer, the closest pair
  will be stored into pointer *res.

Time Complexity:
- O(n log^2 n) per call to closest_pair(lo, hi, &res), where n is the distance
  between lo and hi.

Space Complexity:
- O(n log^2 n) auxiliary stack space for closest_pair(lo, hi, &res), where n is
  the distance between lo and hi.

*/

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double sqnorm(const point &a) { return a.x*a.x + a.y*a.y; }
double norm(const point &a) { return sqrt(sqnorm(a)); }
bool cmp_x(const point &a, const point &b) { return LT(a.x, b.x); }
bool cmp_y(const point &a, const point &b) { return LT(a.y, b.y); }

template<class It>
double closest_pair(It lo, It hi, std::pair<point, point> *res = NULL,
                    double mindist = std::numeric_limits<double>::max(),
                    bool sort_x = true) {
  if (lo == hi) {
    return std::numeric_limits<double>::max();
  }
  if (sort_x) {
    std::sort(lo, hi, cmp_x);
  }
  It mid = lo + (hi - lo)/2;
  double midx = mid->x;
  double d1 = closest_pair(lo, mid, res, mindist, false);
  mindist = std::min(mindist, d1);
  double d2 = closest_pair(mid + 1, hi, res, mindist, false);
  mindist = std::min(mindist, d2);
  std::sort(lo, hi, cmp_y);
  int size = 0;
  It t[hi - lo];
  for (It it = lo; it != hi; ++it) {
    if (fabs(it->x - midx) < mindist) {
      t[size++] = it;
    }
  }
  for (int i = 0; i < size; i++) {
    for (int j = i + 1; j < size; j++) {
      point a(*t[i]), b(*t[j]);
      if (b.y - a.y >= mindist) {
        break;
      }
      double dist = norm(point(a.x - b.x, a.y - b.y));
      if (mindist > dist) {
        mindist = dist;
        if (res) {
          *res = std::make_pair(a, b);
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
  vector<point> v;
  v.push_back(point(2, 3));
  v.push_back(point(12, 30));
  v.push_back(point(40, 50));
  v.push_back(point(5, 1));
  v.push_back(point(12, 10));
  v.push_back(point(3, 4));
  pair<point, point> res;
  assert(EQ(closest_pair(v.begin(), v.end(), &res), sqrt(2)));
  assert(res.first == point(2, 3));
  assert(res.second == point(3, 4));
  return 0;
}
