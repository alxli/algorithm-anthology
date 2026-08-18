/*

Given a set of pairs $(m, b)$ specifying lines of the form $y = mx + b$, answer queries at specified
$x$-coordinates, each asking for the minimum $y$-value over all given lines. This is useful for
dynamic programming recurrences of the form $dp(i) = \min_j (m_j x_i + b_j)$. Only the lower
envelope of the lines can ever answer a query, so each added line pops previously stored lines that
it renders useless, and ascending queries advance a pointer along the envelope so every line is
visited at most twice.

The following implementation is a concise, semi-dynamic version of the convex hull optimization
technique. It supports an interlaced sequence of `add_line()` and `query()` calls, as long as the
preconditions of descending `m` and ascending `x` are satisfied. As a result, it may be necessary to
sort the lines and queries before calling the functions. In that case, the overall time complexity
will be dominated by the sorting step.

- `SemiDynamicCHT()` constructs an empty hull.
- `add_line(m, b)` inserts the line $y = mx + b$. The caller must ensure that slope `m` is less than
  or equal to the slope of every line added so far.
- `query(x)` returns the minimum $y$-value among all inserted lines at coordinate `x`. The caller
  must ensure that query coordinates are nondecreasing across calls. At least one line must have
  been inserted.

Overflow warning: `add_line()` compares intersections by cross-multiplying slope and intercept
differences, a product on the order of the squared coefficient magnitude. For large `m`/`b` (roughly
beyond $10^9$ with `int64_t`), cast that comparison to `__int128`. `query()` evaluates `m * x + b`,
which also needs to fit in `int64_t`.

Time Complexity:
- O(n + q) for any interlaced sequence containing $n$ calls to `add_line()` and $q$ calls to
  `query()`. Each line is removed or passed by the query pointer at most once, so both operations
  take amortized O(1) time.

Space Complexity:
- O(n) for storage of the lines.
- O(1) auxiliary for `add_line()` and `query()`.

*/

#include <cassert>
#include <cstdint>
#include <vector>

class SemiDynamicCHT {
  std::vector<int64_t> vm, vb;
  int ptr = 0;

 public:
  void add_line(int64_t m, int64_t b) {
    int len = static_cast<int>(vm.size());
    if (len > 0 && vm.back() == m) {
      if (vb.back() <= b) {
        return;
      }
      vm.pop_back();
      vb.pop_back();
      len--;
      if (ptr > len) {
        ptr = len;
      }
    }
    while (len > 1 && (vb[len - 2] - vb[len - 1]) * (m - vm[len - 1]) >=
                          (vb[len - 1] - b) * (vm[len - 1] - vm[len - 2])) {  // Overflow warning.
      len--;
    }
    vm.resize(len);
    vb.resize(len);
    vm.push_back(m);
    vb.push_back(b);
  }

  int64_t query(int64_t x) {
    assert(!vm.empty());
    if (ptr >= static_cast<int>(vm.size())) {
      ptr = static_cast<int>(vm.size()) - 1;
    }
    while (ptr + 1 < static_cast<int>(vm.size()) &&
           vm[ptr + 1] * x + vb[ptr + 1] <= vm[ptr] * x + vb[ptr]) {  // Overflow warning.
      ptr++;
    }
    return vm[ptr] * x + vb[ptr];
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  SemiDynamicCHT cht;
  cht.add_line(3, 0);
  cht.add_line(2, 1);
  cht.add_line(1, 2);
  cht.add_line(0, 6);
  assert(cht.query(0) == 0);
  assert(cht.query(1) == 3);
  assert(cht.query(2) == 4);
  assert(cht.query(3) == 5);
  cht.add_line(0, 7);  // Same slope, worse intercept; ignored.
  assert(cht.query(4) == 6);
  cht.add_line(0, 5);  // Same slope, better intercept; replaces the old constant line.
  assert(cht.query(5) == 5);
  return 0;
}
