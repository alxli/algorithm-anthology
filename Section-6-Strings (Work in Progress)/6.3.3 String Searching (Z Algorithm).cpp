/*

Given an text and a pattern to be searched for within the text,
determine the positions of all patterns within the text. This
is as efficient as KMP, but does so through computing the
"Z function." For a string S, Z[i] stores the length of the longest
substring starting from S[i] which is also a prefix of S, i.e. the
maximum k such that S[j] = S[i + j] for all 0 <= j < k.

Time Complexity: O(n + m) where n is the length of the text
and m is the length of the pattern.

Space Complexity: O(m) auxiliary on the length of the pattern.

*/

#include <algorithm>
#include <string>
#include <vector>

std::vector<int> z_function(const std::string & s) {
  std::vector<int> z(s.size());
  for (int i = 1, l = 0, r = 0; i < (int)z.size(); i++) {
    if (i <= r)
      z[i] = std::min(r - i + 1, z[i - l]);
    while (i + z[i] < (int)z.size() && s[z[i]] == s[i + z[i]])
      z[i]++;
    if (r < i + z[i] - 1) {
      l = i;
      r = i + z[i] - 1;
    }
  }
  return z;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  string text = "abcabaaaababab";
  string pattern = "aba";
  vector<int> z = z_function(pattern + "$" + text);
  for (int i = (int)pattern.size() + 1; i < (int)z.size(); i++) {
    if (z[i] == (int)pattern.size())
      cout << "Pattern found starting at index "
           << (i - (int)pattern.size() - 1) << "." << endl;
  }
  return 0;
}
