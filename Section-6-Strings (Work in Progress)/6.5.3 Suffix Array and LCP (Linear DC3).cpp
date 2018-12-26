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
  linear time DC3/skew algorithm by Karkkainen & Sanders (2003) with radix sort.
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
  static bool leq(int a1, int a2, int b1, int b2) {
    return (a1 < b1) || (a1 == b1 && a2 <= b2);
  }

  static bool leq(int a1, int a2, int a3, int b1, int b2, int b3) {
    return (a1 < b1) || (a1 == b1 && leq(a2, a3, b2, b3));
  }

  template<class It>
  static void radix_pass(It a, It b, It r, int n, int K) {
    std::vector<int> cnt(K + 1);
    for (int i = 0; i < n; i++) {
      cnt[r[a[i]]]++;
    }
    for (int i = 1; i <= K; i++) {
      cnt[i] += cnt[i - 1];
    }
    for (int i = n - 1; i >= 0; i--) {
      b[--cnt[r[a[i]]]] = a[i];
    }
  }

  template<class It>
  static void suffix_array_dc3(It s, It sa, int n, int K) {
    int n0 = (n + 2)/3, n1 = (n + 1)/3, n2 = n/3, n02 = n0 + n2;
    std::vector<int> s12(n02 + 3), sa12(n02 + 3), s0(n0), sa0(n0);
    s12[n02] = s12[n02 + 1] = s12[n02 + 2] = 0;
    sa12[n02] = sa12[n02 + 1] = sa12[n02 + 2] = 0;
    for (int i = 0, j = 0; i < n + n0 - n1; i++) {
      if (i % 3 != 0) {
        s12[j++] = i;
      }
    }
    radix_pass(s12.begin(), sa12.begin(), s + 2, n02, K);
    radix_pass(sa12.begin(), s12.begin(), s + 1, n02, K);
    radix_pass(s12.begin(), sa12.begin(), s, n02, K);
    int name = 0, c0 = -1, c1 = -1, c2 = -1;
    for (int i = 0; i < n02; i++) {
      if (s[sa12[i]] != c0 || s[sa12[i] + 1] != c1 || s[sa12[i] + 2] != c2) {
        name++;
        c0 = s[sa12[i]];
        c1 = s[sa12[i] + 1];
        c2 = s[sa12[i] + 2];
      }
      (sa12[i] % 3 == 1 ? s12[sa12[i]/3] : s12[sa12[i]/3 + n0]) = name;
    }
    if (name < n02) {
      suffix_array_dc3(s12.begin(), sa12.begin(), n02, name);
      for (int i = 0; i < n02; i++) {
        s12[sa12[i]] = i + 1;
      }
    } else {
      for (int i = 0; i < n02; i++) {
        sa12[s12[i] - 1] = i;
      }
    }
    for (int i = 0, j = 0; i < n02; i++) {
      if (sa12[i] < n0) {
        s0[j++] = 3*sa12[i];
      }
    }
    radix_pass(s0.begin(), sa0.begin(), s, n0, K);
    for (int p = 0, t = n0 - n1, k = 0; k < n; k++) {
      int i = (sa12[t] < n0) ? 3*sa12[t] + 1 : 3*(sa12[t] - n0) + 2, j = sa0[p];
      if (sa12[t] < n0 ? leq(s[i], s12[sa12[t] + n0],s[j], s12[j/3])
                       : leq(s[i], s[i + 1], s12[sa12[t] - n0 + 1], s[j],
                             s[j + 1], s12[j / 3 + n0])) {
        sa[k] = i;
        if (++t == n02) {
          for (k++; p < n0; p++, k++) {
            sa[k] = sa0[p];
          }
        }
      } else {
        sa[k] = j;
        if (++p == n0) {
          for (k++; t < n02; t++, k++) {
            sa[k] = (sa12[t] < n0) ? 3*sa12[t] + 1 : 3*(sa12[t] - n0) + 2;
          }
        }
      }
    }
  }

  string s;
  std::vector<int> sa;

 public:
  suffix_array(const string &s) : s(s), sa(s.size() + 1) {
    int n = s.size();
    std::vector<int> scopy(s.begin(), s.end());
    scopy.resize(n + 3);
    suffix_array_dc3(scopy.begin(), sa.begin(), n + 1, 255);
    sa.erase(sa.begin());
  }

  std::vector<int> get_sa() {
    return sa;
  }

  std::vector<int> get_lcp() {
    int n = s.size();
    std::vector<int> rank(n), lcp(n - 1);
    for (int i = 0; i < n; i++) {
      rank[sa[i]] = i;
    }
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
