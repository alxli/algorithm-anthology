/*

Maintain a fixed-size array under the range "chmin" update `a[i] = min(a[i], t)` for all `i` in a
range, while answering range-sum and range-maximum queries. An ordinary lazy segment tree cannot
represent a clamp like this with a single composable tag, because clamping affects only the entries
that currently exceed `t`. Segment tree beats (the Ji Driver tree) resolves this by storing, for
each node, the strict maximum, the second-largest distinct value, and a count of entries equal to
the maximum. A clamp then has three cases at each node.

- If `t >= max1`, the clamp changes nothing and the recursion stops (the prune case).
- If `max2 < t < max1`, every entry equal to `max1` drops to `t` and all others are untouched, so
  the node updates in O(1): the sum falls by `(max1 - t) * count` and `max1` becomes `t` (the break,
  or tag, case). This lazy tag pushes down to a child only when the child's maximum is larger.
- Otherwise (`t <= max2`) the node is too coarse to update directly, so the clamp recurses into both
  children and re-pulls.

The break condition is what makes this efficient: a potential-function argument on the number of
distinct values along root-to-leaf paths shows the total work over any sequence of clamps is
amortized O(log^2 n) each, even though a single clamp can momentarily descend deep. The same scheme
extends to range "chmax" by tracking the symmetric minimum trio, and to mixed clamp-and-add
workloads by carrying an additional add tag; only the chmin / sum / max core is shown here. Unlike
the generic lazy segment tree, this structure cannot be reduced to a pluggable `combine` and
`apply`, because a clamp acts on only the maximal entries of a node rather than uniformly on all of
them, so each operation set must be hand-written and its amortized bound argued separately.

Use a 64-bit element type `T` when sums can be large.

- `SegTreeBeats(n, v)` constructs an array of size `n`, indices 0 to `n - 1`, all equal to `v`.
- `SegTreeBeats(lo, hi)` constructs the array from the range `[lo, hi)` of random-access iterators.
- `size()` returns the size of the array.
- `chmin(lo, hi, t)` replaces `a[i]` with `min(a[i], t)` for every `i` in `[lo, hi]`, inclusive.
- `query_sum(lo, hi)` returns the sum of `a[i]` over `i` in `[lo, hi]`, inclusive.
- `query_max(lo, hi)` returns the maximum of `a[i]` over `i` in `[lo, hi]`, inclusive.
- `at(i)` returns the value at index `i`.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log^2 n) amortized per call to `chmin()`.
- O(log n) per call to `query_sum()`, `query_max()`, and `at()`.

Space Complexity:
- O(n) for storage of the tree.
- O(log n) auxiliary stack space per `chmin()` and query call.

*/

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

template<class T>
class SegTreeBeats {
  static constexpr T NEG_INF = std::numeric_limits<T>::lowest();

  int len;
  std::vector<T> max1, max2, sum;
  std::vector<int> cnt;  // Number of entries in the node equal to max1.

  void pull(int i) {
    int l = 2 * i + 1, r = 2 * i + 2;
    sum[i] = sum[l] + sum[r];
    if (max1[l] == max1[r]) {
      max1[i] = max1[l];
      cnt[i] = cnt[l] + cnt[r];
      max2[i] = std::max(max2[l], max2[r]);
    } else if (max1[l] > max1[r]) {
      max1[i] = max1[l];
      cnt[i] = cnt[l];
      max2[i] = std::max(max2[l], max1[r]);
    } else {
      max1[i] = max1[r];
      cnt[i] = cnt[r];
      max2[i] = std::max(max1[l], max2[r]);
    }
  }

  // Lower the node's maximum to t. Valid only when max2 < t < max1, so exactly the cnt entries
  // equal to max1 change and the second maximum is left untouched.
  void apply_chmin(int i, T t) {
    sum[i] -= (max1[i] - t) * static_cast<T>(cnt[i]);
    max1[i] = t;
  }

  void push(int i) {
    for (int c : {2 * i + 1, 2 * i + 2}) {
      if (max1[c] > max1[i]) {
        apply_chmin(c, max1[i]);
      }
    }
  }

  template<class Gen>
  void build(int i, int lo, int hi, const Gen &gen) {
    if (lo == hi) {
      max1[i] = sum[i] = gen(lo);
      max2[i] = NEG_INF;
      cnt[i] = 1;
      return;
    }
    int mid = lo + (hi - lo) / 2;
    build(i * 2 + 1, lo, mid, gen);
    build(i * 2 + 2, mid + 1, hi, gen);
    pull(i);
  }

  void chmin(int i, int lo, int hi, int tlo, int thi, T t) {
    if (thi < lo || hi < tlo || max1[i] <= t) {
      return;  // Prune: outside the range, or every entry is already at most t.
    }
    if (tlo <= lo && hi <= thi && max2[i] < t) {
      apply_chmin(i, t);  // Break: only the maximal entries are affected.
      return;
    }
    push(i);
    int mid = lo + (hi - lo) / 2;
    chmin(i * 2 + 1, lo, mid, tlo, thi, t);
    chmin(i * 2 + 2, mid + 1, hi, tlo, thi, t);
    pull(i);
  }

  T query_sum(int i, int lo, int hi, int tlo, int thi) {
    if (thi < lo || hi < tlo) {
      return 0;
    }
    if (tlo <= lo && hi <= thi) {
      return sum[i];
    }
    push(i);
    int mid = lo + (hi - lo) / 2;
    return query_sum(i * 2 + 1, lo, mid, tlo, thi) + query_sum(i * 2 + 2, mid + 1, hi, tlo, thi);
  }

  T query_max(int i, int lo, int hi, int tlo, int thi) {
    if (thi < lo || hi < tlo) {
      return NEG_INF;
    }
    if (tlo <= lo && hi <= thi) {
      return max1[i];
    }
    push(i);
    int mid = lo + (hi - lo) / 2;
    return std::max(
        query_max(i * 2 + 1, lo, mid, tlo, thi), query_max(i * 2 + 2, mid + 1, hi, tlo, thi)
    );
  }

 public:
  explicit SegTreeBeats(int n, const T &v = T())
      : len(n), max1(4 * n), max2(4 * n), sum(4 * n), cnt(4 * n) {
    if (len > 0) {
      build(0, 0, len - 1, [&](int) { return v; });
    }
  }

  template<class It>
  SegTreeBeats(It lo, It hi)
      : len(static_cast<int>(hi - lo)), max1(4 * len), max2(4 * len), sum(4 * len), cnt(4 * len) {
    if (len > 0) {
      build(0, 0, len - 1, [&](int i) { return *(lo + i); });
    }
  }

  int size() const { return len; }
  void chmin(int lo, int hi, const T &t) { chmin(0, 0, len - 1, lo, hi, t); }
  T query_sum(int lo, int hi) { return query_sum(0, 0, len - 1, lo, hi); }
  T query_max(int lo, int hi) { return query_max(0, 0, len - 1, lo, hi); }
  T at(int i) { return query_sum(0, 0, len - 1, i, i); }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int64_t> a{5, 4, 3, 2, 1};
  SegTreeBeats<int64_t> t(a.begin(), a.end());
  assert(t.query_sum(0, 4) == 15);
  assert(t.query_max(0, 4) == 5);

  t.chmin(0, 2, 3);  // {3, 3, 3, 2, 1}: only the 5 and 4 are clamped down to 3.
  assert(t.at(0) == 3);
  assert(t.at(1) == 3);
  assert(t.query_sum(0, 4) == 12);
  assert(t.query_max(0, 4) == 3);

  t.chmin(0, 4, 2);  // {2, 2, 2, 2, 1}.
  assert(t.query_sum(0, 4) == 9);
  assert(t.query_max(0, 3) == 2);

  // Cross-check against a brute-force array over random clamps and queries.
  vector<int64_t> b{7, 1, 4, 9, 2, 6, 5, 8, 3, 0};
  SegTreeBeats<int64_t> s(b.begin(), b.end());
  int clamps[][3] = {{0, 9, 6}, {2, 5, 3}, {4, 8, 7}, {0, 4, 2}, {3, 9, 5}};
  for (auto &c : clamps) {
    for (int i = c[0]; i <= c[1]; i++) {
      b[i] = min(b[i], static_cast<int64_t>(c[2]));
    }
    s.chmin(c[0], c[1], c[2]);
    for (int lo = 0; lo < 10; lo++) {
      int64_t want_sum = 0, want_max = INT64_MIN;
      for (int hi = lo; hi < 10; hi++) {
        want_sum += b[hi];
        want_max = max(want_max, b[hi]);
        assert(s.query_sum(lo, hi) == want_sum);
        assert(s.query_max(lo, hi) == want_max);
      }
    }
  }
  return 0;
}
