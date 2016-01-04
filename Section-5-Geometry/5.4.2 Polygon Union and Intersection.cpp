/*

Given two ranges of points respectively denoting the vertices of
two polygons, determine the intersection area of those polygons.
Using this, we can easily calculate their union with the forumla:
  union_area(A, B) = area(A) + area(B) - intersection_area(A, B)

Time Complexity: O(n^2 log n), where n is the total number of vertices.

*/

#include <algorithm> /* std::sort() */
#include <cmath>     /* fabs(), sqrt() */
#include <set>
#include <utility>   /* std::pair */
#include <vector>

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define LT(a, b) ((a) < (b) - eps)        /* less than */
#define LE(a, b) ((a) <= (b) + eps)       /* less than or equal to */

typedef std::pair<double, double> point;
#define x first
#define y second

inline int sgn(const double & x) {
  return (0.0 < x) - (x < 0.0);
}

//Line and line segment intersection (see their own sections)

int line_intersection(const point & p1, const point & p2,
                      const point & p3, const point & p4, point * p = 0) {
  double a1 = p2.y - p1.y, b1 = p1.x - p2.x;
  double c1 = -(p1.x * p2.y - p2.x * p1.y);
  double a2 = p4.y - p3.y, b2 = p3.x - p4.x;
  double c2 = -(p3.x * p4.y - p4.x * p3.y);
  double x = -(c1 * b2 - c2 * b1), y = -(a1 * c2 - a2 * c1);
  double det = a1 * b2 - a2 * b1;
  if (EQ(det, 0))
    return (EQ(x, 0) && EQ(y, 0)) ? 1 : -1;
  if (p != 0) *p = point(x / det, y / det);
  return 0;
}

double norm(const point & a) { return a.x * a.x + a.y * a.y; }
double dot(const point & a, const point & b) { return a.x * b.x + a.y * b.y; }
double cross(const point & a, const point & b) { return a.x * b.y - a.y * b.x; }

const bool TOUCH_IS_INTERSECT = true;

bool contain(const double & l, const double & m, const double & h) {
  if (TOUCH_IS_INTERSECT) return LE(l, m) && LE(m, h);
  return LT(l, m) && LT(m, h);
}

bool overlap(const double & l1, const double & h1,
             const double & l2, const double & h2) {
  if (TOUCH_IS_INTERSECT) return LE(l1, h2) && LE(l2, h1);
  return LT(l1, h2) && LT(l2, h1);
}

int seg_intersection(const point & a, const point & b,
                     const point & c, const point & d,
                     point * p = 0, point * q = 0) {
  point ab(b.x - a.x, b.y - a.y);
  point ac(c.x - a.x, c.y - a.y);
  point cd(d.x - c.x, d.y - c.y);
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) { //collinear
    double t0 = dot(ac, ab) / norm(ab);
    double t1 = t0 + dot(cd, ab) / norm(ab);
    if (overlap(std::min(t0, t1), std::max(t0, t1), 0, 1)) {
      point res1 = std::max(std::min(a, b), std::min(c, d));
      point res2 = std::min(std::max(a, b), std::max(c, d));
      if (res1 == res2) {
        if (p != 0) *p = res1;
        return 0; //collinear, meeting at an endpoint
      }
      if (p != 0 && q != 0) *p = res1, *q = res2;
      return 1; //collinear and overlapping
    } else {
      return -1; //collinear and disjoint
    }
  }
  if (EQ(c1, 0)) return -1; //parallel and disjoint
  double t = cross(ac, cd) / c1, u = c2 / c1;
  if (contain(0, t, 1) && contain(0, u, 1)) {
    if (p != 0) *p = point(a.x + t * ab.x, a.y + t * ab.y);
    return 0; //non-parallel with one intersection
  }
  return -1; //non-parallel with no intersections
}

struct event {
  double y;
  int mask_delta;

  event(double y = 0, int mask_delta = 0) {
    this->y = y;
    this->mask_delta = mask_delta;
  }

  bool operator < (const event & e) const {
    if (y != e.y) return y < e.y;
    return mask_delta < e.mask_delta;
  }
};

template<class It>
double intersection_area(It lo1, It hi1, It lo2, It hi2) {
  It plo[2] = {lo1, lo2}, phi[] = {hi1, hi2};
  std::set<double> xs;
  for (It i1 = lo1; i1 != hi1; ++i1) xs.insert(i1->x);
  for (It i2 = lo2; i2 != hi2; ++i2) xs.insert(i2->x);
  for (It i1 = lo1, j1 = hi1 - 1; i1 != hi1; j1 = i1++) {
    for (It i2 = lo2, j2 = hi2 - 1; i2 != hi2; j2 = i2++) {
      point p;
      if (seg_intersection(*i1, *j1, *i2, *j2, &p) == 0)
        xs.insert(p.x);
    }
  }
  std::vector<double> xsa(xs.begin(), xs.end());
  double res = 0;
  for (int k = 0; k < (int)xsa.size() - 1; k++) {
    double x = (xsa[k] + xsa[k + 1]) / 2;
    point sweep0(x, 0), sweep1(x, 1);
    std::vector<event> events;
    for (int poly = 0; poly < 2; poly++) {
      It lo = plo[poly], hi = phi[poly];
      double area = 0;
      for (It i = lo, j = hi - 1; i != hi; j = i++)
        area += (j->x - i->x) * (j->y + i->y);
      for (It j = lo, i = hi - 1; j != hi; i = j++) {
        point p;
        if (line_intersection(*j, *i, sweep0, sweep1, &p) == 0) {
          double y = p.y, x0 = i->x, x1 = j->x;
          if (x0 < x && x1 > x) {
            events.push_back(event(y,  sgn(area) * (1 << poly)));
          } else if (x0 > x && x1 < x) {
            events.push_back(event(y, -sgn(area) * (1 << poly)));
          }
        }
      }
    }
    std::sort(events.begin(), events.end());
    double a = 0.0;
    int mask = 0;
    for (int j = 0; j < (int)events.size(); j++) {
      if (mask == 3)
        a += events[j].y - events[j - 1].y;
      mask += events[j].mask_delta;
    }
    res += a * (xsa[k + 1] - xsa[k]);
  }
  return res;
}

template<class It> double polygon_area(It lo, It hi) {
  if (lo == hi) return 0;
  double area = 0;
  if (*lo != *--hi)
    area += (lo->x - hi->x) * (lo->y + hi->y);
  for (It i = hi, j = hi - 1; i != lo; --i, --j)
    area += (i->x - j->x) * (i->y + j->y);
  return fabs(area / 2.0);
}

template<class It>
double union_area(It lo1, It hi1, It lo2, It hi2) {
  return polygon_area(lo1, hi1) + polygon_area(lo2, hi2) -
         intersection_area(lo1, hi1, lo2, hi2);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<point> p1, p2;

  //irregular pentagon with area 1.5 triangle in quadrant 2
  p1.push_back(point(1, 3));
  p1.push_back(point(1, 2));
  p1.push_back(point(2, 1));
  p1.push_back(point(0, 0));
  p1.push_back(point(-1, 3));
  //a big square in quadrant 2
  p2.push_back(point(0, 0));
  p2.push_back(point(0, 3));
  p2.push_back(point(-3, 3));
  p2.push_back(point(-3, 0));

  assert(EQ(1.5, intersection_area(p1.begin(), p1.end(),
                                   p2.begin(), p2.end())));
  assert(EQ(12.5, union_area(p1.begin(), p1.end(),
                             p2.begin(), p2.end())));
  return 0;
}
