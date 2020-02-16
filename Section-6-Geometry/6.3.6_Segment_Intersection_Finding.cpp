/*

Given a list of line segments in two dimensions, determine whether any pair of
segments intersect using a sweep line algorithm.

- find_intersection(lo, hi, &res1, &res2) returns whether any pair of segments
  intersect given a range [lo, hi) of segments, where lo and hi are
  random-access iterators. If there an intersection is found, then one such pair
  of segments will be stored into pointers res1 and res2. If some segments are
  barely touching (close within EPS), then the result will depend on the setting
  of TOUCH_IS_INTERSECT.

Time Complexity:
- O(n log n) per call to find_intersection(lo, hi, &res1, &res2), where n is
  the distance between lo and hi.

Space Complexity:
- O(n) auxiliary heap space for find_intersection(lo, hi, &res1, &res2), where n
  is the distance between lo and hi.

*/

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double sqnorm(const point &a) { return a.x*a.x + a.y*a.y; }
double norm(const point &a) { return sqrt(sqnorm(a)); }
double dot(const point &a, const point &b) { return a.x*b.x + a.y*b.y; }
double cross(const point &a, const point &b, const point &o = point(0, 0)) {
  return (a.x - o.x)*(b.y - o.y) - (a.y - o.y)*(b.x - o.x);
}

int seg_intersection(const point &a, const point &b, const point &c,
                     const point &d, point *p = NULL, point *q = NULL) {
  static const bool TOUCH_IS_INTERSECT = true;
  point ab(b.x - a.x, b.y - a.y);
  point ac(c.x - a.x, c.y - a.y);
  point cd(d.x - c.x, d.y - c.y);
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    double t0 = dot(ac, ab) / sqnorm(ab);
    double t1 = t0 + dot(cd, ab) / sqnorm(ab);
    double mint = std::min(t0, t1), maxt = std::max(t0, t1);
    bool overlap = TOUCH_IS_INTERSECT ? (LE(mint, 1) && LE(0, maxt))
                                      : (LT(mint, 1) && LT(0, maxt));
    if (overlap) {
      point res1 = std::max(std::min(a, b), std::min(c, d));
      point res2 = std::min(std::max(a, b), std::max(c, d));
      if (res1 == res2) {
        if (p != NULL) {
          *p = res1;
        }
        return 0;  // Collinear and meeting at an endpoint.
      }
      if (p != NULL && q != NULL) {
        *p = res1;
        *q = res2;
      }
      return 1;  // Collinear and overlapping.
    } else {
      return -1;  // Collinear and disjoint.
    }
  }
  if (EQ(c1, 0)) {
    return -1;  // Parallel and disjoint.
  }
  double t = cross(ac, cd)/c1, u = c2/c1;
  bool t_between_01 = TOUCH_IS_INTERSECT ? (LE(0, t) && LE(t, 1))
                                         : (LT(0, t) && LT(t, 1));
  bool u_between_01 = TOUCH_IS_INTERSECT ? (LE(0, u) && LE(u, 1))
                                         : (LT(0, u) && LT(u, 1));
  if (t_between_01 && u_between_01) {
    if (p != NULL) {
      *p = point(a.x + t*ab.x, a.y + t*ab.y);
    }
    return 0;  // Non-parallel with one intersection.
  }
  return -1;  // Non-parallel with no intersections.
}

struct segment {
  point p, q;

  segment() {}
  segment(const point &p, const point &q) : p(min(p, q)), q(max(p, q)) {}

  bool operator<(const segment &rhs) const {
    if (p.x < rhs.p.x) {
      double c = cross(q, rhs.p, p);
      if (c != 0) {
        return c > 0;
      }
    } else if (rhs.p.x < p.x) {
      double c = cross(rhs.q, q, rhs.p);
      if (c != 0) {
        return c < 0;
      }
    }
    return p.y < rhs.p.y;
  }
};

template<class SegIt>
struct event {
  point p;
  int type;
  SegIt seg;

  event() {}
  event(const point &p, int type, SegIt seg) : p(p), type(type), seg(seg) {}

  bool operator<(const event &rhs) const {
    if (p.x != rhs.p.x) {
      return p.x < rhs.p.x;
    }
    if (type != rhs.type) {
      return rhs.type < type;
    }
    return p.y < rhs.p.y;
  }
};

bool intersect(const segment &s1, const segment &s2) {
  return seg_intersection(s1.p, s1.q, s2.p, s2.q) >= 0;
}

template<class It>
bool find_intersection(It lo, It hi, segment *res1, segment *res2) {
  int cnt = 0;
  event<It> e[2*(int)(hi - lo)];
  for (It it = lo; it != hi; ++it) {
    if (it->p > it->q) {
      std::swap(it->p, it->q);
    }
    e[cnt++] = event<It>(it->p, 1, it);
    e[cnt++] = event<It>(it->q, -1, it);
  }
  std::sort(e, e + cnt);
  std::set<segment> s;
  std::set<segment>::iterator it, next, prev;
  for (int i = 0; i < cnt; i++) {
    It seg = e[i].seg;
    if (e[i].type == 1) {
      it = s.lower_bound(*seg);
      if (it != s.end() && intersect(*it, *seg)) {
        *res1 = *it;
        *res2 = *seg;
        return true;
      }
      if (it != s.begin() && intersect(*--it, *seg)) {
        *res1 = *it;
        *res2 = *seg;
        return true;
      }
      s.insert(*seg);
    } else {
      it = s.lower_bound(*seg);
      next = prev = it;
      prev = it;
      if (it != s.begin() && it != --s.end()) {
        if (intersect(*(++next), *(--prev))) {
          *res1 = *next;
          *res2 = *prev;
          return true;
        }
      }
      s.erase(it);
    }
  }
  return false;
}

/*** Example Usage ***/

#include <vector>
using namespace std;

int main() {
  vector<segment> v;
  v.push_back(segment(point(0, 0), point(2, 2)));
  v.push_back(segment(point(3, 0), point(0, -1)));
  v.push_back(segment(point(0, 2), point(2, -2)));
  v.push_back(segment(point(0, 3), point(9, 0)));
  segment res1, res2;
  assert(find_intersection(v.begin(), v.end(), &res1, &res2));
  assert(res1.p == point(0, 0) && res1.q == point(2, 2));
  assert(res2.p == point(0, 2) && res2.q == point(2, -2));
  return 0;
}
