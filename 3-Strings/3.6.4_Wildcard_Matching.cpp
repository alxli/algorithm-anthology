/*

Decide whether a pattern of literal characters, single-character wildcards, and multi-character
wildcards matches an entire text. The wildcard `?` matches exactly one character, and `*` matches
any run of characters including the empty one. This is glob matching rather than regular expression
matching, so there is no alternation and no repetition of a group.

Let $m(i, j)$ mean that the first $i$ characters of the text are matched by the first $j$ characters
of the pattern. A literal or `?` consumes one character from each side, so it depends only on
$m(i - 1, j - 1)$. A `*` either matches nothing, leaving $m(i, j - 1)$, or absorbs one more text
character, leaving $m(i - 1, j)$; every longer run is covered by repeating that second case, which
is what keeps the table two-dimensional instead of requiring a loop over run lengths. Only the
previous row is ever read, so one row of storage suffices.

- `wildcard_match(text, pattern)` returns whether `pattern` matches all of `text`, where `?` matches
  any single character and `*` matches any sequence of characters.

A greedy walk needs only O(1) space: remember the most recent `*` and the text position it was
reached at, then on a mismatch advance the text and resume just after that `*`. Backtracking to the
latest `*` suffices, since an earlier one absorbs whatever a later gives up. It runs in O(n + m) on
ordinary inputs but stays O(n*m) in the worst case, since each mismatch can rescan the pattern from
the last `*`. The table is what generalizes, to counting matches or allowing bounded mismatches.

Time Complexity:
- O(n*m) per call, where $n$ and $m$ are the lengths of `text` and `pattern`.

Space Complexity:
- O(m) auxiliary.

*/

#include <string>
#include <vector>

bool wildcard_match(const std::string &text, const std::string &pattern) {
  int n = static_cast<int>(text.size()), m = static_cast<int>(pattern.size());
  std::vector<char> prev(m + 1), curr(m + 1);
  prev[0] = true;
  for (int j = 1; j <= m; j++) {  // An all-star prefix is the only way to match an empty text.
    prev[j] = prev[j - 1] && pattern[j - 1] == '*';
  }
  for (int i = 1; i <= n; i++) {
    curr[0] = false;
    for (int j = 1; j <= m; j++) {
      if (pattern[j - 1] == '*') {
        curr[j] = curr[j - 1] || prev[j];  // Match nothing, or absorb one more character.
      } else if (pattern[j - 1] == '?' || pattern[j - 1] == text[i - 1]) {
        curr[j] = prev[j - 1];
      } else {
        curr[j] = false;
      }
    }
    prev.swap(curr);
  }
  return prev[m];
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(wildcard_match("", ""));
  assert(wildcard_match("", "*"));
  assert(wildcard_match("", "***"));
  assert(!wildcard_match("", "?"));
  assert(!wildcard_match("abc", ""));

  assert(wildcard_match("abc", "abc"));
  assert(wildcard_match("abc", "a?c"));
  assert(wildcard_match("abc", "a*"));
  assert(wildcard_match("abc", "*c"));
  assert(wildcard_match("abcde", "a*d?"));
  assert(!wildcard_match("abc", "a?"));  // The pattern must cover the whole text.
  assert(!wildcard_match("abc", "*d*"));

  // The wildcard must not consume characters the literals still need.
  assert(wildcard_match("aaa", "*a"));
  assert(!wildcard_match("aab", "*a"));
  assert(wildcard_match("mississippi", "m*i*s*p?"));
  assert(!wildcard_match("mississippi", "m*i*s*x*"));
  return 0;
}
