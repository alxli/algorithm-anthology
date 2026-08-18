/*

A wavelet tree is a static structure over an integer array with values in a known range
$[`min_val`, `max_val`]$. The root splits the value range at its midpoint, recording for every
prefix of the array how many of its elements belong to the lower value half, then stably partitions
the array so that those elements form the left child and the rest form the right child. Recursing on
each half builds a balanced tree of height O(log s), where $s$ is the size of the value range.

The recorded prefix counts let a query at any node translate a range of array positions into the
corresponding range of positions in either child, so a single root-to-leaf descent answers order
statistics and rank queries. This is strictly more powerful than a merge sort tree: besides counting
values below a threshold, it reports the $k$-th smallest value of a range in O(log s) time.

All position ranges use 0-based indices and closed intervals $[`lo`, `hi`]$. If the values are large
or sparse, compress them to a small contiguous range first.

- `WaveletTree(a, min_val, max_val)` builds the tree over the array `a`, whose values must all lie
  in $[`min_val`, `max_val`]$.
- `size()` returns the size of the array.
- `kth_smallest(lo, hi, k)` returns the `k`-th smallest value among positions $[`lo`, `hi`]$, where
  `k` is 1-based (so `k == 1` returns the minimum).
- `count_leq(lo, hi, x)` returns the number of positions `i` $\in [`lo`, `hi`]$ such that `a[i]`
  $\leq$ `x`.
- `count_in(lo, hi, x, y)` returns the number of positions `i` $\in [`lo`, `hi`]$ such that `a[i]`
  $\in [`x`, `y`]$.

Time Complexity:
- O(n log s) per call to the constructor, where $n$ is the size of the array and $s$ is the size of
  the value range.
- O(1) per call to `size()`.
- O(log s) per call to `kth_smallest()`, `count_leq()`, and `count_in()`.

Space Complexity:
- O(n log s) for storage of the tree.
- O(log s) auxiliary stack space per query.

*/

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <memory>
#include <vector>

class WaveletTree {
  int min_val, max_val;  // The value range this node covers.
  std::unique_ptr<WaveletTree> left, right;
  std::vector<int> b;  // b[i] = number of the first i elements that go to the left child.

  WaveletTree(std::vector<int>::iterator lo, std::vector<int>::iterator hi, int mn, int mx)
      : min_val(mn), max_val(mx) {
    assert(min_val <= max_val);
    b.reserve((hi - lo) + 1);
    b.push_back(0);
    if (lo >= hi) {
      return;
    }
    int mid = min_val + static_cast<int>((static_cast<int64_t>(max_val) - min_val) / 2);
    auto go_left = [mid](int v) { return v <= mid; };
    for (auto it = lo; it != hi; ++it) {
      assert(min_val <= *it && *it <= max_val);
      b.push_back(b.back() + (go_left(*it) ? 1 : 0));
    }
    if (min_val == max_val) {
      return;
    }
    auto pivot = std::stable_partition(lo, hi, go_left);
    left.reset(new WaveletTree(lo, pivot, min_val, mid));
    right.reset(new WaveletTree(pivot, hi, mid + 1, max_val));
  }

  // 1-based positions [lo, hi] for the recursion below.
  int kth(int lo, int hi, int k) const {
    if (min_val == max_val) {
      return min_val;
    }
    int in_left = b[hi] - b[lo - 1];
    if (k <= in_left) {
      return left->kth(b[lo - 1] + 1, b[hi], k);
    }
    return right->kth(lo - b[lo - 1], hi - b[hi], k - in_left);
  }

  int leq(int lo, int hi, int x) const {
    if (lo > hi || x < min_val) {
      return 0;
    }
    if (max_val <= x) {
      return hi - lo + 1;
    }
    return left->leq(b[lo - 1] + 1, b[hi], x) + right->leq(lo - b[lo - 1], hi - b[hi], x);
  }

 public:
  WaveletTree(std::vector<int> a, int min_val, int max_val)
      : WaveletTree(a.begin(), a.end(), min_val, max_val) {}

  int size() const { return static_cast<int>(b.size()) - 1; }

  int kth_smallest(int lo, int hi, int k) const {
    assert(0 <= lo && lo <= hi && hi < size());
    assert(1 <= k && k <= hi - lo + 1);
    return kth(lo + 1, hi + 1, k);
  }

  int count_leq(int lo, int hi, int x) const {
    assert(0 <= lo && lo <= hi && hi < size());
    return leq(lo + 1, hi + 1, x);
  }

  int count_in(int lo, int hi, int x, int y) const {
    assert(0 <= lo && lo <= hi && hi < size());
    assert(x <= y);
    return leq(lo + 1, hi + 1, y) - (x <= min_val ? 0 : leq(lo + 1, hi + 1, x - 1));
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{5, 2, 8, 6, 1, 9, 3};
  WaveletTree t(a, 1, 9);

  assert(t.size() == 7);
  assert(t.kth_smallest(0, 6, 1) == 1);  // Minimum of the whole array.
  assert(t.kth_smallest(0, 6, 4) == 5);  // Median of the whole array.
  assert(t.kth_smallest(2, 4, 2) == 6);  // {8, 6, 1} -> 6 is 2nd smallest.

  assert(t.count_leq(0, 6, 5) == 4);    // 5, 2, 1, 3
  assert(t.count_in(0, 6, 3, 8) == 4);  // 5, 8, 6, 3
  assert(t.count_in(1, 5, 2, 6) == 2);  // 2, 6

  vector<int> extremes{INT_MIN, 0, INT_MAX};
  WaveletTree extreme_tree(extremes, INT_MIN, INT_MAX);
  assert(extreme_tree.count_in(0, 2, INT_MIN, 0) == 2);
  return 0;
}
