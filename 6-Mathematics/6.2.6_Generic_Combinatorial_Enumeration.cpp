/*

Ranks, unranks, and enumerates combinatorial sequences in lexicographic order using only a prefix
counting oracle. This is useful when the objects are too numerous to precompute, but there is a
simple dynamic-programming formula for the number of valid completions after a fixed prefix.

`Enumerator(range, length, count)` considers values in $[0, `range`)$ at each position of a
length-`length` sequence. The callback `count(prefix)` returns the number of valid full sequences
beginning with `prefix`; it may capture a precomputed dynamic-programming table or other state. The
factory functions below supply this oracle for the principal combinatorial families, while the class
remains available directly for custom families. The range and length must be nonnegative; the
factory arguments must satisfy $0 \leq k \leq n$.

- `arrangements(n, k)` returns an enumerator for the length-$k$ arrangements of $[0, n)$.
- `permutations(n)` returns an enumerator for the permutations of $[0, n)$.
- `combinations(n, k)` returns an enumerator for the size-$k$ subsets of $[0, n)$, represented as
  strictly increasing sequences.
- `partitions(n)` returns an enumerator for the additive partitions of $n$, represented as
  non-increasing length-$n$ sequences padded with zeros.
- `to_rank(a)` returns the 0-based rank of the valid combinatorial sequence `a`.
- `from_rank(r)` returns a combinatorial sequence of integers that is lexicographically ranked `r`,
  where `r` is a 0-based rank in the range $[0, `total_count()`)$.
- `total_count()` returns the number of valid sequences.
- `enumerate(f)` calls the function `f(a)` on every specified combinatorial sequence `a` in
  lexicographically increasing order.

Overflow warning: All exact counts and ranks must fit in `int64_t`.

Time Complexity:
- O(A*L*C) per call to `to_rank()` and `from_rank()`, where $A$ is `range`, $L$ is `length`, and $C$
  is the cost of `count()`.
- O(T*A*L*C) per call to `enumerate()`, excluding the cost of the callback `f`, where $T$ is
  `total_count()`.
- O(A*(min(L, A - L) + 1)) per call to `combinations()` and O(L^2) per call to `partitions()`; the
  other factories take O(1) time per call.

Space Complexity:
- O(L) auxiliary for `to_rank()`, `from_rank()`, and `enumerate()`, excluding output storage.
- O(S) object storage, where $S$ is the state captured by `count()`.
- O(A*(min(L, A - L) + 1)) object storage for `combinations()` and O(L^2) for `partitions()`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

template<typename Count>
class Enumerator {
  int range, length;
  Count count;

 public:
  Enumerator(int range, int length, Count count)
      : range(range), length(length), count(std::move(count)) {
    assert(range >= 0 && length >= 0);
  }

  int64_t total_count() { return count({}); }

  int64_t to_rank(const std::vector<int> &a) {
    assert(static_cast<int>(a.size()) == length);
    assert(std::all_of(a.begin(), a.end(), [&](int x) { return 0 <= x && x < range; }));
    int64_t res = 0;
    std::vector<int> prefix;
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
      prefix.push_back(0);
      for (; prefix.back() < a[i]; prefix.back()++) {
        res += count(prefix);
      }
    }
    return res;
  }

  std::vector<int> from_rank(int64_t r) {
    assert(0 <= r && r < total_count());
    std::vector<int> a;
    for (int i = 0; i < length; i++) {
      a.push_back(0);
      for (; a.back() < range; a.back()++) {
        int64_t curr = count(a);
        if (r < curr) {
          break;
        }
        r -= curr;
      }
    }
    return a;
  }

  // Accepts any callable f(a), including capturing lambdas and functors.
  template<typename Fn>
  void enumerate(Fn f) {
    int64_t total = total_count();
    for (int64_t i = 0; i < total; i++) {
      std::vector<int> curr = from_rank(i);
      f(curr);
    }
  }
};

auto arrangements(int n, int k) {
  assert(n >= 0 && 0 <= k && k <= n);
  auto count = [=](const std::vector<int> &prefix) -> int64_t {
    int len = static_cast<int>(prefix.size());
    for (int i = 0; i < len - 1; i++) {
      if (prefix[i] == prefix[len - 1]) {
        return 0;
      }
    }
    int64_t res = 1;
    for (int i = 0; i < k - len; i++) {
      res *= n - len - i;  // Overflow warning.
    }
    return res;
  };
  return Enumerator(n, k, count);
}

auto permutations(int n) {
  return arrangements(n, n);
}

auto combinations(int n, int k) {
  assert(n >= 0 && 0 <= k && k <= n);
  std::vector<std::vector<int64_t>> table(n + 1, std::vector<int64_t>(std::min(k, n - k) + 1));
  int max_col = static_cast<int>(table[0].size()) - 1;
  for (int i = 0; i <= n; i++) {
    for (int j = 0; j <= std::min(i, max_col); j++) {
      table[i][j] = (j == 0) ? 1 : table[i - 1][j - 1] + table[i - 1][j];  // Overflow warning.
    }
  }
  auto count = [n, k, table = std::move(table)](const std::vector<int> &pref) -> int64_t {
    int len = static_cast<int>(pref.size());
    if (len >= 2 && pref[len - 1] <= pref[len - 2]) {
      return 0;
    }
    int rem = len == 0 ? n : n - pref.back() - 1;
    int choose = k - len;
    return (choose < 0 || choose > rem) ? 0 : table[rem][std::min(choose, rem - choose)];
  };
  return Enumerator(n, k, std::move(count));
}

auto partitions(int n) {
  assert(n >= 0);
  std::vector<std::vector<int64_t>> table(n + 1, std::vector<int64_t>(n + 1));
  std::fill(table[0].begin(), table[0].end(), 1);
  for (int sum = 1; sum <= n; sum++) {
    for (int max_part = 1; max_part <= n; max_part++) {
      table[sum][max_part] = table[sum][max_part - 1];
      if (max_part <= sum) {
        table[sum][max_part] += table[sum - max_part][max_part];  // Overflow warning.
      }
    }
  }
  auto count = [n, table = std::move(table)](const std::vector<int> &pref) -> int64_t {
    int len = static_cast<int>(pref.size());
    int sum = std::accumulate(pref.begin(), pref.end(), 0);
    if (sum == n) {
      return 1;
    }
    if (sum > n || (len > 0 && pref.back() == 0) || (len >= 2 && pref[len - 1] > pref[len - 2])) {
      return 0;
    }
    return table[n - sum][len == 0 ? n : pref.back()];
  };
  return Enumerator(n + 1, n, std::move(count));
}

/*** Example Usage and Output:

3 permute 2 arrangements:
{0,1} {0,2} {1,0} {1,2} {2,0} {2,1}

Permutations of [0, 3):
{0,1,2} {0,2,1} {1,0,2} {1,2,0} {2,0,1} {2,1,0}

4 choose 3 combinations:
{0,1,2} {0,1,3} {0,2,3} {1,2,3}

Partitions of 4:
{1,1,1,1} {2,1,1,0} {2,2,0,0} {3,1,0,0} {4,0,0,0}

Length-4 binary strings without consecutive ones:
{0,0,0,0} {0,0,0,1} {0,0,1,0} {0,1,0,0} {0,1,0,1} {1,0,0,0} {1,0,0,1} {1,0,1,0}

***/

#include <iostream>
using namespace std;

void print_sequence(const vector<int> &a) {
  cout << "{";
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    cout << a[i] << (i + 1 == static_cast<int>(a.size()) ? "" : ",");
  }
  cout << "} ";
}

int main() {
  cout << "3 permute 2 arrangements:" << endl;
  auto arr = arrangements(3, 2);
  int count = 0;
  arr.enumerate([&](const auto &a) {
    print_sequence(a);
    count++;
  });
  assert(count == 6);
  assert((arr.from_rank(5) == vector<int>{2, 1}));
  assert(arr.to_rank(vector<int>{2, 1}) == 5);

  cout << "\n\nPermutations of [0, 3):" << endl;
  auto perm = permutations(3);
  count = 0;
  perm.enumerate([&](const auto &a) {
    print_sequence(a);
    count++;
  });
  assert(count == 6);

  cout << "\n\n4 choose 3 combinations:" << endl;
  auto comb = combinations(4, 3);
  count = 0;
  comb.enumerate([&](const auto &a) {
    print_sequence(a);
    count++;
  });
  assert(count == 4);
  assert((comb.from_rank(3) == vector<int>{1, 2, 3}));
  assert(comb.to_rank(vector<int>{1, 2, 3}) == 3);
  assert(combinations(67, 1).total_count() == 67);
  assert(combinations(67, 67).total_count() == 1);
  assert(combinations(66, 33).total_count() == 7219428434016265740LL);

  cout << "\n\nPartitions of 4:" << endl;
  auto part = partitions(4);
  count = 0;
  part.enumerate([&](const auto &a) {
    print_sequence(a);
    count++;
  });
  assert(count == 5);

  int length = 4;
  auto count_completions = [length](const vector<int> &pref) -> int64_t {
    if (static_cast<int>(pref.size()) > length) {
      return 0;
    }
    for (int i = 0; i < static_cast<int>(pref.size()); i++) {
      if (pref[i] < 0 || pref[i] > 1 || (i > 0 && pref[i - 1] == 1 && pref[i] == 1)) {
        return 0;
      }
    }
    int64_t after_zero = 1, after_one = 1;
    for (int i = static_cast<int>(pref.size()); i < length; i++) {
      int64_t next_zero = after_zero + after_one;  // Overflow warning.
      after_one = after_zero;
      after_zero = next_zero;
    }
    return pref.empty() || pref.back() == 0 ? after_zero : after_one;
  };
  Enumerator e(2, length, count_completions);
  cout << "\n\nLength-4 binary strings without consecutive ones:" << endl;
  count = 0;
  e.enumerate([&](const auto &a) {
    print_sequence(a);
    count++;
  });
  assert(count == 8 && e.total_count() == 8);
  assert((e.from_rank(7) == vector<int>{1, 0, 1, 0}));
  assert(e.to_rank(vector<int>{1, 0, 1, 0}) == 7);
  cout << endl;
  return 0;
}
