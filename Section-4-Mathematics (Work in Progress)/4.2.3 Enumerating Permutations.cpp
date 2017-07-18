/*

A permutation is an ordered list consisting of n (not necessarily distinct)
elements.

- next_permutation_(lo, hi) is analogous to std::next_permutation(lo, hi),
  taking two BidirectionalIterators lo and hi as a range [lo, hi) for which the
  function tries to rearrange to the next lexicographically greater permutation.
  The function returns true if such a permutation exists, or false if the range
  is already in descending order (in which case the values are unchanged). This
  implementation requires an ordering on the set of possible elements defined by
  the < operator on the iterator's value type.
- next_permutation(n, a) is analogous to next_permutation(), except that it
  takes an array a[] of size n instead of a range.
- next_permutation(x) returns the next lexicographically greater permutation of
  the binary digits of the integer x, that is, the lowest integer greater than
  x with the same number of 1-bits. This can be used to generate combinations of
  a set of n items by treating each 1 bit as whether to "take" the item at the
  corresponding position.
- permutation_by_rank(n, r) returns the permutation of the integers in the range
  [0, n) which is lexicographically ranked r, where r is a zero-based rank in
  the range [0, n!).
- rank_by_permutation(n, a) returns an integer representing the zero-based
  rank of permutation a[], which must be a permutation of the integers [0, n).
- permutation_cycles(n, a) returns the decomposition of the permutation a[] into
  cycles. A permutation cycle is a subset of a permutation whose elements are
  consecutively swapped, relative to a sorted set. For example, {3, 1, 0, 2}
  decomposes to {0, 3, 2} and {1}, meaning that starting from the sorted order
  {0, 1, 2, 3}, the 0th value is replaced by the 3rd, the 3rd by the 2nd, and
  the 2nd by the 0th (0 -> 3 -> 2 -> 0).

Time Complexity:
- O(n^2) per call to next_permutation_(lo, hi), where n is the distance between
  lo and hi.
- O(n^2) per call to next_permutation(n, a), permutation_by_rank(n, r), and
  rank_by_permutation(n, a).
- O(1) per call to next_permutation(x).
- O(n) per call to permutation_cycles().

Space Complexity:
- O(1) auxiliary for next_permutation_() and next_permutation().
- O(n) auxiliary heap space for permutation_by_rank(), rank_by_permutation(),
  and permutation_cycles().

*/

#include <algorithm>
#include <vector>

template<class It>
bool next_permutation_(It lo, It hi) {
  if (lo == hi) {
    return false;
  }
  It i = lo;
  if (++i == hi) {
    return false;
  }
  i = hi;
  --i;
  for (;;) {
    It j = i;
    if (*--i < *j) {
      It k = hi;
      while (!(*i < *--k)) {}
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

template<class T>
bool next_permutation(int n, T a[]) {
  for (int i = n - 2; i >= 0; i--) {
    if (a[i] < a[i + 1]) {
      for (int j = n - 1; ; j--) {
        if (a[i] < a[j]) {
          std::swap(a[i++], a[j]);
          for (j = n - 1; i < j; i++, j--) {
            std::swap(a[i], a[j]);
          }
          return true;
        }
      }
    }
  }
  return false;
}

long long next_permutation(long long x) {
  long long s = x & -x, r = x + s;
  return r | (((x ^ r) >> 2)/s);
}

std::vector<int> permutation_by_rank(int n, long long x) {
  std::vector<long long> factorial(n);
  std::vector<int> values(n), res(n);
  factorial[0] = 1;
  for (int i = 1; i < n; i++) {
    factorial[i] = i*factorial[i - 1];
  }
  for (int i = 0; i < n; i++) {
    values[i] = i;
  }
  for (int i = 0; i < n; i++) {
    int pos = x/factorial[n - 1 - i];
    res[i] = values[pos];
    std::copy(values.begin() + pos + 1, values.end(), values.begin() + pos);
    x %= factorial[n - 1 - i];
  }
  return res;
}

long long rank_by_permutation(int n, int a[]) {
  std::vector<long long> factorial(n);
  factorial[0] = 1;
  for (int i = 1; i < n; i++) {
    factorial[i] = i*factorial[i - 1];
  }
  long long res = 0;
  for (int i = 0; i < n; i++) {
    int v = a[i];
    for (int j = 0; j < i; j++) {
      if (a[j] < a[i]) {
        v--;
      }
    }
    res += v*factorial[n - 1 - i];
  }
  return res;
}

typedef std::vector<std::vector<int> > cycles;

cycles permutation_cycles(int n, int a[]) {
  std::vector<bool> visit(n);
  cycles res;
  for (int i = 0; i < n; i++) {
    if (!visit[i]) {
      int j = i;
      std::vector<int> curr;
      do {
        curr.push_back(j);
        visit[j] = true;
        j = a[j];
      } while (j != i);
      res.push_back(curr);
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

template<class It>
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
    int a[] = {0, 1, 2, 3}, b[n], c[n];
    for (int i = 0; i < n; i++) {
      b[i] = c[i] = a[i];
    }
    cout << "Permutations of [0, " << n << "):" << endl;
    int count = 0;
    do {
      print_range(a, a + n);
      assert(equal(b, b + n, a));
      assert(equal(c, c + n, a));
      vector<int> d = permutation_by_rank(n, count);
      assert(equal(d.begin(), d.end(), a));
      assert(rank_by_permutation(n, a) == count);
      count++;
      std::next_permutation(b, b + n);
      next_permutation(c, c + n);
    } while (next_permutation(n, a));
    cout << endl;
  }
  { // Permutations of binary digits.
    const int n = 5;
    cout << "\nPermutations of 2 zeros and 3 ones:" << endl;
    int lo = bitset<5>(string("00111")).to_ulong();
    int hi = bitset<6>(string("100011")).to_ulong();
    do {
      cout << bitset<n>(lo).to_string() << " ";
    } while ((lo = next_permutation(lo)) != hi);
    cout << endl;
  }
  { // Decomposition into cycles.
    const int n = 4;
    int a[] = {3, 1, 0, 2};
    cout << "\nDecomposition of {3,1,0,2} into cycles:" << endl;
    cycles c = permutation_cycles(n, a);
    for (int i = 0; i < (int)c.size(); i++) {
      print_range(c[i].begin(), c[i].end());
    }
    cout << endl;
  }
  return 0;
}
