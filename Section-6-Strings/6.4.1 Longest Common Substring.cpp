/*

A substring is a consecutive part of a longer string (e.g. "ABC" is
a substring of "ABCDE" but "ABD" is not). Using dynamic programming,
determine the longest string which is a substring common to any two
input strings.

Time Complexity: O(n * m) where n and m are the lengths of the two
input strings, respectively.

Space Complexity: O(min(n, m)) auxiliary.

*/

#include <string>

std::string longest_common_substring
(const std::string & s1, const std::string & s2) {
  if (s1.empty() || s2.empty()) return "";
  if (s1.size() < s2.size())
    return longest_common_substring(s2, s1);
  int * A = new int[s2.size()];
  int * B = new int[s2.size()];
  int startpos = 0, maxlen = 0;
  for (int i = 0; i < (int)s1.size(); i++) {
    for (int j = 0; j < (int)s2.size(); j++) {
      if (s1[i] == s2[j]) {
        A[j] = (i > 0 && j > 0) ? 1 + B[j - 1] : 1;
        if (maxlen < A[j]) {
          maxlen = A[j];
          startpos = i - A[j] + 1;
        }
      } else {
        A[j] = 0;
      }
    }
    int * temp = A;
    A = B;
    B = temp;
  }
  delete[] A;
  delete[] B;
  return s1.substr(startpos, maxlen);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(longest_common_substring("bbbabca", "aababcd") == "babc");
  return 0;
}
