/*

Given a set of pairs $(m, b)$ specifying lines of the form $y = mx + b$, process a set of
$x$-coordinate queries each asking to find the minimum $y$-value when any of the given lines are
evaluated at the specified $x$. This is useful for dynamic programming recurrences of the form
`dp[i] = min(m[j] * x[i] + b[j])`.

The following implementation is a concise, semi-dynamic version of the convex hull optimization
technique. It supports an interlaced sequence of `add_line()` and `query()` calls, as long as the
preconditions of descending `m` and ascending `x` are satisfied. As a result, it may be necessary to
sort the lines and queries before calling the functions. In that case, the overall time complexity
will be dominated by the sorting step.

- `SemiDynamicCHT()` constructs an empty hull.
- `add_line(m, b)` inserts the line $y = mx + b$. The caller must ensure that slope `m` is less than
  or equal to the slope of every line added so far.
- `query(x)` returns the minimum $y$-value among all inserted lines at coordinate `x`. The caller
  must ensure that query coordinates are nondecreasing across calls.

Overflow warning: `add_line()` compares intersections by cross-multiplying slope and intercept
differences, a product on the order of the squared coefficient magnitude. For large `m`/`b` (roughly
beyond $10^9$ with 64-bit `long long`), cast that comparison to `__int128`. `query()` only forms the
single product `m * x`, which simply needs to fit in `long long`.

Time Complexity:
- O(n) for any interlaced sequence of `add_line()` and `query()` calls, where $n$ is the number of
  lines added. This is because the overall number of steps taken by `add_line()` and `query()` are
  respectively bounded by the number of lines. Thus a single call to either `add_line()` or
  `query()` will have an amortized O(1) running time.

Space Complexity:
- O(n) for storage of the lines.
- O(1) auxiliary for `add_line()` and `query()`.

*/

#include <vector>

struct SemiDynamicCHT {
  std::vector<long long> M, B;
  int ptr = 0;

  void add_line(long long m, long long b) {
    int len = M.size();
    if (len > 0 && M.back() == m) {
      if (B.back() <= b) {
        return;
      }
      M.pop_back();
      B.pop_back();
      len--;
      if (ptr > len) {
        ptr = len;
      }
    }
    // Overflow risk: this cross-multiplication is ~O(coeff^2); cast to __int128 for large m/b.
    while (len > 1 && (B[len - 2] - B[len - 1]) * (m - M[len - 1]) >=
                          (B[len - 1] - b) * (M[len - 1] - M[len - 2])) {
      len--;
    }
    M.resize(len);
    B.resize(len);
    M.push_back(m);
    B.push_back(b);
  }

  long long query(long long x) {
    if (ptr >= static_cast<int>(M.size())) {
      ptr = static_cast<int>(M.size()) - 1;
    }
    while (ptr + 1 < static_cast<int>(M.size()) &&
           M[ptr + 1] * x + B[ptr + 1] <= M[ptr] * x + B[ptr]) {
      ptr++;
    }
    return M[ptr] * x + B[ptr];
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
  return 0;
}
