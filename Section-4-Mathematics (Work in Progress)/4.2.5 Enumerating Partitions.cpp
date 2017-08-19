/*

A partition of a natural number n is a way to write n as a sum of positive
integers where the order of the addends does not matter.

- next_partition(p) takes a reference to a vector p[] of positive integers as a
  partition of n for which the function will re-assign to become the next
  lexicographically greater partition. The function returns true if such a
  partition exists, or false if p[] already consists of the lexicographically
  greatest partition (i.e. the single integer n).
- partition_by_rank(n, r) returns the partition of n that is lexicographically
  ranked r if addends in each partition were sorted in non-increasing order,
  where r is a zero-based rank in the range [0, partitions(n)).
- rank_by_partition(p) returns an integer representing the zero-based rank of
  the partition specified by vector p[], which must consist of positive integers
  sorted in non-increasing order.
- generate_increasing_partitions(n, f) calls the function f(lo, hi) on strictly
  increasing partitions of n in lexicographically increasing order of partition,
  where lo and hi are two RandomAccessIterators to a range [lo, hi) of integers.
  Note that non-strictly increasing partitions like {1, 1, 1, 1} are skipped.

Time Complexity:
- O(n) per call to next_partition().
- O(n^2) per call to partition_by_rank(n, r) and rank_by_partition(p).
- O(p(n)) per call to generate_increasing_partitions(n, f), where p(n) is the
  number of partitions of n.

Space Complexity:
- O(1) auxiliary for next_partition().
- O(n^2) auxiliary heap space for partition_function(), partition_by_rank(), and
  rank_by_partition().
- O(n) auxiliary stack space for generate_increasing_partitions().

*/

#include <vector>

bool next_partition(std::vector<int> &p) {
  int n = p.size();
  if (n <= 1) {
    return false;
  }
  int s = p[n - 1] - 1, i = n - 2;
  p.pop_back();
  for (; i > 0 && p[i] == p[i - 1]; i--) {
    s += p[i];
    p.pop_back();
  }
  for (p[i]++; s > 0; s--) {
    p.push_back(1);
  }
  return true;
}

long long partition_function(int a, int b) {
  static std::vector<std::vector<long long> > p(
      1, std::vector<long long>(1, 1));
  if (a >= (int)p.size()) {
    int old = p.size();
    p.resize(a + 1);
    p[0].resize(a + 1);
    for (int i = 1; i <= a; i++) {
      p[i].resize(a + 1);
      for (int j = old; j <= i; j++) {
        p[i][j] = p[i - 1][j - 1] + p[i - j][j];
      }
    }
  }
  return p[a][b];
}

std::vector<int> partition_by_rank(int n, long long r) {
  std::vector<int> res;
  for (int i = n, j; i > 0; i -= j) {
    for (j = 1; ; j++) {
      long long count = partition_function(i, j);
      if (r < count) {
        break;
      }
      r -= count;
    }
    res.push_back(j);
  }
  return res;
}

long long rank_by_partition(const std::vector<int> &p) {
  long long res = 0;
  int sum = 0;
  for (int i = 0; i < (int)p.size(); i++) {
    sum += p[i];
  }
  for (int i = 0; i < (int)p.size(); i++) {
    for (int j = 0; j < p[i]; j++) {
      res += partition_function(sum, j);
    }
    sum -= p[i];
  }
  return res;
}

typedef void (*ReportFunction)(std::vector<int>::iterator,
                               std::vector<int>::iterator);

void generate_increasing_partitions(int left, int prev, int i,
                                    std::vector<int> &p, ReportFunction f) {
  if (left == 0) {
    f(p.begin(), p.begin() + i);
    return;
  }
  for (p[i] = prev + 1; p[i] <= left; p[i]++) {
    generate_increasing_partitions(left - p[i], p[i], i + 1, p, f);
  }
}

void generate_increasing_partitions(int n, ReportFunction f) {
  std::vector<int> p(n, 0);
  generate_increasing_partitions(n, 0, 0, p, f);
}

/*** Example Usage and Output:

Partitions of 4:
{1,1,1,1} {2,1,1} {2,2} {3,1} {4}

Increasing partitions of 8:
{1,2,5} {1,3,4} {1,7} {2,6} {3,5} {8}

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
    int n = 4;
    vector<int> a(n, 1);
    cout << "Partitions of " << n << ":" << endl;
    int count = 0;
    do {
      print_range(a.begin(), a.end());
      vector<int> b = partition_by_rank(n, count);
      assert(equal(a.begin(), a.end(), b.begin()));
      assert(rank_by_partition(a) == count);
      count++;
    } while (next_partition(a));
    cout << endl;
  }
  {
    int n = 8;
    cout << "\nIncreasing partitions of " << n << ":" << endl;
    generate_increasing_partitions(n, print_range);
    cout << endl;
  }
  return 0;
}
