/*

Determines the convex hull from a range of points, that is, the
smallest convex polygon (a polygon such that every line which
crosses through it will only cross through it once) that contains
all of the points. This function uses the monotone chain algorithm
to compute the upper and lower hulls separately.

Returns: a vector of the convex hull points in clockwise order.
Complexity: O(n log n) on the number of points given

Notes: To yield the hull points in counterclockwise order,
       replace every usage of GE() in the function with LE().
       To have the first point on the hull repeated as the last,
       replace the last line of the function to res.resize(k);

*/

#include <algorithm> /* std::sort() */
#include <cmath>     /* fabs() */
#include <utility>   /* std::pair */
#include <vector>

typedef std::pair<double, double> point;
#define x first
#define y second

//change < 0 comparisons to > 0 to produce hull points in CCW order
double cw(const point & o, const point & a, const point & b) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x) < 0;
}

//convex hull from a range [lo, hi) of points
//monotone chain in O(n log n) to find hull points in CW order
//notes: the range of input points will be sorted lexicographically
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

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  //irregular pentagon with only (1, 2) not on the convex hull
  vector<point> v;
  v.push_back(point(1, 3));
  v.push_back(point(1, 2));
  v.push_back(point(2, 1));
  v.push_back(point(0, 0));
  v.push_back(point(-1, 3));
  std::random_shuffle(v.begin(), v.end());
  vector<point> h = convex_hull(v.begin(), v.end());
  cout << "hull points:";
  for (int i = 0; i < (int)h.size(); i++)
    cout << " (" << h[i].x << "," << h[i].y << ")";
  cout << "\n";
  return 0;
}
