/*

Enumerate combinatorial sequences by inheriting an abstract class. Child classes of
`AbstractEnumerator` must implement the `count()` function which should return the number of
combinatorial sequences starting with the given prefix.

- `to_rank(a)` returns an integer representing the 0-based rank of the combinatorial sequence `a`.
- `from_rank(r)` returns a combinatorial sequence of integers that is lexicographically ranked `r`,
  where `r` is a 0-based rank in the range [0, `total_count()`).
- `enumerate(f)` calls the function `f(lo, hi)` on every specified combinatorial sequence in
  lexicographically increasing order, where `lo` and `hi` are two random-access iterators to a range
  [`lo`, `hi`) of integers.

Time Complexity:
- O(n^2) calls will be made to `count()` per call to all operations, where $n$ is the length of the
  combinatorial sequence.

Space Complexity:
- O(n) auxiliary heap space per call to all operations.

*/

#include <cstdint>
#include <vector>

class AbstractEnumerator {
 protected:
  int range, length;

  AbstractEnumerator(int r, int l) : range(r), length(l) {}
  virtual ~AbstractEnumerator() = default;
  virtual int64_t count(const std::vector<int> &prefix) { return 0; }
  std::vector<int> next(std::vector<int> &a) { return from_rank(to_rank(a) + 1); }
  int64_t total_count() { return count(std::vector<int>(0)); }

 public:
  int64_t to_rank(const std::vector<int> &a) {
    int64_t res = 0;
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
      std::vector<int> prefix(a.begin(), a.end());
      prefix.resize(i + 1);
      for (prefix[i] = 0; prefix[i] < a[i]; prefix[i]++) {
        res += count(prefix);
      }
    }
    return res;
  }

  std::vector<int> from_rank(int64_t r) {
    std::vector<int> a(length);
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
      std::vector<int> prefix(a.begin(), a.end());
      prefix.resize(i + 1);
      for (prefix[i] = 0; prefix[i] < range; ++prefix[i]) {
        int64_t curr = count(prefix);
        if (r < curr) {
          break;
        }
        r -= curr;
      }
      a[i] = prefix[i];
    }
    return a;
  }

  // Accepts any callable f(lo, hi), including capturing lambdas and functors.
  template<typename Fn>
  void enumerate(Fn f) {
    int64_t total = total_count();
    for (int64_t i = 0; i < total; i++) {
      std::vector<int> curr = from_rank(i);
      f(curr.begin(), curr.end());
    }
  }
};

class ArrangementEnumerator : public AbstractEnumerator {
 public:
  ArrangementEnumerator(int n, int k) : AbstractEnumerator(n, k) {}

  int64_t count(const std::vector<int> &prefix) {
    int n = static_cast<int>(prefix.size());
    for (int i = 0; i < n - 1; i++) {
      if (prefix[i] == prefix[n - 1]) {
        return 0;
      }
    }
    int64_t res = 1;
    for (int i = 0; i < length - n; i++) {
      res *= range - n - i;
    }
    return res;
  }
};

class PermutationEnumerator : public ArrangementEnumerator {
 public:
  PermutationEnumerator(int n) : ArrangementEnumerator(n, n) {}
};

class CombinationEnumerator : public AbstractEnumerator {
  std::vector<std::vector<int64_t>> table;

 public:
  CombinationEnumerator(int n, int k)
      : AbstractEnumerator(n, k), table(n + 1, std::vector<int64_t>(n + 1)) {
    for (int i = 0; i <= n; i++) {
      for (int j = 0; j <= i; j++) {
        table[i][j] = (j == 0) ? 1 : table[i - 1][j - 1] + table[i - 1][j];
      }
    }
  }

  int64_t count(const std::vector<int> &prefix) {
    int n = static_cast<int>(prefix.size());
    if (n >= 2 && prefix[n - 1] <= prefix[n - 2]) {
      return 0;
    }
    if (n == 0) {
      return table[range][length - n];
    }
    return table[range - prefix[n - 1] - 1][length - n];
  }
};

class PartitionEnumerator : public AbstractEnumerator {
  std::vector<std::vector<int64_t>> table;

 public:
  PartitionEnumerator(int n)
      : AbstractEnumerator(n + 1, n), table(n + 1, std::vector<int64_t>(n + 1)) {
    std::vector<std::vector<int64_t>> tmp(table);
    tmp[0][0] = 1;
    for (int i = 1; i <= n; i++) {
      for (int j = 1; j <= i; j++) {
        tmp[i][j] = tmp[i - 1][j - 1] + tmp[i - j][j];
      }
    }
    for (int i = 1; i <= n; i++) {
      for (int j = 1; j <= n; j++) {
        table[i][j] = tmp[i][j] + table[i][j - 1];
      }
    }
  }

  int64_t count(const std::vector<int> &prefix) {
    int n = static_cast<int>(prefix.size()), sum = 0;
    for (int x : prefix) {
      sum += x;
    }
    if (sum == range - 1) {
      return 1;
    }
    if (sum > range - 1 || (n > 0 && prefix[n - 1] == 0) ||
        (n >= 2 && prefix[n - 1] > prefix[n - 2])) {
      return 0;
    }
    if (n == 0) {
      return table[range - sum - 1][range - 1];
    }
    return table[range - sum - 1][prefix[n - 1]];
  }
};

/*** Example Usage and Output:

3 permute 2 arrangements:
{0,1} {0,2} {1,0} {1,2} {2,0} {2,1}

Permutations of [0, 3):
{0,1,2} {0,2,1} {1,0,2} {1,2,0} {2,0,1} {2,1,0}

4 choose 3 combinations:
{0,1,2} {0,1,3} {0,2,3} {1,2,3}

Partition of 4:
{1,1,1,1} {2,1,1,0} {2,2,0,0} {3,1,0,0} {4,0,0,0}

***/

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
    cout << "3 permute 2 arrangements:" << endl;
    ArrangementEnumerator arr(3, 2);
    arr.enumerate([](auto lo, auto hi) { print_range(lo, hi); });
    cout << endl;
  }
  {
    cout << "\nPermutations of [0, 3):" << endl;
    PermutationEnumerator perm(3);
    perm.enumerate([](auto lo, auto hi) { print_range(lo, hi); });
    cout << endl;
  }
  {
    cout << "\n4 choose 3 combinations:" << endl;
    CombinationEnumerator comb(4, 3);
    comb.enumerate([](auto lo, auto hi) { print_range(lo, hi); });
    cout << endl;
  }
  {
    cout << "\nPartition of 4:" << endl;
    PartitionEnumerator part(4);
    part.enumerate([](auto lo, auto hi) { print_range(lo, hi); });
    cout << endl;
  }
  return 0;
}
