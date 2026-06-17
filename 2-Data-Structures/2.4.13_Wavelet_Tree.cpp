/*

A wavelet tree is a static structure over an integer array with values in a known range
[`min_val`, `max_val`]. The root splits the value range at its midpoint, recording for every prefix
of the array how many of its elements belong to the lower value half, then stably partitions the
array so that those elements form the left child and the rest form the right child. Recursing on
each half builds a balanced tree of height O(log \sigma), where $\sigma$ is the size of the value
range.

The recorded prefix counts let a query at any node translate a range of array positions into the
corresponding range of positions in either child, so a single root-to-leaf descent answers order
statistics and rank queries. This is strictly more powerful than a merge sort tree: besides counting
values below a threshold, it reports the $k$-th smallest value of a range in $O(\log \sigma)$ time.

All position ranges are inclusive [`lo`, `hi`] with 0-based indices. If the values are large or
sparse, compress them to a small contiguous range first.

- `WaveletTree(a, min_val, max_val)` builds the tree over the array `a`, whose values must all lie
  in [`min_val`, `max_val`].
- `kth_smallest(lo, hi, k)` returns the `k`-th smallest value among positions [`lo`, `hi`], where
  `k` is 1-based (so `k == 1` returns the minimum).
- `count_leq(lo, hi, x)` returns the number of positions `i` in [`lo`, `hi`] with `a[i]` $\leq$ `x`.
- `count_in(lo, hi, x, y)` returns the number of positions `i` in [`lo`, `hi`] with
  `x` $\leq$ `a[i]` $\leq$ `y`.

Time Complexity:
- O(n log \sigma) per call to the constructor, where $n$ is the size of the array and $\sigma$ is
  the size of the value range.
- O(log \sigma) per call to `kth_smallest()`, `count_leq()`, and `count_in()`.

Space Complexity:
- O(n log \sigma) for storage of the tree.
- O(log \sigma) auxiliary stack space per query.

*/

#include <algorithm>
#include <memory>
#include <vector>

class WaveletTree {
  int min_val, max_val;  // The value range this node covers.
  std::unique_ptr<WaveletTree> left, right;
  std::vector<int> b;  // b[i] = number of the first i elements that go to the left child.

  WaveletTree(std::vector<int>::iterator lo, std::vector<int>::iterator hi, int x, int y)
      : min_val(x), max_val(y) {
    if (lo >= hi) {
      return;
    }
    int mid = min_val + (max_val - min_val) / 2;
    auto go_left = [mid](int v) { return v <= mid; };
    b.reserve((hi - lo) + 1);
    b.push_back(0);
    for (auto it = lo; it != hi; ++it) {
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

  int kth_smallest(int lo, int hi, int k) const { return kth(lo + 1, hi + 1, k); }
  int count_leq(int lo, int hi, int x) const { return leq(lo + 1, hi + 1, x); }

  int count_in(int lo, int hi, int x, int y) const {
    return leq(lo + 1, hi + 1, y) - leq(lo + 1, hi + 1, x - 1);
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{5, 2, 8, 6, 1, 9, 3};
  WaveletTree t(a, 1, 9);

  assert(t.kth_smallest(0, 6, 1) == 1);  // Minimum of the whole array.
  assert(t.kth_smallest(0, 6, 4) == 5);  // Median of the whole array.
  assert(t.kth_smallest(2, 4, 2) == 6);  // {8, 6, 1} -> 6 is 2nd smallest.

  assert(t.count_leq(0, 6, 5) == 4);    // 5, 2, 1, 3
  assert(t.count_in(0, 6, 3, 8) == 4);  // 5, 8, 6, 3
  assert(t.count_in(1, 5, 2, 6) == 2);  // 2, 6
  return 0;
}
