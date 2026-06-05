/*

Given a list of line segments in two dimensions, determine whether any pair of segments intersect
using a sweep line algorithm.

`Segment<Pt>` is templated on the point type, so endpoints may be integer (`PointI`) or
floating-point (`Point`/`PointD`). The cross-product sign tests in `seg_intersection` are exact for
integer endpoints, so intersection detection is exact. The sweep-line y-ordering compares
interpolated y-values by cross-multiplication, avoiding division.

- `find_intersection(lo, hi, &res1, &res2)` returns whether any pair of segments intersect given a
  range `[lo, hi)` of segments, where `lo` and `hi` are random-access iterators. If an intersection
  is found, then one such pair of segments will be stored into pointers `res1` and `res2`. Touching
  behavior is controlled by `TOUCH_IS_INTERSECT`.

Time Complexity:
- O(n log n) per call to `find_intersection(lo, hi, &res1, &res2)`, where $n$ is the distance
  between `lo` and `hi`.

Space Complexity:
- O(n) auxiliary heap space for `find_intersection(lo, hi, &res1, &res2)`, where $n$ is the distance
  between `lo` and `hi`.

*/

#include <algorithm>
#include <cassert>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

template<class Pt>
bool point_on_segment(const Pt &p, const Pt &a, const Pt &b) {
  return (p.x - a.x) * (b.y - a.y) == (p.y - a.y) * (b.x - a.x) && std::min(a.x, b.x) <= p.x &&
         p.x <= std::max(a.x, b.x) && std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
}

// Simplified detection-only version of `seg_intersection` from 7.2.3, with exact arithmetic for
// integer-coordinate Pt.
template<class Pt>
int seg_intersection(const Pt &a, const Pt &b, const Pt &c, const Pt &d) {
  static const bool TOUCH_IS_INTERSECT = true;
  auto ab_x = b.x - a.x, ab_y = b.y - a.y;
  auto ac_x = c.x - a.x, ac_y = c.y - a.y;
  auto cd_x = d.x - c.x, cd_y = d.y - c.y;
  auto ab2 = ab_x * ab_x + ab_y * ab_y;
  auto cd2 = cd_x * cd_x + cd_y * cd_y;
  if (ab2 == 0) {
    return (TOUCH_IS_INTERSECT && point_on_segment(a, c, d)) ? 0 : -1;
  }
  if (cd2 == 0) {
    return (TOUCH_IS_INTERSECT && point_on_segment(c, a, b)) ? 0 : -1;
  }
  auto c1 = ab_x * cd_y - ab_y * cd_x;
  auto c2 = ac_x * ab_y - ac_y * ab_x;
  if (c1 == 0 && c2 == 0) {
    Pt res1 = std::max(std::min(a, b), std::min(c, d));
    Pt res2 = std::min(std::max(a, b), std::max(c, d));
    bool overlap = TOUCH_IS_INTERSECT ? !(res2 < res1) : (res1 < res2);
    if (overlap) {
      return (res1 == res2) ? 0 : 1;
    }
    return -1;
  }
  if (c1 == 0) {
    return -1;
  }
  auto t_num = ac_x * cd_y - ac_y * cd_x;
  bool c1_pos = c1 > 0;
  bool t_between_01 =
      c1_pos ? (TOUCH_IS_INTERSECT ? (0 <= t_num && t_num <= c1) : (0 < t_num && t_num < c1))
             : (TOUCH_IS_INTERSECT ? (t_num <= 0 && c1 <= t_num) : (t_num < 0 && c1 < t_num));
  bool u_between_01 = c1_pos ? (TOUCH_IS_INTERSECT ? (0 <= c2 && c2 <= c1) : (0 < c2 && c2 < c1))
                             : (TOUCH_IS_INTERSECT ? (c2 <= 0 && c1 <= c2) : (c2 < 0 && c1 < c2));
  return (t_between_01 && u_between_01) ? 0 : -1;
}

// The point type Pt must support operator< (used to canonicalize endpoint order).
template<class Pt>
struct Segment {
  Pt p, q;

  Segment() {}
  Segment(const Pt &p, const Pt &q) : p(std::min(p, q)), q(std::max(p, q)) {}
};

template<class Pt>
bool intersect(const Segment<Pt> &s1, const Segment<Pt> &s2) {
  return seg_intersection(s1.p, s1.q, s2.p, s2.q) >= 0;
}

// Seg is deduced from the output pointers (e.g. Segment<PointI>* or Segment<PointD>*).
template<class It, class Seg>
bool find_intersection(It lo, It hi, Seg *res1, Seg *res2) {
  using Pt = std::decay_t<decltype(lo->p)>;
  struct Event {
    Pt p;
    int type;
    It seg;
  };

  std::vector<Event> e;
  for (It it = lo; it != hi; ++it) {
    if (it->p > it->q) {
      std::swap(it->p, it->q);
    }
    e.push_back(Event{it->p, 1, it});
    e.push_back(Event{it->q, -1, it});
  }
  std::sort(e.begin(), e.end(), [](const Event &a, const Event &b) {
    if (a.p.x != b.p.x) {
      return a.p.x < b.p.x;
    }
    if (a.type != b.type) {
      return b.type < a.type;
    }
    return a.p.y < b.p.y;
  });

  auto ycmp = [](const auto &a, const auto &b, auto x) {
    // Compare y-values at sweep coordinate x without division:
    // y = (p.y*dx + dy*(x - p.x)) / dx. Vertical segments use their lower endpoint.
    auto adx = a.q.x - a.p.x, bdx = b.q.x - b.p.x;
    auto ay = a.p.y * adx + (a.q.y - a.p.y) * (x - a.p.x);
    auto by = b.p.y * bdx + (b.q.y - b.p.y) * (x - b.p.x);
    if (adx == 0) {
      ay = a.p.y;
      adx = 1;
    }
    if (bdx == 0) {
      by = b.p.y;
      bdx = 1;
    }
    auto lhs = ay * bdx, rhs = by * adx;
    return (lhs < rhs) ? -1 : ((rhs < lhs) ? 1 : 0);
  };

  auto cmp = [lo, ycmp](It a, It b) {
    if (a == b) {
      return false;
    }
    auto x = std::max(a->p.x, b->p.x);
    int order = ycmp(*a, *b, x);
    return (order == 0) ? (a - lo < b - lo) : (order < 0);
  };
  using active_set = std::set<It, decltype(cmp)>;
  active_set s(cmp);
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

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
  bool operator==(const PointI &p) const { return x == p.x && y == p.y; }
  bool operator!=(const PointI &p) const { return !(*this == p); }
  bool operator<(const PointI &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const PointI &p) const { return p < *this; }
};

int main() {
  {  // Double-coordinate segments.
    vector<Segment<Point>> v{
        Segment<Point>(Point(0, 0), Point(2, 2)), Segment<Point>(Point(3, 0), Point(0, -1)),
        Segment<Point>(Point(0, 2), Point(2, -2)), Segment<Point>(Point(0, 3), Point(9, 0))
    };
    Segment<Point> res1, res2;
    assert(find_intersection(v.begin(), v.end(), &res1, &res2));
    assert(res1.p == Point(0, 0) && res1.q == Point(2, 2));
    assert(res2.p == Point(0, 2) && res2.q == Point(2, -2));
  }
  {  // Integer-coordinate segments: detection is exact.
    vector<Segment<PointI>> v{
        Segment<PointI>({0, 0}, {2, 2}), Segment<PointI>({3, 0}, {0, -1}),
        Segment<PointI>({0, 2}, {2, -2}), Segment<PointI>({0, 3}, {9, 0})
    };
    Segment<PointI> res1, res2;
    assert(find_intersection(v.begin(), v.end(), &res1, &res2));

    vector<Segment<PointI>> disjoint{
        Segment<PointI>({0, 0}, {1, 0}), Segment<PointI>({0, 5}, {1, 5})
    };
    assert(!find_intersection(disjoint.begin(), disjoint.end(), &res1, &res2));
  }
  return 0;
}
