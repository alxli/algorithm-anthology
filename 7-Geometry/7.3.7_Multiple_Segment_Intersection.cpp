/*

Given a list of line segments in two dimensions, determine whether any pair of segments intersect
using a sweep line algorithm. Endpoint events are processed from left to right while an ordered set
maintains the segments currently crossing the sweep line, sorted by $y$; only segments that become
adjacent in this set need to be tested, since any leftmost intersection must involve a pair that is
adjacent just before it occurs. The input type `Segment<Pt>` is templated on the point type, so
endpoints may be integer (`PointI`) or floating-point (`Point`/`PointD`), but must support
`operator<` which orders points lexicographically. The cross-product sign tests in
`seg_intersection` are exact for integer endpoints, so intersection detection is exact.

- `find_intersection(lo, hi, &res1, &res2, TOUCH_IS_INTERSECT)` returns whether any pair of segments
  intersect given a range [`lo`, `hi`) of segments, where `lo` and `hi` are random-access iterators.
  If an intersection is found, then one such pair of segments will be stored into pointers `res1`
  and `res2`. The `TOUCH_IS_INTERSECT` flag (default `true`) controls whether segments that meet
  only at an endpoint count as intersecting; set it to `false` to report proper crossings only,
  as when testing a polygon for self-intersection among edges that legitimately share endpoints.

Overflow warning: `seg_intersection` forms the usual quadratic cross products, but the $y$-ordering
cross-multiplication (`ay * bdx`) is cubic in the coordinate magnitude. With 32-bit `int` endpoints
this overflows once coordinates reach the low thousands, so use a 64-bit (`int64_t`) coordinate
type for any non-trivial integer inputs.

Time Complexity:
- O(n log n) per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) auxiliary heap space for `find_intersection(lo, hi, &res1, &res2)`, where $n$ is the distance
  between `lo` and `hi`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

template<typename Pt>
bool point_on_segment(const Pt &p, const Pt &a, const Pt &b) {
  return (p.x - a.x) * (b.y - a.y) == (p.y - a.y) * (b.x - a.x) && std::min(a.x, b.x) <= p.x &&
         p.x <= std::max(a.x, b.x) && std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
}

// Simplified detection-only version of `seg_intersection()` from 7.2.3 (exact for integral Pt).
template<typename Pt>
int seg_intersection(
    const Pt &a, const Pt &b, const Pt &c, const Pt &d, const bool TOUCH_IS_INTERSECT = true
) {
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
             : (TOUCH_IS_INTERSECT ? (c1 <= t_num && t_num <= 0) : (c1 < t_num && t_num < 0));
  bool u_between_01 = c1_pos ? (TOUCH_IS_INTERSECT ? (0 <= c2 && c2 <= c1) : (0 < c2 && c2 < c1))
                             : (TOUCH_IS_INTERSECT ? (c1 <= c2 && c2 <= 0) : (c1 < c2 && c2 < 0));
  return (t_between_01 && u_between_01) ? 0 : -1;
}

// The point type Pt must support operator< (used to canonicalize endpoint order).
template<typename Pt>
struct Segment {
  Pt p, q;
  Segment() {}
  Segment(const Pt &p, const Pt &q) : p(std::min(p, q)), q(std::max(p, q)) {}
};

template<typename Pt>
bool intersects(
    const Segment<Pt> &s1, const Segment<Pt> &s2, const bool TOUCH_IS_INTERSECT = true
) {
  return seg_intersection(s1.p, s1.q, s2.p, s2.q, TOUCH_IS_INTERSECT) >= 0;
}

// Seg is deduced from the output pointers (e.g. Segment<PointI>* or Segment<PointD>*).
template<typename It, typename Seg>
bool find_intersection(It lo, It hi, Seg *res1, Seg *res2, const bool TOUCH_IS_INTERSECT = true) {
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
  // Compare y-values at sweep coordinate x without division: y = (p.y*dx + dy*(x - p.x)) / dx.
  // Vertical segments use their lower endpoint
  auto ycmp = [](const auto &a, const auto &b, auto x) {
    // Overflow risk for integer Pt: the final cross-multiply is ~O(max_coord^3); use 64-bit
    // coordinates for non-trivial integer inputs.
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
      if (++next != s.end() && intersects(**next, *seg, TOUCH_IS_INTERSECT)) {
        *res1 = **next;
        *res2 = *seg;
        return true;
      }
      if (it != s.begin() && intersects(**--it, *seg, TOUCH_IS_INTERSECT)) {
        *res1 = **it;
        *res2 = *seg;
        return true;
      }
    } else {
      auto it = position[seg - lo];
      auto next = it;
      if (++next != s.end() && it != s.begin()) {
        auto prev = it;
        if (intersects(**next, **--prev, TOUCH_IS_INTERSECT)) {
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
  {  // TOUCH_IS_INTERSECT = false reports proper crossings only, ignoring shared endpoints.
    vector<Segment<PointI>> shared{
        Segment<PointI>({0, 0}, {2, 2}), Segment<PointI>({2, 2}, {4, 0})
    };
    Segment<PointI> res1, res2;
    assert(find_intersection(shared.begin(), shared.end(), &res1, &res2));
    assert(!find_intersection(shared.begin(), shared.end(), &res1, &res2, false));
  }
  return 0;
}
