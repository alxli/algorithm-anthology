/*

Given axis-aligned rectangles that may overlap arbitrarily, measure their union. Klee's algorithm
sweeps a vertical line left to right, maintaining how much of that line the rectangles currently
cover. That covered length is constant between consecutive events, so each strip contributes the
length times its width, and every other measure below is another aggregate of the same cross
section.

A segment tree over the compressed $y$ coordinates holds the state, storing at each node the number
of intervals covering its whole span and the length of that span which is covered. It never pushes
anything down: a node's count refers only to intervals covering it entirely, so a node with a
positive count reports its full span while one with a zero count defers to its children. Every
addition is matched by a later removal, so counts never go negative and no lazy tag is needed.

Three further aggregates ride along on the same nodes. Doubly covered length shifts the recurrence
by one, a node covered twice reporting its whole span and a node covered once reporting whatever its
children cover at all. The weighted maximum is the largest root-to-leaf sum of weights, so each node
reports its own weight plus the larger of its children's; left at the default weight of one per
rectangle, that is simply the greatest number of rectangles covering a point. The perimeter needs
the number of maximal covered runs, plus whether a node's first and last units are covered so that a
run crossing a boundary is not counted twice: vertical edges are then the change in covered length
at each event, and horizontal edges are twice the run count times the strip width.

Rectangles are half-open, $[x_1, x_2) \times [y_1, y_2)$, so adjacent rectangles sharing only an
edge neither double-count any area nor leave a seam.

- `rect_union(rects)` returns a `UnionMeasure` holding the `area` covered at least once, the
  `overlap_area` covered at least twice, the `perimeter` of the outline including any enclosed hole,
  and `max_weight`, the greatest weighted sum at any point. Subtracting the second from the first
  leaves the area covered exactly once. Each rectangle must satisfy $`x1` < `x2`$ and $`y1` < `y2`$.

Weights change only the maximum, since the three geometric measures count coverage rather than
payload, and weighted area needs no sweep at all: it is $\sum w_i$ times area, one loop over the
input. Area covered at least $k$ times instead needs a vector of $k$ lengths per node; for that or
any aggregate that does not decompose this way, the compression of section 7.3.8 enumerates every
elementary cell at O(n^2).

Overflow warning: Coordinate differences, areas, perimeters, and weight sums must fit in `int64_t`.

Time Complexity:
- O(n log n) per call, where $n$ is the number of rectangles.

Space Complexity:
- O(n) auxiliary.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <tuple>
#include <utility>
#include <vector>

struct Rect {
  int64_t x1, y1, x2, y2, w = 1;
};

struct UnionMeasure {
  int64_t area = 0, overlap_area = 0, perimeter = 0, max_weight = 0;
};

// The cross section of the sweep: over the currently covered y intervals, the length covered at
// least once and at least twice, the number of maximal covered runs, and the greatest weight at any
// point. Each is its own recurrence, so each is pulled up separately.
class CoverTree {
  std::vector<int64_t> ys;  // Compressed coordinates; leaf i spans [ys[i], ys[i + 1]).
  int len;
  std::vector<int> count, runs;
  std::vector<int64_t> covered, covered_twice, weight, heaviest;
  std::vector<char> covers_lo, covers_hi;

  // Length covered at least once and at least twice. The second reads the first one level down, so
  // covering at least k times extends this chain by one array per level.
  void pull_coverage(int i, int lo, int hi) {
    int left = 2 * i + 1, right = left + 1;
    if (count[i] > 0) {
      covered[i] = ys[hi + 1] - ys[lo];
      // One cover here turns anything already covered below into a double cover.
      covered_twice[i] =
          count[i] > 1 ? covered[i] : (lo == hi ? 0 : covered[left] + covered[right]);
    } else if (lo == hi) {
      covered[i] = covered_twice[i] = 0;
    } else {
      covered[i] = covered[left] + covered[right];
      covered_twice[i] = covered_twice[left] + covered_twice[right];
    }
  }

  // Number of maximal covered runs, with the flags that keep a run spanning a boundary from being
  // counted in both children.
  void pull_runs(int i, int lo, int hi) {
    int left = 2 * i + 1, right = left + 1;
    if (count[i] > 0) {
      runs[i] = covers_lo[i] = covers_hi[i] = 1;
    } else if (lo == hi) {
      runs[i] = covers_lo[i] = covers_hi[i] = 0;
    } else {
      runs[i] = runs[left] + runs[right] - (covers_hi[left] && covers_lo[right] ? 1 : 0);
      covers_lo[i] = covers_lo[left];
      covers_hi[i] = covers_hi[right];
    }
  }

  // The weight over an interval is the sum of the weights above it, so the heaviest point is the
  // largest root-to-leaf sum. This one ignores the cover count entirely.
  void pull_weight(int i, int lo, int hi) {
    heaviest[i] = weight[i] + (lo == hi ? 0 : std::max(heaviest[2 * i + 1], heaviest[2 * i + 2]));
  }

 public:
  explicit CoverTree(std::vector<int64_t> coords)
      : ys(std::move(coords)),
        len(static_cast<int>(ys.size()) - 1),
        count(4 * len),
        runs(4 * len),
        covered(4 * len),
        covered_twice(4 * len),
        weight(4 * len),
        heaviest(4 * len),
        covers_lo(4 * len),
        covers_hi(4 * len) {
    assert(len > 0);
  }

  // Adds delta covers of weight w to every elementary interval in [lo, hi]; a removal negates both.
  void update(int lo, int hi, int delta, int64_t w) {
    auto rec = [&](auto &&rec, int i, int l, int h) {
      if (hi < l || h < lo) {
        return;
      }
      if (lo <= l && h <= hi) {
        count[i] += delta;
        weight[i] += delta * w;
      } else {
        int mid = l + (h - l) / 2;
        rec(rec, 2 * i + 1, l, mid);
        rec(rec, 2 * i + 2, mid + 1, h);
      }
      pull_coverage(i, l, h);
      pull_runs(i, l, h);
      pull_weight(i, l, h);
    };
    rec(rec, 0, 0, len - 1);
  }

  int64_t covered_length() const { return covered[0]; }
  int64_t twice_covered_length() const { return covered_twice[0]; }
  int run_count() const { return runs[0]; }
  int64_t heaviest_weight() const { return heaviest[0]; }
};

UnionMeasure rect_union(const std::vector<Rect> &rects) {
  if (rects.empty()) {
    return {};
  }
  std::vector<int64_t> ys;
  for (const Rect &r : rects) {
    assert(r.x1 < r.x2 && r.y1 < r.y2);
    ys.push_back(r.y1);
    ys.push_back(r.y2);
  }
  std::sort(ys.begin(), ys.end());
  ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
  auto index = [&](int64_t y) {
    return static_cast<int>(std::lower_bound(ys.begin(), ys.end(), y) - ys.begin());
  };
  std::vector<std::tuple<int64_t, int, int, int, int64_t>> events;  // (x, lo, hi, delta, weight)
  for (const Rect &r : rects) {
    events.emplace_back(r.x1, index(r.y1), index(r.y2) - 1, 1, r.w);
    events.emplace_back(r.x2, index(r.y1), index(r.y2) - 1, -1, r.w);
  }
  std::sort(events.begin(), events.end());
  CoverTree tree(std::move(ys));
  UnionMeasure res;
  int64_t width = 0, doubled = 0, strips = 0, prev_x = 0;
  for (int i = 0, nevents = static_cast<int>(events.size()); i < nevents;) {
    int64_t x = std::get<0>(events[i]);
    if (i > 0) {  // The strip since the previous event has a constant cross-section.
      res.area += width * (x - prev_x);
      res.overlap_area += doubled * (x - prev_x);
      res.perimeter += 2 * strips * (x - prev_x);
    }
    int end = i;
    while (end < nevents && std::get<0>(events[end]) == x) {
      end++;
    }
    // Add before removing at the same x. The sum of the resulting length changes is the symmetric
    // difference of the old and new cross-sections: shared edges vanish, while disjoint entering
    // and leaving intervals are both counted.
    for (int wanted_delta : {1, -1}) {
      int64_t before = tree.covered_length();
      for (int j = i; j < end; j++) {
        auto [_, tgt_lo, tgt_hi, delta, w] = events[j];
        if (delta == wanted_delta) {
          tree.update(tgt_lo, tgt_hi, delta, w);
        }
      }
      res.perimeter += std::llabs(tree.covered_length() - before);
    }
    i = end;
    res.max_weight = std::max(res.max_weight, tree.heaviest_weight());
    width = tree.covered_length();
    doubled = tree.twice_covered_length();
    strips = tree.run_count();
    prev_x = x;
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(rect_union({}).area == 0 && rect_union({}).perimeter == 0);

  UnionMeasure one = rect_union({{0, 0, 2, 3}});
  assert(one.area == 6 && one.perimeter == 10);
  assert(one.overlap_area == 0 && one.max_weight == 1);

  // Disjoint rectangles simply add up, and nothing is covered twice.
  UnionMeasure apart = rect_union({{0, 0, 1, 1}, {2, 0, 3, 1}});
  assert(apart.area == 2 && apart.perimeter == 8);
  assert(apart.overlap_area == 0 && apart.max_weight == 1);

  // Disjoint intervals can leave and enter at the same x; both vertical edges still count.
  UnionMeasure crossing = rect_union({{0, 0, 1, 1}, {1, 2, 2, 3}});
  assert(crossing.area == 2 && crossing.perimeter == 8);

  // Two squares meeting in a unit square, whose union outlines an L-shaped hexagon.
  UnionMeasure pair = rect_union({{0, 0, 2, 2}, {1, 1, 3, 3}});
  assert(pair.area == 7 && pair.perimeter == 12);
  assert(pair.overlap_area == 1 && pair.max_weight == 2);
  assert(pair.area - pair.overlap_area == 6);  // Area belonging to exactly one square.

  // A rectangle nested inside another adds no area, but doubles the cover where it sits.
  UnionMeasure nested = rect_union({{0, 0, 10, 10}, {2, 2, 4, 4}});
  assert(nested.area == 100 && nested.perimeter == 40);
  assert(nested.overlap_area == 4 && nested.max_weight == 2);

  // Four rectangles enclosing a hole, overlapping only at the four corners.
  UnionMeasure ring = rect_union({{0, 0, 3, 1}, {0, 2, 3, 3}, {0, 0, 1, 3}, {2, 0, 3, 3}});
  assert(ring.area == 8);
  assert(ring.perimeter == 16);  // 12 around the outside and 4 around the unit hole.
  assert(ring.overlap_area == 4 && ring.max_weight == 2);

  // Three rectangles stacked over one cell.
  UnionMeasure stack = rect_union({{0, 0, 2, 2}, {1, 0, 3, 2}, {1, 1, 3, 3}});
  assert(stack.max_weight == 3 && stack.overlap_area == 3);
  return 0;
}
