/*

A combination is a subset of size k chosen from a total of n (not necessarily
distinct) elements, where order does not matter.

- next_combination(lo, mid, hi) takes RandomAccessIterators lo, mid, and hi
  as a range [lo, hi) of n elements for which the function will rearrange such
  that the k elements in [lo, mid) becomes the next lexicographically greater
  combination. The function returns true if such a combination exists, or false
  if [lo, mid) already consists of the lexicographically greatest combination
  of the elements in [lo, hi) (in which case the values are unchanged). This
  implementation requires an ordering on the set of possible elements defined by
  the < operator on the iterator's value type.
- next_combination(n, k, a) rearranges a[] to become the next lexicographically
  greater combination of k distinct integers in the range [0, n). The array a[]
  must consist of k distinct integers in the range [0, n).
- next_combination_mask(x) interprets the bits of an integer x as a mask with
  1-bits specifying the chosen items for a combination and returns the mask of
  the next lexicographically greater combination (that is, the lowest integer
  greater than x with the same number of 1 bits). Note that this does not
  generate combinations in the same order as next_combination(), nor does it
  work if the corresponding n items are not distinct (in that case, duplicate
  combinations will be generated).
- combination_by_rank(n, k, r) returns the combination of k distinct integers in
  the range [0, n) that is lexicographically ranked r, where r is a zero-based
  rank in the range [0, n choose k).
- rank_by_combination(n, k, a) returns an integer representing the zero-based
  rank of combination a[], which must consist of k distinct integers in [0, n).
- next_combination_with_repeats(n, k, a) rearranges a[] to become the next
  lexicographically greater combination of k (not necessarily distinct) integers
  in the range [0, n). The array a[] must consist of k integers in the range
  [0, n). Note that there is a total of n multichoose k combinations if
  repetition is allowed, where n multichoose k = (n + k - 1) choose k.

Time Complexity:
- O(n) per call to next_combination(lo, hi), where n is the distance between
  lo and hi.
- O(k) per call to next_combination(n, k, a) and
  next_combination_with_repeats(n, k, a).
- O(1) per call to next_combination_mask(x).
- O(n*k) per call to combination_by_rank() and rank_by_combination().

Space Complexity:
- O(k) auxiliary heap space for combination_by_rank(n, k, r).
- O(1) auxiliary for all other operations.

*/

#include <algorithm>
#include <iterator>
#include <vector>

template<class It>
bool next_combination(It lo, It mid, It hi) {
  if (lo == mid || mid == hi) {
    return false;
  }
  It l = mid - 1, h = hi - 1;
  int len1 = 1, len2 = 1;
  while (l != lo && !(*l < *h)) {
    --l;
    ++len1;
  }
  if (l == lo && !(*l < *h)) {
    std::rotate(lo, mid, hi);
    return false;
  }
  for (; mid < h; ++len2) {
    if (!(*l < *--h)) {
      ++h;
      break;
    }
  }
  if (len1 == 1 || len2 == 1) {
    std::iter_swap(l, h);
  } else if (len1 == len2) {
    std::swap_ranges(l, mid, h);
  } else {
    std::iter_swap(l++, h++);
    int total = (--len1) + (--len2), gcd = total;
    for (int i = len1; i != 0; ) {
      std::swap(gcd %= i, i);
    }
    int skip = total/gcd - 1;
    for (int i = 0; i < gcd; i++) {
      It curr = (i < len1) ? (l + i) : (h + (i - len1));
      int k = i;
      typename std::iterator_traits<It>::value_type prev(*curr);
      for (int j = 0; j < skip; j++) {
        k = (k + len1) % total;
        It next = (k < len1) ? (l + k) : (h + (k - len1));
        *curr = *next;
        curr = next;
      }
      *curr = prev;
    }
  }
  return true;
}

bool next_combination(int n, int k, int a[]) {
  for (int i = k - 1; i >= 0; i--) {
    if (a[i] < n - k + i) {
      a[i]++;
      while (++i < k) {
        a[i] = a[i - 1] + 1;
      }
      return true;
    }
  }
  return false;
}

long long next_combination_mask(long long x) {
  long long s = x & -x, r = x + s;
  return r | (((x ^ r) >> 2)/s);
}

long long n_choose_k(long long n, long long k) {
  if (k > n - k) {
    k = n - k;
  }
  long long res = 1;
  for (int i = 0; i < k; i++) {
    res = res*(n - i)/(i + 1);
  }
  return res;
}

std::vector<int> combination_by_rank(int n, int k, long long r) {
  std::vector<int> res(k);
  int count = n;
  for (int i = 0; i < k; i++) {
    int j = 1;
    for (;; j++) {
      long long am = n_choose_k(count - j, k - 1 - i);
      if (r < am) {
        break;
      }
      r -= am;
    }
    res[i] = (i > 0) ? (res[i - 1] + j) : (j - 1);
    count -= j;
  }
  return res;
}

long long rank_by_combination(int n, int k, int a[]) {
  long long res = 0;
  int prev = -1;
  for (int i = 0; i < k; i++) {
    for (int j = prev + 1; j < a[i]; j++) {
      res += n_choose_k(n - 1 - j, k - 1 - i);
    }
    prev = a[i];
  }
  return res;
}

bool next_combination_with_repeats(int n, int k, int a[]) {
  for (int i = k - 1; i >= 0; i--) {
    if (a[i] < n - 1) {
      for (++a[i]; ++i < k; ) {
        a[i] = a[i - 1];
      }
      return true;
    }
  }
  return false;
}

/*** Example Usage and Output:

"11234" choose 3:
112 113 114 123 124 134 234

"abcde" choose 3 with masks:
abc abd acd bcd abe ace bce ade bde

5 choose 3:
{0,1,2} {0,1,3} {0,1,4} {0,2,3} {0,2,4} {0,3,4} {1,2,3} {1,2,4} {1,3,4} {2,3,4}

3 multichoose 2:
{0,0} {0,1} {0,2} {1,1} {1,2} {2,2}

***/

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
    int k = 3;
    string s = "11234";
    cout << "\"" << s << "\" choose " << k << ":" << endl;
    do {
      cout << s.substr(0, k) << " ";
    } while (next_combination(s.begin(), s.begin() + k, s.end()));
    cout << endl;
  }
  { // Unordered combinations using masks.
    int n = 5, k = 3;
    string char_set = "abcde";  // Must be distinct.
    cout << "\n\"" << char_set << "\" choose " << k << " with masks:" << endl;
    long long mask = 0, dest = 0;
    for (int i = 0; i < k; i++) {
      mask |= (1 << i);
    }
    for (int i = k - 1; i < n; i++) {
      dest |= (1 << i);
    }
    do {
      for (int i = 0; i < n; i++) {
        if ((mask >> i) & 1) {
          cout << char_set[i];
        }
      }
      cout << " ";
      mask = next_combination_mask(mask);
    } while (mask != dest);
    cout << endl;
  }
  { // Combinations of distinct integers from 0 to n - 1.
    int n = 5, k = 3, a[] = {0, 1, 2};
    cout << "\n" << n << " choose " << k << ":" << endl;
    int count = 0;
    do {
      print_range(a, a + k);
      vector<int> b = combination_by_rank(n, k, count);
      assert(equal(a, a + k, b.begin()));
      assert(rank_by_combination(n, k, a) == count);
      count++;
    } while (next_combination(n, k, a));
    cout << endl;
  }
  { // Combinations with repeats.
    int n = 3, k = 2, a[] = {0, 0};
    cout << "\n" << n << " multichoose " << k << ":" << endl;
    do {
      print_range(a, a + k);
    } while (next_combination_with_repeats(n, k, a));
    cout << endl;
  }
  return 0;
}
