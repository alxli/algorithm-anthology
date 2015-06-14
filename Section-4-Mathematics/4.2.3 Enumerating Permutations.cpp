/*

4.2.3 - Enumerating Permutations

We shall consider a permutation of n objects to be an
ordered list of size n that contains all n elements,
where order is important. E.g. 1 1 2 0 and 0 1 2 1
are considered two different permutations of 0 1 1 2.
Compared to our prior definition of an arrangement, a
permutable range of size n may contain repeated values
of any type, not just the integers from 0 to n - 1.

*/

#include <algorithm>
#include <vector>

//identical to std::next_permutation()
template<class It> bool _next_permutation(It lo, It hi) {
  if (lo == hi) return false;
  It i = lo;
  if (++i == hi) return false;
  i = hi; --i;
  for (;;) {
    It j = i; --i;
    if (*i < *j) {
      It k = hi;
      while (!(*i < *--k)) /* pass */;
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

//array version
template<class T> bool next_permutation(int n, T a[]) {
  for (int i = n - 2; i >= 0; i--)
    if (a[i] < a[i + 1])
      for (int j = n - 1; ; j--)
        if (a[i] < a[j]) {
          std::swap(a[i++], a[j]);
          for (j = n - 1; i < j; i++, j--)
            std::swap(a[i], a[j]);
          return true;
        }
  return false;
}


/*

Calls the custom function f(vector) on all permutations
of the integers from 0 to n - 1. This is more efficient
than making many consecutive calls to next_permutation(),
however, here, the permutations will not be printed in
lexicographically increasing order.

*/

template<class ReportFunction>
void gen_permutations(int n, ReportFunction report,
                      std::vector<int> & p, int d) {
  if (d == n) {
    report(p);
    return;
  }
  for (int i = 0; i < n; i++) {
    if (p[i] == 0) {
      p[i] = d;
      gen_permutations(n, report, p, d + 1);
      p[i] = 0;
    }
  }
}

template<class ReportFunction>
void gen_permutations(int n, ReportFunction report) {
  std::vector<int> perms(n, 0);
  gen_permutations(n, report, perms, 0);
}


/*

Finds the next lexicographically greater permutation of
the binary digits of x. In other words, next_permutation()
simply returns the smallest integer greater than x which
has the same number of 1 bits (i.e. same popcount) as x.

examples: next_permutation(10101 base 2) =  10110
          next_permutation(11100 base 2) = 100011

This can also be used to generate combinations as follows:
If we let k = popcount(x), then we can use this to generate
all possible masks to tell us which k items to take out of
n total items (represented by the first n bits of x).

*/

long long next_permutation(long long x) {
  long long s = x & -x, r = x + s;
  return r | (((x ^ r) >> 2) / s);
}


/*

Given an integer rank x in range [0, n!), returns a vector
of integers representing the x-th lexicographically smallest
permutation of the integers in [0, n).

examples: permutation_by_rank(4, 0) => {0, 1, 2, 3}
          permutation_by_rank(4, 5) => {0, 3, 2, 1}

*/

std::vector<int> permutation_by_rank(int n, long long x) {
  long long fact[n];
  fact[0] = 1;
  for (int i = 1; i < n; i++)
    fact[i] = i * fact[i - 1];
  std::vector<int> free(n), res(n);
  for (int i = 0; i < n; i++) free[i] = i;
  for (int i = 0; i < n; i++) {
    int pos = x / fact[n - 1 - i];
    res[i] = free[pos];
    std::copy(free.begin() + pos + 1, free.end(),
              free.begin() + pos);
    x %= fact[n - 1 - i];
  }
  return res;
}


/*

Given an array a[] of n integers each in range [0, n), returns
the (0-based) lexicographical rank (counting from least to
greatest) of the arrangement specified by a[] in all possible
permutations of the integers from 0 to n - 1.

examples: rank_by_permutation(3, {0, 1, 2}) => 0
          rank_by_permutation(3, {2, 1, 0}) => 5

*/

template<class T> long long rank_by_permutation(int n, T a[]) {
  long long fact[n];
  fact[0] = 1;
  for (int i = 1; i < n; i++)
    fact[i] = i * fact[i - 1];
  long long res = 0;
  for (int i = 0; i < n; i++) {
    int v = a[i];
    for (int j = 0; j < i; j++)
      if (a[j] < a[i]) v--;
    res += v * fact[n - 1 - i];
  }
  return res;
}


/*

Given a permutation a[] of the integers from 0 to n - 1,
returns a decomposition of the permutation into cycles.
A permutation cycle is a subset of a permutation whose
elements trade places with one another. For example, the
permutation {0, 2, 1, 3} decomposes to {0, 3, 2} and {1}.
Here, the notation {0, 3, 2} means that starting from the
original ordering {0, 1, 2, 3}, the 0th value is replaced
by the 3rd, the 3rd by the 2nd, and the 2nd by the first,
See: http://mathworld.wolfram.com/PermutationCycle.html

*/

typedef std::vector<std::vector<int> > cycles;

cycles decompose_into_cycles(int n, int a[]) {
  std::vector<bool> vis(n);
  cycles res;
  for (int i = 0; i < n; i++) {
    if (vis[i]) continue;
    int j = i;
    std::vector<int> cur;
    do {
      cur.push_back(j);
      vis[j] = true;
      j = a[j];
    } while (j != i);
    res.push_back(cur);
  }
  return res;
}


/*** Example Usage ***/

#include <bitset>
#include <cassert>
#include <iostream>
using namespace std;

void printperm(const vector<int> & perm) {
  for (int i = 0; i < perm.size(); i++)
    cout << perm[i] << " ";
  cout << "\n";
}

template<class it> void print(it lo, it hi) {
  for (; lo != hi; ++lo) cout << *lo << " ";
  cout << "\n";
}

int main() {
  { //method 1: ordered
    int n = 4, a[] = {0, 1, 2, 3};
    int b[n], c[n];
    for (int i = 0; i < n; i++) b[i] = c[i] = a[i];
    cout << "Ordered permutations of 0 to " << n-1 << ":\n";
    int cnt = 0;
    do {
      print(a, a + n);
      assert(equal(b, b + n, a));
      assert(equal(c, c + n, a));
      vector<int> d = permutation_by_rank(n, cnt);
      assert(equal(d.begin(), d.end(), a));
      assert(rank_by_permutation(n, a) == cnt);
      cnt++;
      std::next_permutation(b, b + n);
      _next_permutation(c, c + n);
    } while (next_permutation(n, a));
    cout << "\n";
  }

  { //method 2: unordered
    int n = 3;
    cout << "Unordered permutations of 0 to " << n-1 << ":\n";
    gen_permutations(n, printperm);
    cout << "\n";
  }

  { //permuting binary digits
    const int n = 5;
    cout << "Permutations of 2 zeros and 3 ones:\n";
    long long lo = 7;  // 00111 in base 2
    long long hi = 35; //100011 in base 2
    do {
      cout << bitset<n>(lo).to_string() << "\n";
    } while ((lo = next_permutation(lo)) != hi);
    cout << "\n";
  }

  { //permutation cycles
    int n = 4, a[] = {3, 1, 0, 2};
    cout << "Decompose 0 2 1 3 into cycles:\n";
    cycles c = decompose_into_cycles(n, a);
    for (int i = 0; i < c.size(); i++) {
      cout << "Cycle " << i + 1 << ":";
      for (int j = 0; j < c[i].size(); j++)
        cout << " " << c[i][j];
      cout << "\n";
    }
  }
  return 0;
}
