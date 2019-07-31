/*

For the purposes of this section, we define a "size k arrangement of n" to be a
permutation of a size k subset of the integers from 0 to n - 1, for 0 <= k <= n.
There are n permute k possible arrangements, but n^k possible arrangements if
repeated values are allowed.

- next_arragement(n, k, a) tries to rearrange a[] to the next lexicographically
  greater arrangement, returning true if such an arrangement exists or false if
  the array is already in descending order (in which case a[] is unchanged). The
  input a[] must consist of exactly k distinct integers in the range [0, n).
- arrangement_by_rank(n, k, r) returns the size k arrangement of n which is
  lexicographically ranked r out of all size k arrangements of n, where r is
  a zero-based rank in the range [0, n permute k).
- rank_by_arrangement(n, k, a) returns an integer representing the zero-based
  rank of arrangement a[], which must consist of exactly k distinct integers in
  the range [0, n).
- next_arragement_with_repeats(n, k, a) tries to rearrange a[] to the next
  lexicographically greater arrangement with repeats, returning true if such an
  arrangement exists or false if the array is already in descending order (in
  which case a[] is unchanged). The input a[] must consist of exactly k (not
  necessarily distinct) integers in the range [0, n). If a[] were interpreted as
  a k digit integer in base n, this function could be thought of as incrementing
  the integer.

Time Complexity:
- O(n*k) for next_arrangement(), arrangement_by_rank(), and
  rank_by_arrangement().
- O(k) for next_arrangement_with_repeats().

Space Complexity:
- O(n) auxiliary heap space for next_arrangement(), arrangement_by_rank(), and
  rank_by_arrangement().
- O(1) auxiliary for next_arrangement_with_repeats().

*/

#include <algorithm>
#include <vector>

bool next_arrangement(int n, int k, int a[]) {
  std::vector<bool> used(n);
  for (int i = 0; i < k; i++) {
    used[a[i]] = true;
  }
  for (int i = k - 1; i >= 0; i--) {
    used[a[i]] = false;
    for (int j = a[i] + 1; j < n; j++) {
      if (!used[j]) {
        a[i++] = j;
        used[j] = true;
        for (int x = 0; i < k; x++) {
          if (!used[x]) {
            a[i++] = x;
          }
        }
        return true;
      }
    }
  }
  return false;
}

long long n_permute_k(int n, int k) {
  long long res = 1;
  for (int i = 0; i < k; i++) {
    res *= n - i;
  }
  return res;
}

std::vector<int> arrangement_by_rank(int n, int k, long long r) {
  std::vector<int> values(n), res(k);
  for (int i = 0; i < n; i++) {
    values[i] = i;
  }
  for (int i = 0; i < k; i++) {
    long long count = n_permute_k(n - 1 - i, k - 1 - i);
    int pos = r/count;
    res[i] = values[pos];
    std::copy(values.begin() + pos + 1, values.end(), values.begin() + pos);
    r %= count;
  }
  return res;
}

long long rank_by_arrangement(int n, int k, int a[]) {
  long long res = 0;
  std::vector<bool> used(n);
  for (int i = 0; i < k; i++) {
    int count = 0;
    for (int j = 0; j < a[i]; j++) {
      if (!used[j]) {
        count++;
      }
    }
    res += count*n_permute_k(n - i - 1, k - i - 1);
    used[a[i]] = true;
  }
  return res;
}

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

/*** Example Usage and Output:

4 permute 3 arrangements:
{0,1,2} {0,1,3} {0,2,1} {0,2,3} {0,3,1} {0,3,2} {1,0,2} {1,0,3} {1,2,0} {1,2,3}
{1,3,0} {1,3,2} {2,0,1} {2,0,3} {2,1,0} {2,1,3} {2,3,0} {2,3,1} {3,0,1} {3,0,2}
{3,1,0} {3,1,2} {3,2,0} {3,2,1}

4^2 arrangements with repeats:
{0,0} {0,1} {0,2} {0,3} {1,0} {1,1} {1,2} {1,3} {2,0} {2,1} {2,2} {2,3} {3,0}
{3,1} {3,2} {3,3}

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
    int n = 4, k = 3, a[] = {0, 1, 2};
    cout << n << " permute " << k << " arrangements:" << endl;
    int count = 0;
    do {
      print_range(a, a + k);
      vector<int> b = arrangement_by_rank(n, k, count);
      assert(equal(a, a + k, b.begin()));
      assert(rank_by_arrangement(n, k, a) == count);
      count++;
    } while (next_arrangement(n, k, a));
    cout << endl;
  }
  {
    int n = 4, k = 2, a[] = {0, 0};
    cout << endl << n << "^" << k << " arrangements with repeats:" << endl;
    do {
      print_range(a, a + k);
    } while (next_arrangement_with_repeats(n, k, a));
    cout << endl;
  }
  return 0;
}
