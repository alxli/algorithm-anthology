/*

4.5.6 - Enumerating Generic Combinatorial Sequences

The follow provides a universal method for enumerating
abstract combinatorial sequences in O(n^2) time.

*/

#include <vector>

class abstract_enumeration {

 protected:
  int range, length;

  abstract_enumeration(int r, int l): range(r), length(l) {}

  virtual long long count(const std::vector<int> & pre) {
    return 0;
  }

  std::vector<int> next(std::vector<int> & seq) {
    return from_number(to_number(seq) + 1);
  }

  long long total_count() {
    return count(std::vector<int>(0));
  }

public:
  long long to_number(const std::vector<int> & seq) {
    long long res = 0;
    for (int i = 0; i < seq.size(); i++) {
      std::vector<int> pre(seq.begin(), seq.end());
      pre.resize(i + 1);
      for (pre[i] = 0; pre[i] < seq[i]; ++pre[i])
        res += count(pre);
    }
    return res;
  }

  std::vector<int> from_number(long long x) {
    std::vector<int> seq(length);
    for (int i = 0; i < seq.size(); i++) {
      std::vector<int> pre(seq.begin(), seq.end());
      pre.resize(i + 1);
      for (pre[i] = 0; pre[i] < range; ++pre[i]) {
        long long cur = count(pre);
        if (x < cur) break;
        x -= cur;
      }
      seq[i] = pre[i];
    }
    return seq;
  }

  template<class ReportFunction>
  void enumerate(ReportFunction report) {
    long long total = total_count();
    for (long long i = 0; i < total; i++) {
      //assert(i == to_number(from_number(i));
      report(from_number(i));
    }
  }
};


class arrangements: public abstract_enumeration {
 public:
  arrangements(int n, int k) : abstract_enumeration(n, k) {}

  long long count(const std::vector<int> & pre) {
    int sz = pre.size();
    for (int i = 0; i < sz - 1; i++)
      if (pre[i] == pre[sz - 1]) return 0;
    long long res = 1;
    for (int i = 0; i < length - sz; i++)
      res *= range - sz - i;
    return res;
  }
};


class permutations: public arrangements {
 public:
  permutations(int n) : arrangements(n, n) {}
};


class combinations: public abstract_enumeration {

  std::vector<std::vector<long long> > binomial;

 public:
  combinations(int n, int k) : abstract_enumeration(n, k),
   binomial(n + 1, std::vector<long long>(n + 1, 0)) {
    for (int i = 0; i <= n; i++)
      for (int j = 0; j <= i; j++)
        binomial[i][j] = (j == 0) ? 1 :
              binomial[i - 1][j - 1] + binomial[i - 1][j];
  }

  long long count(const std::vector<int> & pre) {
    int sz = pre.size();
    if (sz >= 2 && pre[sz - 1] <= pre[sz - 2]) return 0;
    int last = sz > 0 ? pre[sz - 1] : -1;
    return binomial[range - 1 - last][length - sz];
  }
};


class partitions: public abstract_enumeration {

  std::vector<std::vector<long long> > p;

 public:
  partitions(int n) : abstract_enumeration(n + 1, n),
   p(n + 1, std::vector<long long>(n + 1, 0)) {
    std::vector<std::vector<long long> > pp(p);
    pp[0][0] = 1;
    for (int i = 1; i <= n; i++)
      for (int j = 1; j <= i; j++)
        pp[i][j] = pp[i - 1][j - 1] + pp[i - j][j];
    for (int i = 1; i <= n; i++)
      for (int j = 1; j <= n; j++)
        p[i][j] = pp[i][j] + p[i][j - 1];
  }

  long long count(const std::vector<int> & pre) {
    int size = pre.size(), sum = 0;
    for (int i = 0; i < pre.size(); i++) sum += pre[i];
    if (sum == range - 1) return 1;
    if (sum > range - 1 || size > 0 && pre[size - 1] == 0 ||
        size >= 2 && pre[size - 1] > pre[size - 2]) return 0;
    int last = size > 0 ? pre[size - 1] : range - 1;
    return p[range - 1 - sum][last];
  }
};


/*** Example Usage ***/

#include <iostream>
using namespace std;

void print(const std::vector<int> & v) {
  for (int i = 0; i < v.size(); i++)
    cout << v[i] << " ";
  cout << "\n";
}

int main() {
  cout << "Arrangement(3, 2):\n";
  arrangements arrg(3, 2);
  arrg.enumerate(print);

  cout << "Permutation(3):\n";
  permutations perm(3);
  perm.enumerate(print);

  cout << "Combination(4, 3):\n";
  combinations comb(4, 3);
  comb.enumerate(print);

  cout << "Partition(4):\n";
  partitions part(4);
  part.enumerate(print);
  return 0;
}
