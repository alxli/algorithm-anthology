/*

Given two strings s1 and s2, the edit distance between them is the
minimum number of operations required to transform s1 into s2,
where each operation can be any one of the following:
  - insert a letter anywhere into the current string
  - delete any letter from the current string
  - replace any letter of the current string with any other letter

Time Complexity: O(n * m) where n and m are the lengths of the two
input strings, respectively.

Space Complexity: O(n * m) auxiliary.

*/

#include <algorithm>
#include <string>
#include <vector>

int edit_distance(const std::string & s1, const std::string & s2) {
  int n = s1.size(), m = s2.size();
  std::vector< std::vector<int> > dp;
  dp.resize(n + 1, std::vector<int>(m + 1, 0));
  for (int i = 0; i <= n; i++) dp[i][0] = i;
  for (int j = 0; j <= m; j++) dp[0][j] = j;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      if (s1[i] == s2[j]) {
        dp[i + 1][j + 1] = dp[i][j];
      } else {
        dp[i + 1][j + 1] = 1 + std::min(dp[i][j],        //replace
                               std::min(dp[i + 1][j],    //insert
                                        dp[i][j + 1]));  //delete
      }
    }
  }
  return dp[n][m];
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(edit_distance("abxdef", "abcdefg") == 2);
  return 0;
}
