/*

Solves basic palindrome dynamic programming problems on a string. The core interval state considers
substring $[`l`, `r`]$: if the two endpoints match, they can wrap an optimal answer for the inside
substring; otherwise, at least one endpoint must be skipped or inserted around the other side. Since
each state depends on shorter substrings, intervals are processed by increasing length.

The longest palindromic subsequence (LPS) is a maximum-length subsequence that reads the same
forward and backward. The minimum number of insertions needed to make a string a palindrome is the
number of characters outside an LPS, so it equals $n$ minus the LPS length.

- `longest_palindromic_subsequence(s)` returns one longest palindromic subsequence of `s`.
- `min_insertions_palindrome(s)` returns the minimum number of characters that must be inserted
  anywhere in `s` to make it a palindrome.

Time Complexity:
- O(n^2) per call to `longest_palindromic_subsequence(s)` and `min_insertions_palindrome(s)`, where
  $n$ is the length of `s`.

Space Complexity:
- O(n^2) auxiliary for `longest_palindromic_subsequence(s)`.
- O(n) auxiliary for `min_insertions_palindrome(s)`.

*/

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
using std::string;

string longest_palindromic_subsequence(const string &s) {
  int n = static_cast<int>(s.size());
  if (n == 0) {
    return "";
  }
  std::vector<std::vector<int>> dp(n, std::vector<int>(n, 0));
  for (int len = 1; len <= n; len++) {
    for (int l = 0, r = len - 1; r < n; l++, r++) {
      if (l == r) {
        dp[l][r] = 1;
      } else if (s[l] == s[r]) {
        dp[l][r] = 2 + (l + 1 <= r - 1 ? dp[l + 1][r - 1] : 0);
      } else {
        dp[l][r] = std::max(dp[l + 1][r], dp[l][r - 1]);
      }
    }
  }
  string left, right;
  for (int l = 0, r = n - 1; l <= r;) {
    if (l == r) {
      left += s[l];
      break;
    }
    if (s[l] == s[r] && dp[l][r] == 2 + (l + 1 <= r - 1 ? dp[l + 1][r - 1] : 0)) {
      left += s[l++];
      right += s[r--];
    } else if (dp[l + 1][r] >= dp[l][r - 1]) {
      l++;
    } else {
      r--;
    }
  }
  std::reverse(right.begin(), right.end());
  return left + right;
}

int min_insertions_palindrome(const string &s) {
  int n = static_cast<int>(s.size());
  std::vector<int> dp(n, 0);
  for (int l = n - 1; l >= 0; l--) {
    int inside = 0;
    for (int r = l + 1; r < n; r++) {
      int old = dp[r];
      if (s[l] == s[r]) {
        dp[r] = inside;
      } else {
        dp[r] = 1 + std::min(dp[r], dp[r - 1]);
      }
      inside = old;
    }
  }
  return n == 0 ? 0 : dp[n - 1];
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(longest_palindromic_subsequence("bbbab") == "bbbb");
  assert(longest_palindromic_subsequence("character") == "carac");
  assert(min_insertions_palindrome("leetcode") == 5);
  assert(min_insertions_palindrome("race") == 3);
  assert(min_insertions_palindrome("") == 0);
  return 0;
}
