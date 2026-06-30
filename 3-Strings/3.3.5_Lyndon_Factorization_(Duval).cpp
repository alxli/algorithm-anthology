/*

A Lyndon word is a nonempty string that is strictly smaller than all of its proper suffixes, or
equivalently, strictly smaller than all of its nontrivial cyclic rotations. By the Chen-Fox-Lyndon
theorem, every string has a unique factorization into a sequence of Lyndon words whose values are
non-increasing: $s = w_1 w_2 \cdots w_k$ with $w_1 \geq w_2 \geq \cdots \geq w_k$. Duval's algorithm
computes this factorization in linear time and constant auxiliary by scanning the string while
tracking the current candidate Lyndon prefix and the position it last matched.

The same scan, applied to the string viewed as a cycle, yields the starting position of its
lexicographically least rotation. This is the standard way to canonicalize a necklace, for example
to test whether two strings are cyclic rotations of one another.

Both functions below operate on any random-access sequence whose elements support `operator<`.

- `lyndon_factorization(s)` returns a vector of the starting indices of the Lyndon factors of `s`,
  in order. The factor at index `i` of the result spans the half-open range from `res[i]` to
  `res[i + 1]` (or to `s.size()` for the final factor).
- `min_rotation(s)` returns the 0-indexed position `p` such that the rotation beginning at `p`, that
  is, `s[p]`, `s[(p + 1) % n]`, ..., `s[(p + n - 1) % n]`, is lexicographically smallest. If several
  rotations tie, the smallest such `p` is returned.

Time Complexity:
- O(n) per call to `lyndon_factorization(s)` and `min_rotation(s)`, where $n$ is the length of `s`.

Space Complexity:
- O(k) auxiliary for `lyndon_factorization(s)`, where $k$ is the number of factors.
- O(1) auxiliary for `min_rotation(s)`.

*/

#include <vector>

template<typename Seq>
std::vector<int> lyndon_factorization(const Seq &s) {
  int n = static_cast<int>(s.size());
  std::vector<int> starts;
  int i = 0;
  while (i < n) {
    int j = i + 1, k = i;
    while (j < n && !(s[j] < s[k])) {
      k = (s[k] < s[j]) ? i : k + 1;
      j++;
    }
    while (i <= k) {
      starts.push_back(i);
      i += j - k;
    }
  }
  return starts;
}

template<typename Seq>
int min_rotation(const Seq &s) {
  int n = static_cast<int>(s.size());
  if (n == 0) {
    return 0;
  }
  int i = 0, res = 0;
  while (i < n) {
    res = i;
    int j = i + 1, k = i;
    while (j < 2 * n && !(s[j % n] < s[k % n])) {
      k = (s[k % n] < s[j % n]) ? i : k + 1;
      j++;
    }
    while (i <= k) {
      i += j - k;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <string>
using std::string;

int main() {
  // "banana" factors as "b" >= "an" >= "an" >= "a".
  assert((lyndon_factorization(string("banana")) == std::vector<int>{0, 1, 3, 5}));

  // Among the rotations of "baca", "abac" (starting at index 3) is smallest.
  assert(min_rotation(string("baca")) == 3);

  // Both routines work on any comparable sequence, not just strings.
  assert(min_rotation(std::vector<int>{3, 1, 2, 1, 3}) == 1);
  return 0;
}
