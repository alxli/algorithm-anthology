/*

4.2.2 - Enumerating Arrangements

We shall consider an arrangement to be a permutation of
all the integers from 0 to n - 1. For our purposes, the
difference between an arrangement and a permutation is
simply that a permutation can pertain to a set of any
given values, not just distinct integers from 0 to n-1.

*/

#include <algorithm>
#include <vector>

/*

Changes a[] to the next lexicographically greater
permutation of any k distinct integers in range [0, n).
The values of a[] that's passed should be k distinct
integers, each in range [0, n).

returns: whether the function could rearrange a[] to
a lexicographically greater arrangement.

examples:
next_arrangement(4, 3, {0, 1, 2}) => 1,  a[] = {0, 1, 3}
next_arrangement(4, 3, {0, 1, 3}) => 1,  a[] = {0, 2, 1}
next_arrangement(4, 3, {3, 2, 1}) => 0,  a[] unchanged

*/

bool next_arrangement(int n, int k, int a[]) {
  std::vector<bool> used(n);
  for (int i = 0; i < k; i++) used[a[i]] = true;
  for (int i = k - 1; i >= 0; i--) {
    used[a[i]] = false;
    for (int j = a[i] + 1; j < n; j++) {
      if (!used[j]) {
        a[i++] = j;
        used[j] = true;
        for (int x = 0; i < k; x++)
          if (!used[x]) a[i++] = x;
        return true;
      }
    }
  }
  return false;
}


/*

Computes n permute k using formula: nPk = n!/(n - k)!
Complexity: O(k).  E.g. n_permute_k(10, 7) = 604800

*/

long long n_permute_k(int n, int k) {
  long long res = 1;
  for (int i = 0; i < k; i++) res *= n - i;
  return res;
}


/*

Given an integer rank x in range [0, n permute k), returns
a vector of integers representing the x-th lexicographically
smallest permutation of any k distinct integers in [0, n).

examples: arrangement_by_rank(4, 3, 0) => {0, 1, 2}
          arrangement_by_rank(4, 3, 5) => {0, 3, 2}

*/

std::vector<int> arrangement_by_rank(int n, int k, int x) {
  std::vector<int> free(n), res(k);
  for (int i = 0; i < n; i++) free[i] = i;
  for (int i = 0; i < k; i++) {
    long long cnt = n_permute_k(n - 1 - i, k - 1 - i);
    int pos = x / cnt;
    res[i] = free[pos];
    std::copy(free.begin() + pos + 1, free.end(),
              free.begin() + pos);
    x %= cnt;
  }
  return res;
}


/*

Given an array a[] of k integers each in range [0, n), returns
the (0-based) lexicographical rank (counting from least to
greatest) of the arrangement specified by a[] in all possible
permutations of k distinct integers in range [0, n).

examples: rank_by_arrangement(4, 3, {0, 1, 2}) => 0
          rank_by_arrangement(4, 3, {0, 3, 2}) => 5

*/

long long rank_by_arrangement(int n, int k, int a[]) {
  long long res = 0;
  std::vector<bool> used(n);
  for (int i = 0; i < k; i++) {
    int cnt = 0;
    for (int j = 0; j < a[i]; j++)
      if (!used[j]) cnt++;
    res += cnt * n_permute_k(n - i - 1, k - i - 1);
    used[a[i]] = true;
  }
  return res;
}


/*

Changes a[] to the next lexicographically greater
permutation of k (not-necessarily distinct) integers in
range [0, n). The values of a[] should be in range [0, n).
If a[] was interpreted as a base-n integer that is k digits
long, this function would be equivalent to incrementing a.
Ergo, there are n^k arrangements if repeats are allowed.

returns: whether the function could rearrange a[] to a
lexicographically greater arrangement with repeats.

examples:
n_a_w_r(4, 3, {0, 0, 0}) => 1,  a[] = {0, 0, 1}
n_a_w_r(4, 3, {0, 1, 3}) => 1,  a[] = {0, 2, 0}
n_a_w_r(4, 3, {3, 3, 3}) => 0,  a[] unchanged

*/

bool next_arrangement_with_repeats(int n, int k, int a[]) {
  for (int i = k - 1; i >= 0; i--) {
    if (a[i] < n - 1) {
      a[i]++;
      std::fill(a + i + 1, a + k, 0);
      return true;
    }
  }
  return false;
}


/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

template<class it> void print(it lo, it hi) {
  for (; lo != hi; ++lo) cout << *lo << " ";
  cout << "\n";
}

int main() {
  {
    int n = 4, k = 3, a[] = {0, 1, 2};
    cout << n << " permute " << k << " arrangements:\n";
    int cnt = 0;
    do {
      print(a, a + k);
      vector<int> b = arrangement_by_rank(n, k, cnt);
      assert(equal(a, a + k, b.begin()));
      assert(rank_by_arrangement(n, k, a) == cnt);
      cnt++;
    } while (next_arrangement(n, k, a));
    cout << "\n";
  }
  {
    int n = 4, k = 2, a[] = {0, 0};
    cout << n << "^" << k << " arrangements with repeats:\n";
    int cnt = 0;
    do {
      print(a, a + k);
      cnt++;
    } while (next_arrangement_with_repeats(n, k, a));
  }
  return 0;
}
