/*

4.2.5 - Enumerating Partitions

We shall consider a partition of an integer n to be an
unordered multiset of positive integers that has a total
sum equal to n. Since both 2 1 1 and 1 2 1 represent the
same partition of 4, we shall consider only descending
sorted lists as "valid" partitions for functions below.

*/

#include <vector>

/*

Given a vector representing a partition of some
integer n (the sum of all values in the vector),
changes p to the next lexicographically greater
partition of n and returns whether the change was
successful (whether a lexicographically greater
partition existed). Note that the "initial" value
of p must be a vector of size n, all initialized 1.

e.g. next_partition({2, 1, 1}) => 1, p becomes {2, 2}
     next_partition({2, 2})    => 1, p becomes {3, 1}
     next_partition({4})       => 0, p is unchanged

*/

bool next_partition(std::vector<int> & p) {
  int n = p.size();
  if (n <= 1) return false;
  int s = p[n - 1] - 1, i = n - 2;
  p.pop_back();
  for (; i > 0 && p[i] == p[i - 1]; i--) {
    s += p[i];
    p.pop_back();
  }
  for (p[i]++; s-- > 0; ) p.push_back(1);
  return true;
}

/* Returns the number of partitions of n. */

long long count_partitions(int n) {
  std::vector<long long> p(n + 1, 0);
  p[0] = 1;
  for (int i = 1; i <= n; i++)
    for (int j = i; j <= n; j++)
      p[j] += p[j - i];
  return p[n];
}

/* Helper function for partitioning by rank */

std::vector< std::vector<long long> >
  p(1, std::vector<long long>(1, 1)); //memoization

long long partition_function(int a, int b) {
  if (a >= p.size()) {
    int old = p.size();
    p.resize(a + 1);
    p[0].resize(a + 1);
    for (int i = 1; i <= a; i++) {
      p[i].resize(a + 1);
      for (int j = old; j <= i; j++)
        p[i][j] = p[i - 1][j - 1] + p[i - j][j];
    }
  }
  return p[a][b];
}

/*

Given an integer n to partition and a 0-based rank x,
returns a vector of integers representing the x-th
lexicographically smallest partition of n (if values
in each partition were sorted in decreasing order).

examples: partition_by_rank(4, 0) => {1, 1, 1, 1}
          partition_by_rank(4, 3) => {3, 1}

*/

std::vector<int> partition_by_rank(int n, long long x) {
  std::vector<int> res;
  for (int i = n; i > 0; ) {
    int j = 1;
    for (;; j++) {
      long long cnt = partition_function(i, j);
      if (x < cnt) break;
      x -= cnt;
    }
    res.push_back(j);
    i -= j;
  }
  return res;
}

/*

Given a partition of an integer n (sum of all values
in vector p), returns a 0-based rank x of the partition
represented by p, considering partitions from least to
greatest in lexicographical order (if each partition
had values sorted in descending order).

examples: rank_by_partition({1, 1, 1, 1}) => 0
          rank_by_partition({3, 1})       => 3

*/

long long rank_by_partition(const std::vector<int> & p) {
  long long res = 0;
  int sum = 0;
  for (int i = 0; i < p.size(); i++) sum += p[i];
  for (int i = 0; i < p.size(); i++) {
    for (int j = 0; j < p[i]; j++)
      res += partition_function(sum, j);
    sum -= p[i];
  }
  return res;
}

/*

Calls the custom function f(vector) on all partitions
which consist of strictly *increasing* integers.
This will exclude partitions such as {1, 1, 1, 1}.

*/

template<class ReportFunction>
void gen_increasing_partitons(int left, int prev, int i,
                 ReportFunction f, std::vector<int> & p) {
  if (left == 0) {
    //warning: slow constructor - modify accordingly
    f(std::vector<int>(p.begin(), p.begin() + i));
    return;
  }
  for (p[i] = prev + 1; p[i] <= left; p[i]++)
    gen_increasing_partitons(left - p[i], p[i], i + 1, f, p);
}

template<class ReportFunction>
void gen_increasing_partitons(int n, ReportFunction f) {
  std::vector<int> partitions(n, 0);
  gen_increasing_partitons(n, 0, 0, f, partitions);
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

void print(const vector<int> & v) {
  for (int i = 0; i < v.size(); i++)
    cout << v[i] << " ";
  cout << "\n";
}

int main() {
  assert(count_partitions(5) == 7);
  assert(count_partitions(20) == 627);
  assert(count_partitions(30) == 5604);
  assert(count_partitions(50) == 204226);
  assert(count_partitions(100) == 190569292);

  {
    int n = 4;
    vector<int> a(n, 1);
    cout << "Partitions of " << n << ":\n";
    int cnt = 0;
    do {
      print(a);
      vector<int> b = partition_by_rank(n, cnt);
      assert(equal(a.begin(), a.end(), b.begin()));
      assert(rank_by_partition(a) == cnt);
      cnt++;
    } while (next_partition(a));
    cout << "\n";
  }

  {
    int n = 8;
    cout << "Increasing partitions of " << n << ":\n";
    gen_increasing_partitons(n, print);
  }
  return 0;
}
