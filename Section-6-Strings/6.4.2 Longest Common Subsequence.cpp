/*

A subsequence is a sequence that can be derived from another sequence
by deleting some elements without changing the order of the remaining
elements (e.g. "ACE" is a subsequence of "ABCDE", but "BAE" is not).
Using dynamic programming, determine the longest string which
is a subsequence common to any two input strings.

In addition, the shortest common supersequence between two strings is
a closely related problem, which involves finding the shortest string
which has both input strings as subsequences (e.g. "ABBC" and "BCB" has
the shortest common supersequence of "ABBCB"). The answer is simply:
  (sum of lengths of s1 and s2) - (length of LCS of s1 and s2)

Time Complexity: O(n * m) where n and m are the lengths of the two
input strings, respectively.

Space Complexity: O(n * m) auxiliary.

*/

#include <string>
#include <vector>

std::string longest_common_subsequence
(const std::string & s1, const std::string & s2) {
  int n = s1.size(), m = s2.size();
  std::vector< std::vector<int> > dp;
  dp.resize(n + 1, std::vector<int>(m + 1, 0));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      if (s1[i] == s2[j]) {
        dp[i + 1][j + 1] = dp[i][j] + 1;
      } else if (dp[i + 1][j] > dp[i][j + 1]) {
        dp[i + 1][j + 1] = dp[i + 1][j];
      } else {
        dp[i + 1][j + 1] = dp[i][j + 1];
      }
    }
  }
  std::string ret;
  for (int i = n, j = m; i > 0 && j > 0; ) {
    if (s1[i - 1] == s2[j - 1]) {
      ret = s1[i - 1] + ret;
      i--;
      j--;
    } else if (dp[i - 1][j] < dp[i][j - 1]) {
      j--;
    } else {
      i--;
    }
  }
  return ret;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(longest_common_subsequence("xmjyauz", "mzjawxu") == "mjau");
  return 0;
}
