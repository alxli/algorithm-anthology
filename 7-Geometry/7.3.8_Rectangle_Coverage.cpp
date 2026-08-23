/*

Given weighted axis-aligned rectangles, iterate over every elementary cell induced by their
coordinate-compressed boundaries. Within each such cell, the accumulated weight is constant, so the
callback can compute rectangle union area, exactly-$k$ coverage area, threshold area, maximum
overlap, weighted integrals, coverage histograms, or other offline coverage statistics.

Rectangles are half-open, $[x_1, x_2) \times [y_1, y_2)$, so adjacent rectangles sharing only an
edge do not double-count any area. This also matches the difference-array update: add at the lower
boundaries and subtract at the first coordinate after the rectangle.

- `for_each_rect_cell(rects, visit)` calls `visit(x1, y1, x2, y2, weight)` once for each
  positive-area compressed cell inside the bounding box of the rectangles, where `weight` is the sum
  of weights covering that cell. Cells are visited in increasing `x1`, then increasing `y1`. Each
  rectangle must satisfy $`x1` < `x2`$ and $`y1` < `y2`$.

Enumerating cells costs O(n^2) time and memory, which is the price of answering any statistic at all
rather than one chosen in advance. When only the union area or perimeter is wanted, the sweep of
section 7.3.9 computes it in O(n log n) time and O(n) memory, which is the only option once the
rectangle count reaches the thousands.

Overflow warning: Coordinate differences, cell areas, and accumulated weights must fit in `int64_t`.

Time Complexity:
- O(n^2 + n log n) per call, where $n$ is the number of rectangles.

Space Complexity:
- O(n^2) auxiliary.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

struct Rect {
  int64_t x1, y1, x2, y2, w = 1;
};

template<typename Fn>
void for_each_rect_cell(const std::vector<Rect> &rects, Fn visit) {
  assert(std::all_of(rects.begin(), rects.end(), [](const Rect &r) {
    return r.x1 < r.x2 && r.y1 < r.y2;
  }));
  std::vector<int64_t> xs, ys;
  for (const Rect &r : rects) {
    xs.push_back(r.x1);
    xs.push_back(r.x2);
    ys.push_back(r.y1);
    ys.push_back(r.y2);
  }
  std::sort(xs.begin(), xs.end());
  std::sort(ys.begin(), ys.end());
  xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
  ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
  int X = static_cast<int>(xs.size()), Y = static_cast<int>(ys.size());
  std::vector<std::vector<int64_t>> diff(X + 1, std::vector<int64_t>(Y + 1));
  auto index = [](const std::vector<int64_t> &v, int64_t x) {
    return static_cast<int>(std::lower_bound(v.begin(), v.end(), x) - v.begin());
  };
  for (const Rect &r : rects) {
    int x1 = index(xs, r.x1), x2 = index(xs, r.x2);
    int y1 = index(ys, r.y1), y2 = index(ys, r.y2);
    diff[x1][y1] += r.w;  // Overflow warning.
    diff[x2][y1] -= r.w;
    diff[x1][y2] -= r.w;
    diff[x2][y2] += r.w;
  }
  for (int x = 0; x < X; x++) {
    for (int y = 0; y < Y; y++) {
      diff[x][y] += (x > 0) ? diff[x - 1][y] : 0;
      diff[x][y] += (y > 0) ? diff[x][y - 1] : 0;
      diff[x][y] -= (x > 0 && y > 0) ? diff[x - 1][y - 1] : 0;
      if (x + 1 < X && y + 1 < Y) {
        visit(xs[x], ys[y], xs[x + 1], ys[y + 1], diff[x][y]);
      }
    }
  }
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  auto rect_area_if = [](const vector<Rect> &rects, auto pred) {
    int64_t area = 0;
    for_each_rect_cell(rects, [&](int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t w) {
      if (pred(w)) {
        area += (x2 - x1) * (y2 - y1);  // Overflow warning.
      }
    });
    return area;
  };

  // Rectangles A=[0,2)x[0,2), B=[1,3)x[1,3), and C=[2,4)x[0,1) all with default weight 1.
  //
  // y=3 +---+---+---+---+
  //     |   | B | B |   |
  // y=2 +---+---+---+---+
  //     | A |A/B| B |   |
  // y=1 +---+---+---+---+
  //     | A | A | C | C |
  // y=0 +---+---+---+---+
  //   x=0   1   2   3   4
  vector<Rect> rects{{0, 0, 2, 2}, {1, 1, 3, 3}, {2, 0, 4, 1}};
  assert(rect_area_if(rects, [](int64_t w) { return w > 0; }) == 9);
  assert(rect_area_if(rects, [](int64_t w) { return w == 2; }) == 1);

  // Rectangles A=[0,4)x[0,3) of w=2, B=[2,6)x[1,4) of w=3, and C=[1,5)x[2,5) of w=1.
  //
  // y=5 +---+---+---+---+---+---+
  //     |   | 1 | 1 | 1 | 1 |   |
  // y=4 +---+---+---+---+---+---+
  //     |   | 1 | 4 | 4 | 4 | 3 |
  // y=3 +---+---+---+---+---+---+
  //     | 2 | 3 | 6 | 6 | 4 | 3 |
  // y=2 +---+---+---+---+---+---+
  //     | 2 | 2 | 5 | 5 | 3 | 3 |
  // y=1 +---+---+---+---+---+---+
  //     | 2 | 2 | 2 | 2 |   |   |
  // y=0 +---+---+---+---+---+---+
  //   x=0   1   2   3   4   5   6
  vector<Rect> weighted{{0, 0, 4, 3, 2}, {2, 1, 6, 4, 3}, {1, 2, 5, 5, 1}};
  assert(rect_area_if(weighted, [](int64_t w) { return w >= 3; }) == 13);
  assert(rect_area_if(weighted, [](int64_t w) { return w >= 5; }) == 4);

  int64_t max_weight = 0, integral = 0;
  for_each_rect_cell(weighted, [&](int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t w) {
    max_weight = max(max_weight, w);
    integral += w * (x2 - x1) * (y2 - y1);  // Overflow warning.
  });
  assert(max_weight == 6);
  assert(integral == 12 * 2 + 12 * 3 + 12);
  return 0;
}
