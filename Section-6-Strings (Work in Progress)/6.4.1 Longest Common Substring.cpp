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
  if (s1.empty() || s2.empty()) {
    return "";
  }
  if (s1.size() < s2.size()) {
    return longest_common_substring(s2, s1);
  }
  std::vector<int> A(s2.size()), B(s2.size());
  int pos = 0, len = 0;
  for (int i = 0; i < (int)s1.size(); i++) {
    for (int j = 0; j < (int)s2.size(); j++) {
      if (s1[i] == s2[j]) {
        A[j] = (i > 0 && j > 0) ? B[j - 1] + 1 : 1;
        if (len < A[j]) {
          len = A[j];
          pos = i - A[j] + 1;
        }
      } else {
        A[j] = 0;
      }
    }
    A.swap(B);
  }
  return s1.substr(pos, len);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(longest_common_substring("bbbabca", "aababcd") == "babc");
  return 0;
}
