/*

Given an text and a pattern to be searched for within the text,
determine the first position in which the pattern occurs in
the text. The KMP algorithm is much faster than the naive,
quadratic time, string searching algorithm that is found in
string.find() in the C++ standard library.

KMP generates a table using a prefix function of the pattern.
Then, the precomputed table of the pattern can be used indefinitely
for any number of texts.

Time Complexity: O(n + m) where n is the length of the text
and m is the length of the pattern.

Space Complexity: O(m) auxiliary on the length of the pattern.

*/

#include <string>
#include <vector>

int find(const std::string & text, const std::string & pattern) {
  if (pattern.empty()) return 0;
  //generate table using pattern
  std::vector<int> p(pattern.size());
  for (int i = 0, j = p[0] = -1; i < (int)pattern.size(); ) {
    while (j >= 0 && pattern[i] != pattern[j])
      j = p[j];
    i++;
    j++;
    p[i] = (pattern[i] == pattern[j]) ? p[j] : j;
  }
  //use the precomputed table to search within text
  //the following can be repeated on many different texts
  for (int i = 0, j = 0; j < (int)text.size(); ) {
    while (i >= 0 && pattern[i] != text[j])
      i = p[i];
    i++;
    j++;
    if (i >= (int)pattern.size())
      return j - i;
  }
  return std::string::npos;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(15 == find("ABC ABCDAB ABCDABCDABDE", "ABCDABD"));
  return 0;
}
