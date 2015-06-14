/*

4.2.4 - Enumerating Combinations

We shall consider a combination n choose k to be an
set of k elements chosen from a total of n elements.
Unlike n permute k, the order here doesn't matter.
That is, 0 1 2 is considered the same as 0 2 1, so
we will consider the sorted representation of each
combination for purposes of the functions below.

*/

#include <algorithm>
#include <iterator>
#include <vector>

/*

Rearranges the values in the range [lo, hi) such that
elements in the range [lo, mid) becomes the next
lexicographically greater combination of the values from
[lo, hi) than it currently is, and returns whether the
function could rearrange [lo, hi) to a lexicographically
greater combination. If the range [lo, hi) contains n
elements and the range [lo, mid) contains k elements,
then starting off with a sorted range [lo, hi) and
calling next_combination() repeatedly will return true
for n choose k iterations before returning false.

*/

template<class It>
bool next_combination(It lo, It mid, It hi) {
  if (lo == mid || mid == hi) return false;
  It l(mid - 1), h(hi - 1);
  int sz1 = 1, sz2 = 1;
  while (l != lo && !(*l < *h)) --l, ++sz1;
  if (l == lo && !(*l < *h)) {
    std::rotate(lo, mid, hi);
    return false;
  }
  for (; mid < h; ++sz2) if (!(*l < *--h)) { ++h; break; }
  if (sz1 == 1 || sz2 == 1) {
    std::iter_swap(l, h);
  } else if (sz1 == sz2) {
    std::swap_ranges(l, mid, h);
  } else {
    std::iter_swap(l, h);
    ++l; ++h; --sz1; --sz2;
    int total = sz1 + sz2, gcd = total;
    for (int i = sz1; i != 0; ) std::swap(gcd %= i, i);
    int skip = total / gcd - 1;
    for (int i = 0; i < gcd; i++) {
      It curr(i < sz1 ? l + i : h + (i - sz1));
      int k = i;
      typename std::iterator_traits<It>::value_type v(*curr);
      for (int j = 0; j < skip; j++) {
        k = (k + sz1) % total;
        It next(k < sz1 ? l + k : h + (k - sz1));
        *curr = *next;
        curr = next;
      }
      *curr = v;
    }
  }
  return true;
}


/*

Changes a[] to the next lexicographically greater
combination of any k distinct integers in range [0, n).
The values of a[] that's passed should be k distinct
integers, each in range [0, n).

*/

bool next_combination(int n, int k, int a[]) {
  for (int i = k - 1; i >= 0; i--) {
    if (a[i] < n - k + i) {
      for (++a[i]; ++i < k; ) a[i] = a[i - 1] + 1;
      return true;
    }
  }
  return false;
}


/*

Finds the "mask" of the next combination of x. This is
equivalent to the next lexicographically greater permutation
of the binary digits of x. In other words, the function
simply returns the smallest integer greater than x which
has the same number of 1 bits (i.e. same popcount) as x.

examples: next_combination_mask(10101 base 2) =  10110
          next_combination_mask(11100 base 2) = 100011

If we arbitrarily number the n items of our collection from
0 to n-1, then generating all combinations n choose k can
be done as follows: initialize x such that popcount(x) = k
and the first (least-significant) k bits are all set to 1
(e.g. to do 5 choose 3, start at x = 00111 (base 2) = 7).
Then, we repeatedly call x = next_combination_mask(x) until
we reach 11100 (the lexicographically greatest mask for 5
choose 3), after which we stop. At any point in the process,
we can say that the i-th item is being "taken" (0 <= i < n)
iff the i-th bit of x is set.

Note: this does not produce combinations in the same order
as next_combination, nor does it work if your n items have
repeated values (in that case, repeated combos will be
generated).

*/

long long next_combination_mask(long long x) {
  long long s = x & -x, r = x + s;
  return r | (((x ^ r) >> 2) / s);
}


//n choose k in O(min(k, n - k))
long long n_choose_k(long long n, long long k) {
  if (k > n - k) k = n - k;
  long long res = 1;
  for (int i = 0; i < k; i++)
    res = res * (n - i) / (i + 1);
  return res;
}


/*

Given an integer rank x in range [0, n choose k), returns
a vector of integers representing the x-th lexicographically
smallest combination k distinct integers in [0, n).

examples: combination_by_rank(4, 3, 0) => {0, 1, 2}
          combination_by_rank(4, 3, 2) => {0, 2, 3}

*/

std::vector<int> combination_by_rank(int n, int k, int x) {
  std::vector<int> res(k);
  int cnt = n;
  for (int i = 0; i < k; i++) {
    int j = 1;
    for (;; j++) {
      long long am = n_choose_k(cnt - j, k - 1 - i);
      if (x < am) break;
      x -= am;
    }
    res[i] = i > 0 ? (res[i - 1] + j) : (j - 1);
    cnt -= j;
  }
  return res;
}


/*

Given an array a[] of k integers each in range [0, n), returns
the (0-based) lexicographical rank (counting from least to
greatest) of the combination specified by a[] in all possible
combination of k distinct integers in range [0, n).

examples: rank_by_combination(4, 3, {0, 1, 2}) => 0
          rank_by_combination(4, 3, {0, 2, 3}) => 2

*/

long long rank_by_combination(int n, int k, int a[]) {
  long long res = 0;
  int prev = -1;
  for (int i = 0; i < k; i++) {
    for (int j = prev + 1; j < a[i]; j++)
      res += n_choose_k(n - 1 - j, k - 1 - i);
    prev = a[i];
  }
  return res;
}


/*

Changes a[] to the next lexicographically greater
combination of any k (not necessarily distinct) integers
in range [0, n). The values of a[] that's passed should
be k integers, each in range [0, n). Note that there are
a total of n multichoose k combinations with repetition,
where n multichoose k = (n + k - 1) choose k

*/

bool next_combination_with_repeats(int n, int k, int a[]) {
  for (int i = k - 1; i >= 0; i--) {
    if (a[i] < n - 1) {
      for (++a[i]; ++i < k; ) a[i] = a[i - 1];
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
  { //like std::next_permutation(), repeats in the range allowed
    int k = 3;
    string s = "11234";
    cout << s << " choose " << k << ":\n";
    do {
      cout << s.substr(0, k) << "\n";
    } while (next_combination(s.begin(), s.begin() + k, s.end()));
    cout << "\n";
  }

  { //unordered combinations with masks
    int n = 5, k = 3;
    string s = "abcde"; //must be distinct values
    cout << s << " choose " << k << " with masks:\n";
    long long mask = 0, dest = 0;
    for (int i = 0; i < k; i++) mask |= 1 << i;
    for (int i = k - 1; i < n; i++) dest |= 1 << i;
    do {
      for (int i = 0; i < n; i++)
        if ((mask >> i) & 1) cout << s[i];
      cout << "\n";
      mask = next_combination_mask(mask);
    } while (mask != dest);
    cout << "\n";
  }

  { //only combinations of distinct integers from 0 to n - 1
    int n = 5, k = 3, a[] = {0, 1, 2};
    cout << n << " choose " << k << ":\n";
    int cnt = 0;
    do {
      print(a, a + k);
      vector<int> b = combination_by_rank(n, k, cnt);
      assert(equal(a, a + k, b.begin()));
      assert(rank_by_combination(n, k, a) == cnt);
      cnt++;
    } while (next_combination(n, k, a));
    cout << "\n";
  }

  { //combinations with repetition
    int n = 3, k = 2, a[] = {0, 0};
    cout << n << " multichoose " << k << ":\n";
    do {
      print(a, a + k);
    } while (next_combination_with_repeats(n, k, a));
  }
  return 0;
}
