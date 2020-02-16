/*

Given two strings, determine their longest common subsequence. A subsequence is
a string that can be derived from the original string by deleting some elements
without changing the order of the remaining elements (e.g. "ACE" is a
subsequence of "ABCDE", but "BAE" is not).

- longest_common_subsequence(s1, s2) returns the longest common subsequence of
  strings s1 and s2 using a classic dynamic programming approach. This
  implementation computes dp[i][j] (the length of the longest common subsequence
  for the length i prefix of s1 and the length j prefix of s2) before following
  the path backwards to construct the answer.
- hirschberg_lcs(s1, s2) returns the longest common subsequence of strings s1
  and s2 using the more memory efficient Hirschberg's algorithm.

Time Complexity:
- O(n*m) per call to longest_common_subsequence(s1, s2) as well as
  hirschberg_lcs(s1, s2), where n and m are the lengths of s1 and s2,
  respectively.

Space Complexity:
- O(n*m) auxiliary heap space for longest_common_subsequence(s1, s2), where n
  and m are the lengths of s1 and s2, respectively.
- O(log max(n, m)) auxiliary stack space and O(min(n, m)) auxiliary heap space
  for hirschberg_lcs(s1, s2), where n and m are the lengths of s1 and s2,
  respectively.

*/

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
using std::string;

string longest_common_subsequence(const string &s1, const string &s2) {
  int n = s1.size(), m = s2.size();
  std::vector<std::vector<int> > dp(n + 1, std::vector<int>(m + 1, 0));
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= m; j++) {
      if (s1[i - 1] == s2[j - 1]) {
        dp[i][j] = dp[i - 1][j - 1] + 1;
      } else {
        dp[i][j] = std::max(dp[i][j - 1], dp[i - 1][j]);
      }
    }
  }
  string res;
  for (int i = n, j = m; i > 0 && j > 0; ) {
    if (s1[i - 1] == s2[j - 1]) {
      res += s1[i - 1];
      i--;
      j--;
    } else if (dp[i - 1][j] >= dp[i][j - 1]) {
      i--;
    } else {
      j--;
    }
  }
  std::reverse(res.begin(), res.end());
  return res;
}

template<class It>
std::vector<int> lcs_len(It lo1, It hi1, It lo2, It hi2) {
  std::vector<int> res(std::distance(lo2, hi2) + 1), prev(res);
  for (It it1 = lo1; it1 != hi1; ++it1) {
    res.swap(prev);
    int i = 0;
    for (It it2 = lo2; it2 != hi2; ++it2) {
      res[i + 1] = (*it1 == *it2) ? prev[i] + 1 : std::max(res[i], prev[i + 1]);
      i++;
    }
  }
  return res;
}

template<class It>
void hirschberg_rec(It lo1, It hi1, It lo2, It hi2, string *res) {
  if (lo1 == hi1) {
    return;
  }
  if (lo1 + 1 == hi1) {
    if (std::find(lo2, hi2, *lo1) != hi2) {
      *res += *lo1;
    }
    return;
  }
  It mid1 = lo1 + (hi1 - lo1)/2;
  std::reverse_iterator<It> rlo1(hi1), rmid1(mid1), rlo2(hi2), rhi2(lo2);
  std::vector<int> fwd = lcs_len(lo1, mid1, lo2, hi2);
  std::vector<int> rev = lcs_len(rlo1, rmid1, rlo2, rhi2);
  It mid2 = lo2;
  int maxlen = -1;
  for (int i = 0, j = (int)rev.size() - 1; i < (int)fwd.size(); i++, j--) {
    if (fwd[i] + rev[j] > maxlen) {
      maxlen = fwd[i] + rev[j];
      mid2 = lo2 + i;
    }
  }
  hirschberg_rec(lo1, mid1, lo2, mid2, res);
  hirschberg_rec(mid1, hi1, mid2, hi2, res);
}

string hirschberg_lcs(const string &s1, const string &s2) {
  if (s1.size() < s2.size()) {
    return hirschberg_lcs(s2, s1);
  }
  string res;
  hirschberg_rec(s1.begin(), s1.end(), s2.begin(), s2.end(), &res);
  return res;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(longest_common_subsequence("xmjyauz", "mzjawxu") == "mjau");
  assert(hirschberg_lcs("xmjyauz", "mzjawxu") == "mjau");
  return 0;
}
