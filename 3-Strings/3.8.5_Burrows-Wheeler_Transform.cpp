/*

The Burrows-Wheeler transform (BWT) permutes a string into one that tends to be far more
compressible while remaining perfectly invertible. It writes down every cyclic rotation of the
string, sorts them lexicographically, and reads off the last character of each sorted rotation.
Characters that occur in similar contexts are brought together, producing long runs that run-length
and move-to-front coders exploit; it is the heart of the bzip2 compressor and a building block of
compressed-text indexes.

To make the permutation invertible, a unique sentinel character smaller than every real character is
appended before transforming, so the rotations are all distinct and one of them is distinguished as
the original. Sorting the rotations of the sentineled string is the same as sorting its suffixes, so
the forward transform is computed with a prefix-doubling rotation sort. The inverse rebuilds the
string with the last-to-first (LF) mapping: stably ordering the transformed characters yields, for
each position of the sorted first column, the row that supplies the next character, and following
that mapping from the sentinel row walks the original string out one character at a time.

The input must not contain the sentinel character `'\0'`.

- `bwt(s)` returns the BWT of `s`. The result has length `s.size() + 1` and contains the sentinel
  `'\0'` exactly once.
- `inverse_bwt(code)` returns the original string, inverting `bwt`. The argument must be a string
  produced by that function.

Time Complexity:
- O(n log^2 n) per call to `bwt(s)`, where $n$ is the length of `s`.
- O(n log n) per call to `inverse_bwt(code)`.

Space Complexity:
- O(n) auxiliary heap space for both functions.

*/

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

std::string bwt(std::string s) {
  s.push_back('\0');  // Unique sentinel, smaller than every real character.
  int n = static_cast<int>(s.size());
  // Sort the indices of the cyclic rotations by prefix doubling; with the sentinel this matches
  // sorting the suffixes.
  std::vector<int> order(n), rank(n), tmp(n);
  for (int i = 0; i < n; i++) {
    order[i] = i;
    rank[i] = static_cast<unsigned char>(s[i]);
  }
  for (int k = 1; k < n; k <<= 1) {
    auto cmp = [&](int a, int b) {
      if (rank[a] != rank[b]) {
        return rank[a] < rank[b];
      }
      return rank[(a + k) % n] < rank[(b + k) % n];
    };
    std::sort(order.begin(), order.end(), cmp);
    tmp[order[0]] = 0;
    for (int i = 1; i < n; i++) {
      tmp[order[i]] = tmp[order[i - 1]] + (cmp(order[i - 1], order[i]) ? 1 : 0);
    }
    rank = tmp;
    if (rank[order[n - 1]] == n - 1) {
      break;
    }
  }
  std::string code(n, '\0');
  for (int i = 0; i < n; i++) {
    code[i] = s[(order[i] + n - 1) % n];  // Last character of the i-th sorted rotation.
  }
  return code;
}

std::string inverse_bwt(const std::string &code) {
  int n = static_cast<int>(code.size());
  // first[k] is the row supplying the first column's k-th character; following it from row 0 (the
  // sentinel row, sorted first) reconstructs the sentinel followed by the original string.
  std::vector<int> first(n);
  std::iota(first.begin(), first.end(), 0);
  std::stable_sort(first.begin(), first.end(), [&](int a, int b) {
    return static_cast<unsigned char>(code[a]) < static_cast<unsigned char>(code[b]);
  });
  std::string res;
  res.reserve(n);
  for (int i = 0, row = 0; i < n; i++) {
    row = first[row];
    res.push_back(code[row]);
  }
  return res.substr(1);  // Drop the leading sentinel.
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  string code = bwt("banana");
  assert(code.size() == 7);  // "banana" plus the sentinel.
  assert(inverse_bwt(code) == "banana");

  // The transform clusters equal characters: "mississippi" yields long runs.
  string text = "mississippi";
  string t = bwt(text);
  assert(inverse_bwt(t) == text);

  // Round-trips on assorted strings, including repeats and a single character.
  for (const string &s :
       {string(""), string("a"), string("aaaa"), string("abracadabra"),
        string("the quick brown fox"), string("\1\2abc", 5)}) {
    assert(inverse_bwt(bwt(s)) == s);
  }
  return 0;
}
