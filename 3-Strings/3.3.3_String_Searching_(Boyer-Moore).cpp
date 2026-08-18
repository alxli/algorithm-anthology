/*

Given a single string (needle) and subsequent queries of texts (haystacks) to be searched, determine
the positions in which the needle occurs within the given haystacks using the Boyer-Moore algorithm.
Unlike KMP and the Z algorithm, which compare left to right and inspect every character of the
haystack, Boyer-Moore aligns the needle and compares right to left, which lets a single mismatch
prove that many alignments are impossible. Most alignments are then skipped entirely, so a long
needle over a large alphabet is typically matched in sublinear time.

Two precomputed rules propose a shift, and the algorithm takes the larger of the two.

The bad character rule looks at the haystack character that caused the mismatch. If it occurs in the
needle, slide the needle so that its last occurrence lines up with that position; if it does not
occur at all, slide past the position entirely. This rule is what makes a large alphabet cheap,
since a character absent from the needle skips a full needle length.

The good suffix rule looks at the suffix of the needle that did match before the mismatch. The
needle can only be reused where that suffix appears again, so slide to the next occurrence of the
matched suffix within the needle, or, when none remains, to the longest prefix of the needle that is
also a suffix of the matched part. Both cases come from one border table, computed exactly as in KMP
but on the reversed needle.

Dropping the good suffix rule and keying the bad character shift on the window's last character
rather than the mismatched one gives the Boyer-Moore-Horspool variant: only the O(A) table, shorter
to write under pressure, and nearly as fast in practice. Library routines such as
`std::string::find` and `memmem()` are usually built on one of these.

- `BoyerMoore(needle)` constructs the two shift tables for a nonempty string `needle` that is to be
  searched for subsequently in `haystack` queries.
- `find_in(haystack)` returns the first position that `needle` occurs in `haystack`, or
  `std::string::npos` if it cannot be found.
- `find_all_in(haystack)` returns the positions of every occurrence of `needle` in `haystack`, in
  increasing order. Overlapping occurrences are all reported, since after a match the needle only
  advances by its period.

Speed here is average-case, not worst-case: reporting every occurrence of a periodic needle such as
`"aaaa"` in a run of one character rescans the matched part each time, giving O(n*m) until the Galil
rule, which remembers how much of the needle a shift by the period already matched, restores
O(n + m). Prefer KMP in section 3.3.1 when the guarantee matters more than the average.

Time Complexity:
- O(m + A) per call to the constructor, where $m$ is the length of `needle` and $A$ is the alphabet
  size.
- O(n/m) per call to `find_in()` in the best case and O(n*m) in the worst case, where $n$ is the
  length of `haystack`.
- O(n*m) per call to `find_all_in()` in the worst case, plus O(z) for reporting $z$ matches.

Space Complexity:
- O(m + A) for storage of the shift tables.
- O(1) auxiliary for `find_in()`, and O(z) for the vector returned by `find_all_in()`.

*/

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <string>
#include <vector>
using std::string;

const int ALPHABET = 1 << CHAR_BIT;

class BoyerMoore {
  string needle;
  std::vector<int> last, good_suffix;

  // Returns the first occurrence at or after start, or -1 if there is none.
  int find_from(const string &haystack, int start) const {
    int n = static_cast<int>(haystack.size()), m = static_cast<int>(needle.size());
    for (int i = start; i + m <= n;) {
      int j = m - 1;
      while (j >= 0 && needle[j] == haystack[i + j]) {
        j--;
      }
      if (j < 0) {
        return i;
      }
      i += std::max(good_suffix[j + 1], j - last[static_cast<unsigned char>(haystack[i + j])]);
    }
    return -1;
  }

 public:
  explicit BoyerMoore(const string &needle) : needle(needle), last(ALPHABET, -1) {
    assert(!needle.empty());
    int m = static_cast<int>(needle.size());
    for (int i = 0; i < m; i++) {
      last[static_cast<unsigned char>(needle[i])] = i;
    }
    // border[i] is one past the start of the widest border of the suffix beginning at index i.
    good_suffix.assign(m + 1, 0);
    std::vector<int> border(m + 1);
    border[m] = m + 1;
    for (int i = m, j = m + 1; i > 0;) {
      while (j <= m && needle[i - 1] != needle[j - 1]) {
        if (good_suffix[j] == 0) {  // The first shift proposed for a suffix is the smallest one.
          good_suffix[j] = j - i;
        }
        j = border[j];
      }
      border[--i] = --j;
    }
    // Suffixes with no other occurrence fall back to the widest prefix that is also a suffix.
    for (int i = 0, j = border[0]; i <= m; i++) {
      if (good_suffix[i] == 0) {
        good_suffix[i] = j;
      }
      if (i == j) {
        j = border[j];
      }
    }
  }

  size_t find_in(const string &haystack) const {
    int pos = find_from(haystack, 0);
    return pos < 0 ? string::npos : static_cast<size_t>(pos);
  }

  std::vector<int> find_all_in(const string &haystack) const {
    std::vector<int> res;
    for (int pos = find_from(haystack, 0); pos >= 0;
         pos = find_from(haystack, pos + good_suffix[0])) {
      res.push_back(pos);
    }
    return res;
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  BoyerMoore needle("needle");
  assert(needle.find_in("haystack with a needle in it") == 16);
  assert(needle.find_in("haystack without one") == string::npos);
  assert(needle.find_in("needle") == 0);
  assert(needle.find_in("need") == string::npos);  // A haystack shorter than the needle.

  // A character absent from the needle lets the search skip a whole needle length.
  assert(BoyerMoore("abcd").find_in("xxxxxxxxabcd") == 8);

  // Overlapping occurrences are all reported.
  assert((BoyerMoore("aa").find_all_in("aaaa") == vector<int>{0, 1, 2}));
  assert((BoyerMoore("aba").find_all_in("abababa") == vector<int>{0, 2, 4}));
  assert(BoyerMoore("ab").find_all_in("cccc").empty());

  return 0;
}
