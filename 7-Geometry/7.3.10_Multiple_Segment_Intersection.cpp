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

- `find_intersecting_pair(segs)` returns the indices of one pair of intersecting segments, or
  `std::nullopt` if none exist. Constructing a `Segment` places its lexicographically smaller
  endpoint first, as required by the sweep.

Overflow warning: `seg_intersection` forms the usual quadratic cross products, but the $y$-ordering
cross-multiplication (`ay * bdx`) is cubic in the coordinate magnitude. Use `int64_t` coordinates
for modest integer inputs and a wider intermediate type when these cubic products may exceed it.

Time Complexity:
- O(n log n) per call, where $n$ is the number of segments.

Space Complexity:
- O(n) auxiliary, where $n$ is the number of segments.

*/

#include <algorithm>
#include <cassert>
#include <optional>
#include <set>
#include <tuple>
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
  Segment(const Pt &p, const Pt &q) : p(std::min(p, q)), q(std::max(p, q)) {}
};

template<typename Pt>
bool intersects(const Segment<Pt> &s1, const Segment<Pt> &s2) {
  return seg_intersection(s1.p, s1.q, s2.p, s2.q);
}

template<typename Pt>
std::optional<std::pair<int, int>> find_intersecting_pair(const std::vector<Segment<Pt>> &segs) {
  struct Event {
    Pt p;
    bool is_end;
    int seg;
  };
  std::vector<Event> e;
  for (int i = 0; i < static_cast<int>(segs.size()); i++) {
    e.push_back(Event{segs[i].p, false, i});
    e.push_back(Event{segs[i].q, true, i});
  }
  std::sort(e.begin(), e.end(), [](const Event &a, const Event &b) {
    return std::tie(a.p.x, a.is_end, a.p.y, a.seg) < std::tie(b.p.x, b.is_end, b.p.y, b.seg);
  });
  // Compare y-values at sweep coordinate x without division: y = (p.y*dx + dy*(x - p.x)) / dx.
  // Vertical segments use their lower endpoint.
  auto ycmp = [](const auto &a, const auto &b, auto x) {
    // Overflow risk for integer Pt: the final cross-multiply is ~O(max_coord^3).
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
  auto cmp = [&segs, ycmp](int a, int b) {
    if (a == b) {
      return false;
    }
    auto x = std::max(segs[a].p.x, segs[b].p.x);
    int order = ycmp(segs[a], segs[b], x);
    return (order == 0) ? (a < b) : (order < 0);
  };
  using ActiveSet = std::set<int, decltype(cmp)>;
  ActiveSet s(cmp);
  std::vector<typename ActiveSet::iterator> position(segs.size());
  auto result = [](int i, int j) { return std::pair{std::min(i, j), std::max(i, j)}; };
  for (const auto &ev : e) {
    int seg = ev.seg;
    if (!ev.is_end) {
      auto it = s.insert(seg).first;
      position[seg] = it;
      auto next = it;
      if (++next != s.end() && intersects(segs[*next], segs[seg])) {
        return result(*next, seg);
      }
      if (it != s.begin() && intersects(segs[*--it], segs[seg])) {
        return result(*it, seg);
      }
    } else {
      auto it = position[seg];
      auto next = it;
      if (++next != s.end() && it != s.begin()) {
        auto prev = it;
        if (intersects(segs[*next], segs[*--prev])) {
          return result(*next, *prev);
        }
      }
      s.erase(it);
    }
  }
  return std::nullopt;
}

/*** Example Usage ***/

#include <vector>
using namespace std;

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
  bool operator<(const PointI &p) const { return x != p.x ? x < p.x : y < p.y; }
};

int main() {
  {  // Double-coordinate segments.
    vector<Segment<Point>> v{
        Segment<Point>(Point(0, 0), Point(2, 2)), Segment<Point>(Point(3, 0), Point(0, -1)),
        Segment<Point>(Point(0, 2), Point(2, -2)), Segment<Point>(Point(0, 3), Point(9, 0))
    };
    assert((find_intersecting_pair(v) == pair{0, 2}));
  }
  {  // Integer-coordinate segments: detection is exact.
    vector<Segment<PointI>> v{
        Segment<PointI>({0, 0}, {2, 2}), Segment<PointI>({3, 0}, {0, -1}),
        Segment<PointI>({0, 2}, {2, -2}), Segment<PointI>({0, 3}, {9, 0})
    };
    assert((find_intersecting_pair(v) == pair{0, 2}));

    const vector<Segment<PointI>> disjoint{
        Segment<PointI>({0, 0}, {1, 0}), Segment<PointI>({0, 5}, {1, 5})
    };
    assert(!find_intersecting_pair(disjoint));
    const vector<Segment<PointI>> duplicate{
        Segment<PointI>({0, 0}, {2, 2}), Segment<PointI>({0, 0}, {2, 2})
    };
    assert((find_intersecting_pair(duplicate) == pair{0, 1}));
  }
  {  // Shared endpoints count as intersections.
    vector<Segment<PointI>> shared{
        Segment<PointI>({0, 0}, {2, 2}), Segment<PointI>({2, 2}, {4, 0})
    };
    assert((find_intersecting_pair(shared) == pair{0, 1}));
  }
  return 0;
}
