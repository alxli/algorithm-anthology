/*

Enumerate combinatorial sequence by inheriting an abstract class. Child classes
of abstract_enumerator must implement the count() function which should return
the number of combinatorial sequences starting with the given prefix.

- to_rank(a) returns an integer representing the zero-based rank of the
  combinatorial sequence a.
- from_rank(r) returns a combinatorial sequence of integers that is
  lexicographically ranked r, where r is a zero-based rank in the range
  [0, total_count()).
- enumerate(f) calls the function f(lo, hi) on every specified combinatorial
  sequence in lexicographically increasing order, where lo and hi are two
  RandomAccessIterators to a range [lo, hi) of integers.

Time Complexity:
- O(n^2) calls will be made to count() per call to all operations, where n is
  the length of the combinatorial sequence.

Space Complexity:
- O(n) auxiliary heap space per call to all operations.

*/

#include <vector>

class abstract_enumerator {
 protected:
  int range, length;

  abstract_enumerator(int r, int l) : range(r), length(l) {}

  virtual long long count(const std::vector<int> &prefix) {
    return 0;
  }

  std::vector<int> next(std::vector<int> &a) {
    return from_rank(to_rank(a) + 1);
  }

  long long total_count() {
    return count(std::vector<int>(0));
  }

 public:
  long long to_rank(const std::vector<int> &a) {
    long long res = 0;
    for (int i = 0; i < (int)a.size(); i++) {
      std::vector<int> prefix(a.begin(), a.end());
      prefix.resize(i + 1);
      for (prefix[i] = 0; prefix[i] < a[i]; prefix[i]++) {
        res += count(prefix);
      }
    }
    return res;
  }

  std::vector<int> from_rank(long long r) {
    std::vector<int> a(length);
    for (int i = 0; i < (int)a.size(); i++) {
      std::vector<int> prefix(a.begin(), a.end());
      prefix.resize(i + 1);
      for (prefix[i] = 0; prefix[i] < range; ++prefix[i]) {
        long long curr = count(prefix);
        if (r < curr) {
          break;
        }
        r -= curr;
      }
      a[i] = prefix[i];
    }
    return a;
  }

  void enumerate(void (*f)(std::vector<int>::iterator,
                           std::vector<int>::iterator)) {
    long long total = total_count();
    for (long long i = 0; i < total; i++) {
      std::vector<int> curr = from_rank(i);
      f(curr.begin(), curr.end());
    }
  }
};

class arrangement_enumerator : public abstract_enumerator {
 public:
  arrangement_enumerator(int n, int k) : abstract_enumerator(n, k) {}

  long long count(const std::vector<int> &prefix) {
    int n = prefix.size();
    for (int i = 0; i < n - 1; i++) {
      if (prefix[i] == prefix[n - 1]) {
        return 0;
      }
    }
    long long res = 1;
    for (int i = 0; i < length - n; i++) {
      res *= range - n - i;
    }
    return res;
  }
};

class permutation_enumerator : public arrangement_enumerator {
 public:
  permutation_enumerator(int n) : arrangement_enumerator(n, n) {}
};

class combination_enumerator : public abstract_enumerator {
  std::vector<std::vector<long long> > table;

 public:
  combination_enumerator(int n, int k)
      : abstract_enumerator(n, k), table(n + 1, std::vector<long long>(n + 1)) {
    for (int i = 0; i <= n; i++) {
      for (int j = 0; j <= i; j++) {
        table[i][j] = (j == 0) ? 1 : table[i - 1][j - 1] + table[i - 1][j];
      }
    }
  }

  long long count(const std::vector<int> &prefix) {
    int n = prefix.size();
    if (n >= 2 && prefix[n - 1] <= prefix[n - 2]) {
      return 0;
    }
    if (n == 0) {
      return table[range][length - n];
    }
    return table[range - prefix[n - 1] - 1][length - n];
  }
};

class partition_enumerator : public abstract_enumerator {
  std::vector<std::vector<long long> > table;

 public:
  partition_enumerator(int n) : abstract_enumerator(n + 1, n),
                                table(n + 1, std::vector<long long>(n + 1)) {
    std::vector<std::vector<long long> > tmp(table);
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

  long long count(const std::vector<int> &prefix) {
    int n = (int)prefix.size(), sum = 0;
    for (int i = 0; i < n; i++) {
      sum += prefix[i];
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

Permutatations of [0, 3):
{0,1,2} {0,2,1} {1,0,2} {1,2,0} {2,0,1} {2,1,0}

4 choose 3 combinations:
{0,1,2} {0,1,3} {0,2,3} {1,2,3}

Partition of 4:
{1,1,1,1} {2,1,1,0} {2,2,0,0} {3,1,0,0} {4,0,0,0}

***/

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
    cout << "3 permute 2 arrangement_enumerator:" << endl;
    arrangement_enumerator arr(3, 2);
    arr.enumerate(print_range);
    cout << endl;
  }
  {
    cout << "\nPermutatations of [0, 3):" << endl;
    permutation_enumerator perm(3);
    perm.enumerate(print_range);
    cout << endl;
  }
  {
    cout << "\n4 choose 3 combinations:" << endl;
    combination_enumerator comb(4, 3);
    comb.enumerate(print_range);
    cout << endl;
  }
  {
    cout << "\nPartition of 4:" << endl;
    partition_enumerator part(4);
    part.enumerate(print_range);
    cout << endl;
  }
  return 0;
}
