/*

A permutation is an ordered list consisting of $n$ (not necessarily distinct) elements.

The lexicographic successor is found by locating the rightmost ascent, increasing that position by
the smallest possible amount, then reversing the suffix into its minimum order. Ranking and
unranking permutations use the factorial number system: each position contributes the number of
unused smaller values before the chosen one, scaled by the number of possible suffix permutations.
The cycle decomposition views a permutation as a function on indices and follows each unvisited
orbit until it returns to its start.

- `next_permutation2(lo, hi, comp = std::less<>())` is analogous to `std::next_permutation()`,
  taking two BidirectionalIterators as a range $[`lo`, `hi`)$ for which the function tries to
  rearrange to the next lexicographically greater permutation according to `comp`. The function
  returns true if such a permutation exists, or false if the range is already in reverse comparator
  order, in which case it is rearranged into comparator order.
- `next_permutation(a, comp = std::less<>())` is analogous to `next_permutation2()`, except that it
  takes a vector instead of a range.
- `next_permutation_mask(x)` returns the next integer having the same number of 1-bits. Treating
  each 1-bit as whether to take its corresponding item generates combinations of a set of $n$ items.
- `permutation_by_rank(n, r)` returns the permutation of the integers in the range $[0, `n`)$ which
  is lexicographically ranked $r$, where $r$ is a 0-based rank in the range $[0, n!)$.
- `rank_by_permutation(a)` returns an integer representing the 0-based rank of permutation `a`,
  which must be a permutation of the integers $[0, n)$.
- `permutation_cycles(a)` returns the orbits of the index mapping `i` $\mapsto$ `a[i]`. For example,
  $\{3, 1, 0, 2\}$ decomposes into cycles $\{0, 3, 2\}$ and $\{1\}$ because following the mapping
  from index $0$ gives $0 \to 3 \to 2 \to 0$, while index $1$ maps to itself.

Time Complexity:
- O(n) per call to `next_permutation2()` and `next_permutation()`, where $n$ is the input size.
- O(n^2) per call to `permutation_by_rank()` and `rank_by_permutation()`.
- O(1) per call to `next_permutation_mask()`.
- O(n) per call to `permutation_cycles()`.

Space Complexity:
- O(1) auxiliary for `next_permutation2()` and `next_permutation()`.
- O(n) auxiliary for `permutation_by_rank()`, `rank_by_permutation()`, and `permutation_cycles()`.

*/

#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>
#include <utility>
#include <vector>

template<typename It, typename Compare = std::less<>>
bool next_permutation2(It lo, It hi, Compare comp = Compare()) {
  if (lo == hi) {
    return false;
  }
  It i = lo;
  if (++i == hi) {
    return false;
  }
  i = hi;
  --i;
  while (true) {
    It j = i;
    if (comp(*--i, *j)) {
      It k = hi;
      while (!comp(*i, *--k)) {
      }
      std::iter_swap(i, k);
      std::reverse(j, hi);
      return true;
    }
    if (i == lo) {
      std::reverse(lo, hi);
      return false;
    }
  }
}

template<typename T, typename Compare = std::less<>>
bool next_permutation(std::vector<T> &a, Compare comp = Compare()) {
  int n = static_cast<int>(a.size());
  for (int i = n - 2; i >= 0; i--) {
    if (comp(a[i], a[i + 1])) {
      for (int j = n - 1;; j--) {
        if (comp(a[i], a[j])) {
          std::swap(a[i++], a[j]);
          for (j = n - 1; i < j; i++, j--) {
            std::swap(a[i], a[j]);
          }
          return true;
        }
      }
    }
  }
  std::reverse(a.begin(), a.end());
  return false;
}

int64_t next_permutation_mask(int64_t x) {
  if (x == 0) {
    return 0;
  }
  int64_t s = x & -x, r = x + s;
  return r | (((x ^ r) >> 2) / s);
}

std::vector<int> permutation_by_rank(int n, int64_t r) {
  if (n == 0) {
    return {};
  }
  std::vector<int64_t> factorial(n);
  std::vector<int> values(n), res(n);
  factorial[0] = 1;
  for (int i = 1; i < n; i++) {
    factorial[i] = i * factorial[i - 1];
  }
  std::iota(values.begin(), values.end(), 0);
  for (int i = 0; i < n; i++) {
    int pos = r / factorial[n - 1 - i];
    res[i] = values[pos];
    values.erase(values.begin() + pos);
    r %= factorial[n - 1 - i];
  }
  return res;
}

int64_t rank_by_permutation(const std::vector<int> &a) {
  int n = static_cast<int>(a.size());
  if (n == 0) {
    return 0;
  }
  std::vector<int64_t> factorial(n);
  factorial[0] = 1;
  for (int i = 1; i < n; i++) {
    factorial[i] = i * factorial[i - 1];
  }
  int64_t res = 0;
  for (int i = 0; i < n; i++) {
    int v = a[i];
    for (int j = 0; j < i; j++) {
      if (a[j] < a[i]) {
        v--;
      }
    }
    res += v * factorial[n - 1 - i];
  }
  return res;
}

using Cycles = std::vector<std::vector<int>>;

Cycles permutation_cycles(const std::vector<int> &a) {
  int n = static_cast<int>(a.size());
  std::vector<char> visit(n);
  Cycles res;
  for (int i = 0; i < n; i++) {
    if (!visit[i]) {
      int j = i;
      std::vector<int> curr;
      do {
        curr.push_back(j);
        visit[j] = true;
        j = a[j];
      } while (j != i);
      res.push_back(std::move(curr));
    }
  }
  return res;
}

/*** Example Usage and Output:

Permutations of [0, 4):
{0,1,2,3} {0,1,3,2} {0,2,1,3} {0,2,3,1} {0,3,1,2} {0,3,2,1} {1,0,2,3} {1,0,3,2}
{1,2,0,3} {1,2,3,0} {1,3,0,2} {1,3,2,0} {2,0,1,3} {2,0,3,1} {2,1,0,3} {2,1,3,0}
{2,3,0,1} {2,3,1,0} {3,0,1,2} {3,0,2,1} {3,1,0,2} {3,1,2,0} {3,2,0,1} {3,2,1,0}

Permutations of 2 zeros and 3 ones:
00111 01011 01101 01110 10011 10101 10110 11001 11010 11100

Decomposition of {3,1,0,2} into cycles:
{0,3,2} {1}

***/

#include <bitset>
#include <cassert>
#include <iostream>
using namespace std;

template<typename It>
void print_range(It lo, It hi) {
  cout << "{";
  for (; lo != hi; ++lo) {
    cout << *lo << (lo == hi - 1 ? "" : ",");
  }
  cout << "} ";
}

int main() {
  {
    const int n = 4;
    vector<int> a{0, 1, 2, 3}, b = a, c = a;
    cout << "Permutations of [0, " << n << "):" << endl;
    int count = 0;
    do {
      print_range(a.begin(), a.end());
      assert(b == a);
      assert(c == a);
      vector<int> d = permutation_by_rank(n, count);
      assert(d == a);
      assert(rank_by_permutation(a) == count);
      count++;
      if (count == 8 || count == 16) {
        cout << endl;
      }
      std::next_permutation(b.begin(), b.end());
      next_permutation(c);
    } while (next_permutation(a));
    assert(count == 24);
    assert((a == vector<int>{0, 1, 2, 3}));
    cout << endl;
  }
  {  // Permutations of binary digits.
    const int n = 5;
    cout << "\nPermutations of 2 zeros and 3 ones:" << endl;
    int lo = bitset<5>(string("00111")).to_ulong();
    int hi = bitset<6>(string("100011")).to_ulong();
    int count = 0;
    do {
      cout << bitset<n>(lo).to_string() << " ";
      count++;
    } while ((lo = next_permutation_mask(lo)) != hi);
    assert(count == 10);
    cout << endl;
  }
  {
    vector<int> a{3, 2, 1}, b = a;
    assert(next_permutation(a, greater<int>()));
    assert(next_permutation2(b.begin(), b.end(), greater<int>()));
    assert((a == vector<int>{3, 1, 2} && b == a));
  }
  {  // Decomposition into cycles.
    vector<int> a{3, 1, 0, 2};
    cout << "\nDecomposition of {3,1,0,2} into cycles:" << endl;
    Cycles c = permutation_cycles(a);
    assert(c.size() == 2);
    assert((c[0] == vector<int>{0, 3, 2}));
    assert((c[1] == vector<int>{1}));
    for (const auto &cycle : c) {
      print_range(cycle.begin(), cycle.end());
    }
    cout << endl;
  }
  return 0;
}
