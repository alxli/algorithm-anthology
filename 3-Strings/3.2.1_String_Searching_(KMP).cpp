/*

Given a single string (needle) and subsequent queries of texts (haystacks) to be
searched, determine the first positions in which the needle occurs within the
given haystacks in linear time using the Knuth-Morris-Pratt algorithm. In
comparison, std::string::find runs in quadratic time.

- kmp(needle) constructs the partial match table for a string needle that is to
  be searched for subsequently in haystack queries.
- find_in(haystack) returns the first position that needle occurs in haystack,
  or std::string::npos if it cannot be found. Note that the function can be
  modified to return all matches by simply letting the loop run and storing
  the results instead of returning early.

Time Complexity:
- O(m) per call to the constructor, where m is the length of needle.
- O(n) per call to find_in(haystack), where n is the length of haystack.

Space Complexity:
- O(m) for storage of the partial match table, where m is the length of needle.
- O(1) auxiliary space per call to find_in(haystack).

*/

#include <string>
#include <vector>
using std::string;

class kmp {
  string needle;
  std::vector<int> table;

 public:
  kmp(const string &needle) : needle(needle), table(needle.size()) {
    for (int i = 1, j = 0; i < (int)needle.size(); i++) {
      while (j > 0 && needle[i] != needle[j]) {
        j = table[j - 1];
      }
      if (needle[i] == needle[j]) {
        j++;
      }
      table[i] = j;
    }
  }

  size_t find_in(const string &haystack) {
    int m = needle.size();
    if (m == 0) {
      return 0;
    }
    for (int i = 0, j = 0; i < (int)haystack.size(); i++) {
      while (j > 0 && needle[j] != haystack[i]) {
        j = table[j - 1];
      }
      if (needle[j] == haystack[i]) {
        j++;
      }
      if (j == m) {
        return i + 1 - m;
      }
    }
    return string::npos;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(15 == kmp("ABCDABD").find_in("ABC ABCDAB ABCDABCDABDE"));
  return 0;
}
