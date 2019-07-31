/*

Given two strings, determine their longest common substring (i.e. consecutive
subsequence) using dynamic programming.

Time Complexity:
- O(n*m) per call to longest_common_substring(s1, s2), where n and m are the
  lengths of s1 and s2, respectively.

Space Complexity:
- O(min(n, m)) auxiliary heap space, where n and m are the lengths of s1 and
  s2, respectively.

*/

#include <string>
#include <vector>
using std::string;

string longest_common_substring(const string &s1, const string &s2) {
  int n = s1.size(), m = s2.size();
  if (n == 0 || m == 0) {
    return "";
  }
  if (n < m) {
    return longest_common_substring(s2, s1);
  }
  std::vector<int> curr(m), prev(m);
  int pos = 0, len = 0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      if (s1[i] == s2[j]) {
        curr[j] = (i > 0 && j > 0) ? prev[j - 1] + 1 : 1;
        if (len < curr[j]) {
          len = curr[j];
          pos = i - curr[j] + 1;
        }
      } else {
        curr[j] = 0;
      }
    }
    curr.swap(prev);
  }
  return s1.substr(pos, len);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(longest_common_substring("bbbabca", "aababcd") == "babc");
  return 0;
}
