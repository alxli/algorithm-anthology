/*

Given a string `s`, a suffix array is the array of the smallest starting positions for the sorted
suffixes of `s`. That is, the $i$-th position of the suffix array stores the starting position of
the $i$-th lexicographically smallest suffix of `s`. For example, `s = "cab"` has the suffixes
`"cab"`, `"ab"`, and `"b"`. When sorted, the indices of the suffixes are `"ab"`, `"b"`, and `"cab"`,
so the suffix array (assuming zero-based indices) is `[1, 2, 0]`.

For a string `s` of length $n$, the longest common prefix (LCP) array of length $n - 1$ stores the
lengths of the longest common prefixes between all pairs of lexicographically adjacent suffixes in
`s`. For example, `"baa"` has the sorted suffixes `"a"`, `"aa"`, and `"baa"`, with an LCP array of
`[1, 0]`.

- `SuffixArrayManberMyers(s)` constructs a suffix array from the given string `s` using the original
  Manber-Myers doubling algorithm with a comparison-based sort.
- `get_sa()` returns the constructed suffix array.
- `get_lcp()` returns the corresponding LCP array for the suffix array.
- `find(needle)` returns one position that `needle` occurs in `s` (not necessarily the first), or
  `std::string::npos` if it cannot be found. For a `needle` of length $m$, this implementation uses
  an O(m log n) binary search, but can be optimized to O(m + log n) by first computing the LCP-LR
  array using the LCP array.

Time Complexity:
- O(n log^2 n) per call to the constructor, where $n$ is the length of `s`.
- O(1) per call to `get_sa()`.
- O(n) per call to `get_lcp()`, where $n$ is the length of `s`.
- O(m log n) per call to `find(needle)`, where $m$ is the length of `needle` and $n$ is the length
  of `s`.

Space Complexity:
- O(n) auxiliary for storage of the suffix and LCP arrays, where $n$ is the length of `s`.
- O(n) auxiliary heap space for the constructor.
- O(1) auxiliary space for all other operations.

*/

#include <algorithm>
#include <numeric>
#include <string>
#include <utility>
#include <vector>
using std::string;

class SuffixArrayManberMyers {
  string s;
  std::vector<int> sa, rk;

 public:
  explicit SuffixArrayManberMyers(const string &s) : s(s), sa(s.size()), rk(s.size()) {
    int n = s.size();
    std::iota(sa.begin(), sa.end(), 0);
    for (int i = 0; i < n; i++) {
      rk[i] = static_cast<unsigned char>(s[i]);
    }
    std::vector<std::pair<int, int>> rk2(n);
    for (int gap = 1; gap < n; gap *= 2) {
      for (int i = 0; i < n; i++) {
        rk2[i] = {rk[i], i + gap < n ? rk[i + gap] + 1 : 0};
      }
      std::sort(sa.begin(), sa.end(), [&](int i, int j) { return rk2[i] < rk2[j]; });
      for (int i = 0; i < n; i++) {
        rk[sa[i]] = (i > 0 && rk2[sa[i - 1]] == rk2[sa[i]]) ? rk[sa[i - 1]] : i;
      }
    }
  }

  const std::vector<int> &get_sa() const { return sa; }

  std::vector<int> get_lcp() const {
    int n = s.size();
    if (n == 0) {
      return {};  // Avoid constructing a vector of size (size_t)(-1).
    }
    std::vector<int> lcp(n - 1);
    for (int i = 0, k = 0; i < n; i++) {
      if (rk[i] < n - 1) {
        int j = sa[rk[i] + 1];
        while (std::max(i, j) + k < n && s[i + k] == s[j + k]) {
          k++;
        }
        lcp[rk[i]] = k;
        if (k > 0) {
          k--;
        }
      }
    }
    return lcp;
  }

  size_t find(const string &needle) {
    if (needle.empty()) {
      return 0;
    }
    int lo = 0, hi = static_cast<int>(s.size()) - 1;
    while (lo <= hi) {
      int mid = lo + (hi - lo) / 2;
      int cmp = s.compare(sa[mid], needle.size(), needle);
      if (cmp < 0) {
        lo = mid + 1;
      } else if (cmp > 0) {
        hi = mid - 1;
      } else {
        return sa[mid];
      }
    }
    return string::npos;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  SuffixArrayManberMyers sa("banana");
  vector<int> sarr = sa.get_sa(), lcp = sa.get_lcp();
  vector<int> sarr_expected{5, 3, 1, 0, 4, 2};
  vector<int> lcp_expected{1, 3, 0, 0, 2};
  assert(sarr == sarr_expected);
  assert(lcp == lcp_expected);
  assert(sa.find("ana") == 1);
  assert(sa.find("x") == string::npos);
  return 0;
}
