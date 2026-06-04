/*

Given a list of line segments in two dimensions, determine whether any pair of segments intersect
using a sweep line algorithm.

- `find_intersection(lo, hi, &res1, &res2)` returns whether any pair of segments intersect given a
  range `[lo, hi)` of segments, where `lo` and `hi` are random-access iterators. If an intersection
  is found, then one such pair of segments will be stored into pointers `res1` and `res2`. If some
  segments are barely touching (close within `EPS`), then the result will depend on the setting of
  `TOUCH_IS_INTERSECT`.

Time Complexity:
- O(n log n) per call to `find_intersection(lo, hi, &res1, &res2)`, where $n$ is the distance
  between `lo` and `hi`.

Space Complexity:
- O(n) auxiliary heap space for `find_intersection(lo, hi, &res1, &res2)`, where $n$ is the distance
  between `lo` and `hi`.

*/

#include <algorithm>
#include <cassert>
#include <cmath>
#include <set>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

using point = std::pair<double, double>;
#define x first
#define y second

// clang-format off
double sqnorm(const point &a) { return a.x * a.x + a.y * a.y;}
double norm(const point &a) { return sqrt(sqnorm(a)); }
double dot(const point &a, const point &b) { return a.x * b.x + a.y * b.y; }
// clang-format on

double cross(const point &a, const point &b, const point &o = point(0, 0)) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

bool point_on_segment(const point &p, const point &a, const point &b) {
  return EQ(cross(p, b, a), 0) && LE(std::min(a.x, b.x), p.x) && LE(p.x, std::max(a.x, b.x)) &&
         LE(std::min(a.y, b.y), p.y) && LE(p.y, std::max(a.y, b.y));
}

int seg_intersection(
    const point &a, const point &b, const point &c, const point &d, point *p = nullptr,
    point *q = nullptr
) {
  static const bool TOUCH_IS_INTERSECT = true;
  point ab(b.x - a.x, b.y - a.y);
  point ac(c.x - a.x, c.y - a.y);
  point cd(d.x - c.x, d.y - c.y);
  if (EQ(sqnorm(ab), 0)) {
    if (TOUCH_IS_INTERSECT && point_on_segment(a, c, d)) {
      if (p != nullptr) *p = a;
      return 0;
    }
    return -1;
  }
  if (EQ(sqnorm(cd), 0)) {
    if (TOUCH_IS_INTERSECT && point_on_segment(c, a, b)) {
      if (p != nullptr) *p = c;
      return 0;
    }
    return -1;
  }
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    double t0 = dot(ac, ab) / sqnorm(ab);
    double t1 = t0 + dot(cd, ab) / sqnorm(ab);
    double mint = std::min(t0, t1), maxt = std::max(t0, t1);
    bool overlap = TOUCH_IS_INTERSECT ? (LE(mint, 1) && LE(0, maxt)) : (LT(mint, 1) && LT(0, maxt));
    if (overlap) {
      point res1 = std::max(std::min(a, b), std::min(c, d));
      point res2 = std::min(std::max(a, b), std::max(c, d));
      if (res1 == res2) {
        if (p != nullptr) {
          *p = res1;
        }
        return 0;  // Collinear and meeting at an endpoint.
      }
      if (p != nullptr && q != nullptr) {
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
  double t = cross(ac, cd) / c1, u = c2 / c1;
  bool t_between_01 = TOUCH_IS_INTERSECT ? (LE(0, t) && LE(t, 1)) : (LT(0, t) && LT(t, 1));
  bool u_between_01 = TOUCH_IS_INTERSECT ? (LE(0, u) && LE(u, 1)) : (LT(0, u) && LT(u, 1));
  if (t_between_01 && u_between_01) {
    if (p != nullptr) {
      *p = point(a.x + t * ab.x, a.y + t * ab.y);
    }
    return 0;  // Non-parallel with one intersection.
  }
  return -1;  // Non-parallel with no intersections.
}

struct Segment {
  point p, q;

  Segment() {}
  Segment(const point &p, const point &q) : p(min(p, q)), q(max(p, q)) {}
};

double get_y(const Segment &s, double x) {
  if (EQ(s.p.x, s.q.x)) {
    return s.p.y;
  }
  return s.p.y + (s.q.y - s.p.y) * (x - s.p.x) / (s.q.x - s.p.x);
}

template<class It>
struct SegmentComparator {
  It lo;

  SegmentComparator(It lo) : lo(lo) {}

  bool operator()(It a, It b) const {
    if (a == b) {
      return false;
    }
    double x = std::max(a->p.x, b->p.x);
    double ay = get_y(*a, x), by = get_y(*b, x);
    return (ay == by) ? (a - lo < b - lo) : (ay < by);
  }
};

template<class SegIt>
struct Event {
  point p;
  int type;
  SegIt seg;

  Event() {}
  Event(const point &p, int type, SegIt seg) : p(p), type(type), seg(seg) {}

  bool operator<(const Event &rhs) const {
    if (p.x != rhs.p.x) {
      return p.x < rhs.p.x;
    }
    if (type != rhs.type) {
      return rhs.type < type;
    }
    return p.y < rhs.p.y;
  }
};

bool intersect(const Segment &s1, const Segment &s2) {
  return seg_intersection(s1.p, s1.q, s2.p, s2.q) >= 0;
}

template<class It>
bool find_intersection(It lo, It hi, Segment *res1, Segment *res2) {
  std::vector<Event<It>> e;
  for (It it = lo; it != hi; ++it) {
    if (it->p > it->q) {
      std::swap(it->p, it->q);
    }
    e.emplace_back(it->p, 1, it);
    e.emplace_back(it->q, -1, it);
  }
  std::sort(e.begin(), e.end());
  using active_set = std::set<It, SegmentComparator<It>>;
  active_set s((SegmentComparator<It>(lo)));
  std::vector<typename active_set::iterator> position(hi - lo);
  for (const auto &ev : e) {
    It seg = ev.seg;
    if (ev.type == 1) {
      auto it = s.insert(seg).first;
      position[seg - lo] = it;
      auto next = it;
      if (++next != s.end() && intersect(**next, *seg)) {
        *res1 = **next;
        *res2 = *seg;
        return true;
      }
      if (it != s.begin() && intersect(**--it, *seg)) {
        *res1 = **it;
        *res2 = *seg;
        return true;
      }
    } else {
      auto it = position[seg - lo];
      auto next = it;
      if (++next != s.end() && it != s.begin()) {
        auto prev = it;
        if (intersect(**next, **--prev)) {
          *res1 = **next;
          *res2 = **prev;
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
  vector<Segment> v;
  v.emplace_back(point(0, 0), point(2, 2));
  v.emplace_back(point(3, 0), point(0, -1));
  v.emplace_back(point(0, 2), point(2, -2));
  v.emplace_back(point(0, 3), point(9, 0));
  Segment res1, res2;
  assert(find_intersection(v.begin(), v.end(), &res1, &res2));
  assert(res1.p == point(0, 0) && res1.q == point(2, 2));
  assert(res2.p == point(0, 2) && res2.q == point(2, -2));
  return 0;
}
