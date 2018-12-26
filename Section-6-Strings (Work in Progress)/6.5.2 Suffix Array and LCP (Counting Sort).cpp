/*

Given a string s, a suffix array is the array of the smallest starting positions
for the sorted suffices of s. That is, the i-th position of the suffix array
stores the starting position of the i-th lexicographically smallest suffix of s.
For examples, s = "cab" has the suffices "cab", "ab", and "b". When sorted, the
indices of the suffixes are "ab", "b", and "cab", so the suffix array (assuming
zero-based indices) is [1, 2, 0].

For a string s of length n the longest common prefix (LCP) array of length n - 1
stores the lengths of the longest common prefixes between all pairs of
lexicographically adjacent suffices in s. For example, "baa" has the sorted
suffices "a", "aa", and "baa", with an LCP array of [1, 0].

- suffix_array(s) constructs a suffix array from the given string s using the
  original Manber-Myers gap partitioning algorithm with a counting sort instead
  of a comparison-based sort to reduce the running time to O(n log n).
- get_sa() returns the constructed suffix array.
- get_lcp() returns the corresponding LCP array for the suffix array.
- find(needle) returns one position that needle occurs in s (not necessarily the
  first), or std::string::npos if it cannot be found. For a needle of length m,
  this implementation uses an O(m log n) binary search, but can be optimized to
  O(m + log n) by first computing the LCP-LR array using the LCP array.

Time Complexity:
- O(n log n) per call to the constructor, where n is the length of s.
- O(1) per call to get_sa().
- O(n) per call to get_lcp(), where n is the length of s.
- O(m log n) per call to find(needle), where m is the length of needle and n is
  the length of s.

Space Complexity:
- O(n) auxiliary for storage of the suffix and LCP arrays, where n is the length
  of s.
- O(n) auxiliary heap space for the constructor.
- O(1) auxiliary space for all other operations.

*/

#include <algorithm>
#include <string>
#include <utility>
#include <vector>
using std::string;

class suffix_array {
  struct comp {
    const string &s;

    comp(const string &s) : s(s) {}

    bool operator()(int i, int j) {
      return s[i] < s[j];
    }
  };

  string s;
  std::vector<int> sa, rank;

 public:
  suffix_array(const string &s) : s(s), sa(s.size()), rank(s.size()) {
    int n = s.size();
    for (int i = 0; i < n; i++) {
      sa[i] = n - 1 - i;
      rank[i] = (int)s[i];
    }
    std::stable_sort(sa.begin(), sa.end(), comp(s));
    for (int gap = 1; gap < n; gap *= 2) {
      std::vector<int> prev_rank(rank), prev_sa(sa), cnt(n);
      for (int i = 0; i < n; i++) {
        cnt[i] = i;
      }
      for (int i = 0; i < n; i++) {
        rank[sa[i]] = (i > 0 && prev_rank[sa[i - 1]] == prev_rank[sa[i]] &&
                       sa[i - 1] + gap < n &&
                       prev_rank[sa[i - 1] + gap/2] == prev_rank[sa[i] + gap/2])
                           ? rank[sa[i - 1]] : i;
      }
      for (int i = 0; i < n; i++) {
        int s1 = prev_sa[i] - gap;
        if (s1 >= 0) {
          sa[cnt[rank[s1]]++] = s1;
        }
      }
    }
  }

  std::vector<int> get_sa() {
    return sa;
  }

  std::vector<int> get_lcp() {
    int n = s.size();
    std::vector<int> lcp(n - 1);
    for (int i = 0, k = 0; i < n; i++) {
      if (rank[i] < n - 1) {
        int j = sa[rank[i] + 1];
        while (std::max(i, j) + k < n && s[i + k] == s[j + k]) {
          k++;
        }
        lcp[rank[i]] = k;
        if (k > 0) {
          k--;
        }
      }
    }
    return lcp;
  }

  size_t find(const string &needle) {
    int lo = 0, hi = (int)s.size() - 1;
    while (lo <= hi) {
      int mid = lo + (hi - lo)/2;
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
  suffix_array sa("banana");
  vector<int> sarr = sa.get_sa(), lcp = sa.get_lcp();
  int sarr_expected[] = {5, 3, 1, 0, 4, 2};
  int lcp_expected[] = {1, 3, 0, 0, 2};
  assert(equal(sarr.begin(), sarr.end(), sarr_expected));
  assert(equal(lcp.begin(), lcp.end(), lcp_expected));
  assert(sa.find("ana") == 1);
  assert(sa.find("x") == string::npos);
  return 0;
}
