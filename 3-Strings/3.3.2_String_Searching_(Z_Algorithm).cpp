/*

Given a single string (needle) and a single text (haystack) to be searched, determine the first
position in which the needle occurs within the haystack in linear time using the Z algorithm. In
comparison, `std::string::find` runs in quadratic time.

The `find()` function below calls the Z algorithm on the concatenation of `needle` and `haystack`,
separated by a sentinel value that is guaranteed not to collide with any byte in either input.

- `z_array(s)` constructs the Z array for a string `needle` that can be used for string searching.
  The Z array on an input string `s` is an array `z` where `z[i]` is the length of the longest
  substring starting from `s[i]` which is also a prefix of `s`.
- `find(haystack, needle)` returns the first position that `needle` occurs in `haystack`, or
  `std::string::npos` if it cannot be found. Note that the function can be modified to return all
  matches by simply letting the loop run and storing the results instead of returning early.

Time Complexity:
- O(n) per call to `z_array(s)`, where $n$ is the length of `s`.
- O(n + m) per call to `find(haystack, needle)`, where $n$ is the length of `haystack` and $m$ is
  the length of `needle`.

Space Complexity:
- O(n) auxiliary heap space for `z_array(s)`, where $n$ is the length of `s`.
- O(n + m) auxiliary heap space for `find(haystack, needle)` where $n$ is the length of `haystack`
  and $m$ is the length of `needle`.

*/

#include <algorithm>
#include <string>
#include <vector>
using std::string;

template<class Seq>
std::vector<int> z_array(const Seq &s) {
  std::vector<int> z(s.size());
  for (int i = 1, l = 0, r = 0; i < static_cast<int>(z.size()); i++) {
    if (i <= r) {
      z[i] = std::min(r - i + 1, z[i - l]);
    }
    while (i + z[i] < static_cast<int>(z.size()) && s[z[i]] == s[i + z[i]]) {
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
  if (needle.empty()) {
    return 0;
  }
  std::vector<int> s;
  s.reserve(needle.size() + haystack.size() + 1);
  for (unsigned char c : needle) {
    s.push_back(c + 1);
  }
  s.push_back(0);
  for (unsigned char c : haystack) {
    s.push_back(c + 1);
  }
  auto z = z_array(s);
  int m = static_cast<int>(needle.size());
  for (int i = m + 1; i < static_cast<int>(z.size()); i++) {
    if (z[i] == m) {
      return i - m - 1;
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
