/*

Given a range of segments on the Cartesian plane, identify one
pair of segments which intersect each other. This is done using
a sweep line algorithm.

Time Complexity: O(n log n) where n is the number of segments.

*/

#include <algorithm> /* std::min(), std::max(), std::sort() */
#include <cmath>     /* fabs() */
#include <set>
#include <utility>   /* std::pair */

typedef std::pair<double, double> point;
#define x first
#define y second

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps)  /* equal to */
#define LT(a, b) ((a) < (b) - eps)         /* less than */
#define LE(a, b) ((a) <= (b) + eps)        /* less than or equal to */

double norm(const point & a) { return a.x * a.x + a.y * a.y; }
double dot(const point & a, const point & b) { return a.x * b.x + a.y * b.y; }
double cross(const point & a, const point & b) { return a.x * b.y - a.y * b.x; }
double cross(const point & o, const point & a, const point & b) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

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
                     const point & c, const point & d) {
  point ab(b.x - a.x, b.y - a.y);
  point ac(c.x - a.x, c.y - a.y);
  point cd(d.x - c.x, d.y - c.y);
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) {
    double t0 = dot(ac, ab) / norm(ab);
    double t1 = t0 + dot(cd, ab) / norm(ab);
    if (overlap(std::min(t0, t1), std::max(t0, t1), 0, 1)) {
      point res1 = std::max(std::min(a, b), std::min(c, d));
      point res2 = std::min(std::max(a, b), std::max(c, d));
      return (res1 == res2) ? 0 : 1;
    }
    return -1;
  }
  if (EQ(c1, 0)) return -1;
  double t = cross(ac, cd) / c1, u = c2 / c1;
  if (contain(0, t, 1) && contain(0, u, 1)) return 0;
  return -1;
}

struct segment {
  point p, q;

  segment() {}
  segment(const point & p, const point & q) {
    if (p < q) {
      this->p = p;
      this->q = q;
    } else {
      this->p = q;
      this->q = p;
    }
  }

  bool operator < (const segment & rhs) const {
    if (p.x < rhs.p.x) {
      double c = cross(p, q, rhs.p);
      if (c != 0) return c > 0;
    } else if (p.x > rhs.p.x) {
      double c = cross(rhs.p, rhs.q, q);
      if (c != 0) return c < 0;
    }
    return p.y < rhs.p.y;
  }
};

template<class SegIt> struct event {
  point p;
  int type;
  SegIt seg;

  event() {}
  event(const point & p, const int type, SegIt seg) {
    this->p = p;
    this->type = type;
    this->seg = seg;
  }

  bool operator < (const event & rhs) const {
    if (p.x != rhs.p.x) return p.x < rhs.p.x;
    if (type != rhs.type) return type > rhs.type;
    return p.y < rhs.p.y;
  }
};

bool intersect(const segment & s1, const segment & s2) {
  return seg_intersection(s1.p, s1.q, s2.p, s2.q) >= 0;
}

//returns whether any pair of segments in the range [lo, hi) intersect
//if the result is true, one such intersection pair will be stored
//into values pointed to by res1 and res2.
template<class It>
bool find_intersection(It lo, It hi, segment * res1, segment * res2) {
  int cnt = 0;
  event<It> e[2 * (hi - lo)];
  for (It it = lo; it != hi; ++it) {
    if (it->p > it->q) std::swap(it->p, it->q);
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
        *res1 = *it; *res2 = *seg;
        return true;
      }
      if (it != s.begin() && intersect(*--it, *seg)) {
        *res1 = *it; *res2 = *seg;
        return true;
      }
      s.insert(*seg);
    } else {
      it = s.lower_bound(*seg);
      next = prev = it;
      prev = it;
      if (it != s.begin() && it != --s.end()) {
        ++next;
        --prev;
        if (intersect(*next, *prev)) {
          *res1 = *next; *res2 = *prev;
          return true;
        }
      }
      s.erase(it);
    }
  }
  return false;
}

/*** Example Usage ***/

#include <iostream>
#include <vector>
using namespace std;

void print(const segment & s) {
  cout << "(" << s.p.x << "," << s.p.y << ")<->";
  cout << "(" << s.q.x << "," << s.q.y << ")\n";
}

int main() {
  vector<segment> v;
  v.push_back(segment(point(0, 0), point(2, 2)));
  v.push_back(segment(point(3, 0), point(0, -1)));
  v.push_back(segment(point(0, 2), point(2, -2)));
  v.push_back(segment(point(0, 3), point(9, 0)));
  segment res1, res2;
  bool res = find_intersection(v.begin(), v.end(), &res1, &res2);
  if (res) {
    print(res1);
    print(res2);
  } else {
    cout << "No intersections.\n";
  }
  return 0;
}
