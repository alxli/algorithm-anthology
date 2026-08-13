/*

Given a list of line segments in two dimensions, determine whether any pair of segments intersect
using a sweep line algorithm. Endpoint events are processed from left to right while an ordered set
maintains the segments currently crossing the sweep line, sorted by $y$; only segments that become
adjacent in this set need to be tested, since any leftmost intersection must involve a pair that is
adjacent just before it occurs. The input type `Segment<Pt>` is templated on the point type, so
endpoints may be integer (`PointI`) or floating-point (`Point` or `PointD`), but must support
`operator<` which orders points lexicographically. The cross-product sign tests in
`seg_intersection` are exact for integer endpoints, so intersection detection is exact.

This implementation counts endpoint contacts as intersections. See 7.2.3 for pairwise intersection
predicates when that distinction matters.

- `find_intersecting_pair(lo, hi, &res1, &res2)` returns whether any pair of segments intersect
  given a range $[`lo`, `hi`)$ of segments, where `lo` and `hi` are random-access iterators. If an
  intersection is found, then one such pair of segments is stored in `res1` and `res2`. Constructing
  a `Segment` places its lexicographically smaller endpoint first, as required by the sweep. If no
  intersection is found, the output segments are unchanged.

Overflow warning: `seg_intersection` forms the usual quadratic cross products, but the $y$-ordering
cross-multiplication (`ay * bdx`) is cubic in the coordinate magnitude. With 32-bit `int` endpoints
this overflows once coordinates reach the low thousands, so use a 64-bit (`int64_t`) coordinate type
for any non-trivial integer inputs.

Time Complexity:
- O(n log n) per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) auxiliary, where $n$ is the distance between `lo` and `hi`.

*/

#include <algorithm>
#include <cassert>
#include <iterator>
#include <set>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Simplified detection-only version of seg_intersection() from 7.2.3 (exact for integral Pt).
template<typename Pt>
bool seg_intersection(const Pt &a, const Pt &b, const Pt &c, const Pt &d) {
  auto cross = [](const Pt &p, const Pt &q, const Pt &r) {
    return (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
  };
  auto c1 = cross(a, b, c), c2 = cross(a, b, d), c3 = cross(c, d, a), c4 = cross(c, d, b);
  if (c1 == 0 && c2 == 0 && c3 == 0 && c4 == 0) {
    Pt p = std::max(std::min(a, b), std::min(c, d));
    Pt q = std::min(std::max(a, b), std::max(c, d));
    return !(q < p);
  }
  auto straddles = [](auto x, auto y) { return (x <= 0 && 0 <= y) || (y <= 0 && 0 <= x); };
  return straddles(c1, c2) && straddles(c3, c4);
}

// The point type Pt must support operator< (used to canonicalize endpoint order).
template<typename Pt>
struct Segment {
  Pt p, q;
  Segment() = default;
  Segment(const Pt &p, const Pt &q) : p(std::min(p, q)), q(std::max(p, q)) {}
};

template<typename Pt>
bool intersects(const Segment<Pt> &s1, const Segment<Pt> &s2) {
  return seg_intersection(s1.p, s1.q, s2.p, s2.q);
}

template<typename It>
bool find_intersecting_pair(
    It lo, It hi, typename std::iterator_traits<It>::value_type *res1,
    typename std::iterator_traits<It>::value_type *res2
) {
  using Pt = std::decay_t<decltype(lo->p)>;
  struct Event {
    Pt p;
    bool is_end;
    It seg;
  };
  std::vector<Event> e;
  for (It it = lo; it != hi; ++it) {
    e.push_back(Event{it->p, false, it});
    e.push_back(Event{it->q, true, it});
  }
  std::sort(e.begin(), e.end(), [](const Event &a, const Event &b) {
    return std::tie(a.p.x, a.is_end, a.p.y, a.seg) < std::tie(b.p.x, b.is_end, b.p.y, b.seg);
  });
  // Compare y-values at sweep coordinate x without division: y = (p.y*dx + dy*(x - p.x)) / dx.
  // Vertical segments use their lower endpoint.
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
  using ActiveSet = std::set<It, decltype(cmp)>;
  ActiveSet s(cmp);
  std::vector<typename ActiveSet::iterator> position(hi - lo);
  for (const auto &ev : e) {
    It seg = ev.seg;
    if (!ev.is_end) {
      auto it = s.insert(seg).first;
      position[seg - lo] = it;
      auto next = it;
      if (++next != s.end() && intersects(**next, *seg)) {
        *res1 = **next;
        *res2 = *seg;
        return true;
      }
      if (it != s.begin() && intersects(**--it, *seg)) {
        *res1 = **it;
        *res2 = *seg;
        return true;
      }
    } else {
      auto it = position[seg - lo];
      auto next = it;
      if (++next != s.end() && it != s.begin()) {
        auto prev = it;
        if (intersects(**next, **--prev)) {
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
    assert(find_intersecting_pair(v.begin(), v.end(), &res1, &res2));
    assert(res1.p == Point(0, 0) && res1.q == Point(2, 2));
    assert(res2.p == Point(0, 2) && res2.q == Point(2, -2));
  }
  {  // Integer-coordinate segments: detection is exact.
    vector<Segment<PointI>> v{
        Segment<PointI>({0, 0}, {2, 2}), Segment<PointI>({3, 0}, {0, -1}),
        Segment<PointI>({0, 2}, {2, -2}), Segment<PointI>({0, 3}, {9, 0})
    };
    Segment<PointI> res1, res2;
    assert(find_intersecting_pair(v.begin(), v.end(), &res1, &res2));

    const vector<Segment<PointI>> disjoint{
        Segment<PointI>({0, 0}, {1, 0}), Segment<PointI>({0, 5}, {1, 5})
    };
    assert(!find_intersecting_pair(disjoint.begin(), disjoint.end(), &res1, &res2));
  }
  {  // Shared endpoints count as intersections.
    vector<Segment<PointI>> shared{
        Segment<PointI>({0, 0}, {2, 2}), Segment<PointI>({2, 2}, {4, 0})
    };
    Segment<PointI> res1, res2;
    assert(find_intersecting_pair(shared.begin(), shared.end(), &res1, &res2));
  }
  return 0;
}
