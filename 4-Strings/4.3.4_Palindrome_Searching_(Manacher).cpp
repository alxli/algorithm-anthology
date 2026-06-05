/*

Computes all odd- and even-length palindromic radii of a string in linear time using Manacher's
algorithm. These radii can be used to test whether any substring is a palindrome in O(1), find the
longest palindromic substring, or count all palindromic substrings.

For odd palindromes, `odd[i]` is the radius centered at character `s[i]`, including the center
character. Thus the palindrome spans `[i - odd[i] + 1, i + odd[i])`. For even palindromes, `even[i]`
is the radius centered between `s[i - 1]` and `s[i]`. Thus the palindrome spans
`[i - even[i], i + even[i])`.

- `Manacher(s)` constructs the palindromic radii arrays for string `s`.
- `is_palindrome(lo, hi)` returns whether substring `[lo, hi)` is a palindrome.
- `longest_palindrome()` returns one longest palindromic substring of `s`.
- `count_palindromes()` returns the total number of palindromic substrings of `s`, counted with
  multiplicity by position.

Time Complexity:
- O(n) per constructor call, where $n$ is the length of `s`.
- O(1) per call to `is_palindrome(lo, r)`.
- O(n) per call to `longest_palindrome()` and `count_palindromes()`.

Space Complexity:
- O(n) for storage of the radii arrays.
- O(1) auxiliary per query.

*/

#include <algorithm>
#include <string>
#include <vector>
using std::string;

class Manacher {
  string s;
  std::vector<int> odd, even;

 public:
  explicit Manacher(const string &s) : s(s), odd(s.size()), even(s.size()) {
    int n = s.size();
    for (int i = 0, l = 0, r = -1; i < n; i++) {
      int k = (i > r) ? 1 : std::min(odd[l + r - i], r - i + 1);
      while (0 <= i - k && i + k < n && s[i - k] == s[i + k]) {
        k++;
      }
      odd[i] = k;
      if (i + k - 1 > r) {
        l = i - k + 1;
        r = i + k - 1;
      }
    }
    for (int i = 0, l = 0, r = -1; i < n; i++) {
      int k = (i > r) ? 0 : std::min(even[l + r - i + 1], r - i + 1);
      while (0 <= i - k - 1 && i + k < n && s[i - k - 1] == s[i + k]) {
        k++;
      }
      even[i] = k;
      if (i + k - 1 > r) {
        l = i - k;
        r = i + k - 1;
      }
    }
  }

  bool is_palindrome(int lo, int hi) const {
    int len = hi - lo;
    if (len <= 0) {
      return true;
    }
    int mid = (lo + hi) / 2;
    return (len % 2) ? odd[mid] >= len / 2 + 1 : even[mid] >= len / 2;
  }

  string longest_palindrome() const {
    int best_l = 0, best_len = 0;
    int n = static_cast<int>(s.size());
    for (int i = 0; i < n; i++) {
      int len = 2 * odd[i] - 1;
      if (len > best_len) {
        best_len = len;
        best_l = i - odd[i] + 1;
      }
      len = 2 * even[i];
      if (len > best_len) {
        best_len = len;
        best_l = i - even[i];
      }
    }
    return s.substr(best_l, best_len);
  }

  long long count_palindromes() const {
    long long res = 0;
    for (int i = 0; i < static_cast<int>(s.size()); i++) {
      res += odd[i] + even[i];
    }
    return res;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  Manacher m("abacaba");
  assert(m.is_palindrome(0, 7));
  assert(m.is_palindrome(1, 6));
  assert(!m.is_palindrome(0, 6));
  assert(m.longest_palindrome() == "abacaba");
  assert(m.count_palindromes() == 12);

  Manacher e("cabbad");
  assert(e.is_palindrome(1, 5));
  assert(e.longest_palindrome() == "abba");
  return 0;
}
