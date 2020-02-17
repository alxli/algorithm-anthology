/*

Given a single string (needle) and a single text (haystack) to be searched,
determine the first position in which the needle occurs within the haystack in
linear time using the Z algorithm. In comparison, std::string::find runs in
quadratic time.

The find function below calls the Z algorithm on the concatenation of the needle
and the haystack, separated by a sentinel character (in this case '\0'), which
should be chosen such that it does not occur within either of the input strings.

- z_array(s) constructs the Z array for a string needle that can be used for
  string searching. The Z array on an input string s is an array z where z[i] is
  the length of the longest substring starting from s[i] which is also a prefix
  of s.
- find(needle, haystack) returns the first position that needle occurs in
  haystack, or std::string::npos if it cannot be found. Note that the function
  can be modified to return all matches by simply letting the loop run and
  storing the results instead of returning early.

Time Complexity:
- O(n) per call to z_array(s), where n is the length of s.
- O(n + m) per call to find(haystack, needle), where n is the length of haystack
  and m is the length of needle.

Space Complexity:
- O(n) auxiliary heap space for z_array(s), where n is the length of s.
- O(n + m) auxiliary heap space for find(haystack, needle) where n is the length
  of haystack and m is the length of needle.

*/

#include <algorithm>
#include <string>
#include <vector>
using std::string;

std::vector<int> z_array(const string &s) {
  std::vector<int> z(s.size());
  for (int i = 1, l = 0, r = 0; i < (int)z.size(); i++) {
    if (i <= r) {
      z[i] = std::min(r - i + 1, z[i - l]);
    }
    while (i + z[i] < (int)z.size() && s[z[i]] == s[i + z[i]]) {
      z[i]++;
    }
    if (r < i + z[i] - 1) {
      l = i;
      r = i + z[i] - 1;
    }
  }
  return z;
}

size_t find(const string &haystack, const string &needle) {
  std::vector<int> z = z_array(needle + '\0' + haystack);
  for (int i = (int)needle.size() + 1; i < (int)z.size(); i++) {
    if (z[i] == (int)needle.size()) {
      return i - (int)needle.size() - 1;
    }
  }
  return string::npos;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(15 == find("ABC ABCDAB ABCDABCDABDE", "ABCDABD"));
  return 0;
}
