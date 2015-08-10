/*

5.3.3 - 2D Convex Hull

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

const double eps = 1e-9;

#define LE(a, b) ((a) <= (b) + eps)       /* less than or equal to */
#define GE(a, b) ((a) >= (b) - eps)       /* greater than or equal to */

//magnitude of the 3D cross product with Z component implicitly equal to 0
//the answer assumes the origin (0, 0) is instead shifted to point o.
//this is equal to 2x the signed area of the triangle from these 3 points.
double cross(const point & o, const point & a, const point & b) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

//convex hull from a range [lo, hi) of points
//monotone chain in O(n log n) to find hull points in CW order
//notes: the range of input points will be sorted lexicographically
//       change GE comparisons to LE to produce hull points in CCW order
template<class It> std::vector<point> convex_hull(It lo, It hi) {
  int k = 0;
  std::vector<point> res(2 * (int)(hi - lo));
  std::sort(lo, hi); //compare by x, then by y if x-values are equal
  for (It it = lo; it != hi; ++it) {
    while (k >= 2 && GE(cross(res[k - 2], res[k - 1], *it), 0)) k--;
    res[k++] = *it;
  }
  int t = k + 1;
  for (It it = hi - 2; it != lo - 1; --it) {
    while (k >= t && GE(cross(res[k - 2], res[k - 1], *it), 0)) k--;
    res[k++] = *it;
  }
  res.resize(k - 1);
  return res;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;
#define pt point

int main() {
  //irregular pentagon with only (1, 2) not on the convex hull
  pt pts[] = {pt(1, 3), pt(1, 2), pt(2, 1), pt(0, 0), pt(-1, 3)};
  vector<pt> v(pts, pts + 5);
  std::random_shuffle(v.begin(), v.end());
  vector<pt> h = convex_hull(v.begin(), v.end());
  cout << "hull points:";
  for (int i = 0; i < (int)h.size(); i++)
    cout << " (" << h[i].x << "," << h[i].y << ")";
  cout << "\n";
  return 0;
}
