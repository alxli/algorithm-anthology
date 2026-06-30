/*

For the purposes of this section, we define a "size $k$ arrangement of $n$" to be a permutation of a
size $k$ subset of the integers from $0$ to $n - 1$, for $0 \leq k \leq n$. There are $n
\mathbin{\text{permute}} k$ possible arrangements, but $n^k$ possible arrangements if repeated
values are allowed.

For arrangements without repeats, each prefix fixes some used values and leaves a shrinking set of
choices for the remaining positions. Ranking counts how many unused smaller choices could have been
placed at the current position, multiplied by the number of possible suffix arrangements; unranking
performs the inverse search by subtracting those block sizes. With repeats allowed, the sequence is
just a base-$n$ counter.

- `next_arrangement(n, a)` tries to rearrange `a` to the next lexicographically greater arrangement,
  returning true if such an arrangement exists or false if the array is already in descending order
  (in which case `a` is unchanged). The input `a` must consist of distinct integers in the range
  $[0, n)$.
- `arrangement_by_rank(n, k, r)` returns the size $k$ arrangement of $n$ which is lexicographically
  ranked $r$ out of all size $k$ arrangements of $n$, where $r$ is a 0-based rank in the range
  $[0, n \mathbin{\text{permute}} k)$.
- `rank_by_arrangement(n, a)` returns an integer representing the 0-based rank of arrangement `a`,
  which must consist of distinct integers in the range $[0, n)$.
- `next_arrangement_with_repeats(n, a)` tries to rearrange `a` to the next lexicographically greater
  arrangement with repeats, returning true if such an arrangement exists or false if the array is
  already in descending order (in which case `a` is unchanged). The input `a` must consist of
  integers in the range $[0, n)$. If `a` were interpreted as a $k$ digit integer in base $n$, this
  function could be thought of as incrementing the integer.

Time Complexity:
- O(n*k) for `next_arrangement(n, a)`, `arrangement_by_rank(n, k, r)`, and
  `rank_by_arrangement(n, a)`, where $k$ is the size of `a`.
- O(k) for `next_arrangement_with_repeats(n, a)`.

Space Complexity:
- O(n) auxiliary for `next_arrangement()`, `arrangement_by_rank()`, and `rank_by_arrangement()`.
- O(1) auxiliary for `next_arrangement_with_repeats()`.

*/

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

bool next_arrangement(int n, std::vector<int> &a) {
  int k = static_cast<int>(a.size());
  std::vector<char> used(n);
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

int64_t n_permute_k(int n, int k) {
  int64_t res = 1;
  for (int i = 0; i < k; i++) {
    res *= n - i;
  }
  return res;
}

std::vector<int> arrangement_by_rank(int n, int k, int64_t r) {
  std::vector<int> values(n), res(k);
  std::iota(values.begin(), values.end(), 0);
  for (int i = 0; i < k; i++) {
    int64_t count = n_permute_k(n - 1 - i, k - 1 - i);
    int pos = r / count;
    res[i] = values[pos];
    std::copy(values.begin() + pos + 1, values.end(), values.begin() + pos);
    r %= count;
  }
  return res;
}

int64_t rank_by_arrangement(int n, const std::vector<int> &a) {
  int k = static_cast<int>(a.size());
  int64_t res = 0;
  std::vector<char> used(n);
  for (int i = 0; i < k; i++) {
    int count = 0;
    for (int j = 0; j < a[i]; j++) {
      if (!used[j]) {
        count++;
      }
    }
    res += count * n_permute_k(n - i - 1, k - i - 1);
    used[a[i]] = true;
  }
  return res;
}

bool next_arrangement_with_repeats(int n, std::vector<int> &a) {
  int k = static_cast<int>(a.size());
  for (int i = k - 1; i >= 0; i--) {
    if (a[i] < n - 1) {
      a[i]++;
      std::fill(a.begin() + i + 1, a.end(), 0);
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
    int n = 4, k = 3;
    vector<int> a{0, 1, 2};
    cout << n << " permute " << k << " arrangements:" << endl;
    int count = 0;
    do {
      print_range(a.begin(), a.end());
      auto b = arrangement_by_rank(n, k, count);
      assert(a == b);
      assert(rank_by_arrangement(n, a) == count);
      count++;
      if (count == 10 || count == 20) {
        cout << endl;
      }
    } while (next_arrangement(n, a));
    assert(count == 24);
    cout << endl;
  }
  {
    int n = 4, k = 2;
    vector<int> a{0, 0};
    cout << endl << n << "^" << k << " arrangements with repeats:" << endl;
    int count = 0;
    do {
      print_range(a.begin(), a.end());
      count++;
      if (count == 13) {
        cout << endl;
      }
    } while (next_arrangement_with_repeats(n, a));
    assert(count == 16);
    cout << endl;
  }
  return 0;
}
