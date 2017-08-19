/*

The following functions implement common operations in combinatorics. All input
arguments must be non-negative. All return values and table entries are computed
as 64-bit integers modulo an input argument m or p.

- factorial(n, m) returns n! mod m.
- factorialp(n, p) returns n! mod p, where p is prime.
- binomial_table(n, m) returns rows 0 to n of Pascal's triangle as a table t
  such that t[i][j] is equal to (i choose j) mod m.
- permute(n, k, m) returns (n permute k) mod m.
- choose(n, k, p) returns (n choose k) mod p, where p is prime.
- multichoose(n, k, p) returns (n multi-choose k) mod p, where p is prime.
- catalan(n, p) returns the nth Catalan number mod p, where p is prime.
- partitions(n, m) returns the number of partitions of n, mod m.
- partitions(n, k, m) returns the number of partitions of n into k parts, mod m.
- stirling1(n, k, m) returns the (n, k) unsigned Stirling number of the 1st kind
  mod m.
- stirling2(n, k, m) returns the (n, k) Stirling number of the 2nd kind mod m.
- eulerian1(n, k, m) returns the (n, k) Eulerian number of the 1st kind mod m,
  where n > k.
- eulerian2(n, k, m) returns the (n, k) Eulerian number of the 2nd kind mod m,
  where n > k.

Time Complexity:
- O(n) for factorial(n, m).
- O(p log n) for factorialp(n, p).
- O(n^2) for binomial_table(n, m).
- O(k) for permute(n, k, p).
- O(min(k, n - k)) for choose(n, k, p).
- O(k) for multichoose(n, k, p).
- O(n) for catalan(n, p).
- O(n^2) for partitions(n, m).
- O(n*k) for partitions(n, k, m), stirling1(n, k, m), stirling2(n, k, m),
  eulerian1(n, k, m), and eulerian2(n, k, m).

Space Complexity:
- O(n^2) auxiliary heap space for binomial_table(n, m).
- O(n*k) auxiliary heap space for partitions(n, k, m), stirling1(n, k, m),
  stirling2(n, k, m), eulerian1(n, k, m), and eulerian2(n, k, m).
- O(1) auxiliary for all other operations.

*/

#include <vector>

typedef long long int64;
typedef std::vector<std::vector<int64> > table;

int64 factorial(int n, int m = 1000000007) {
  int64 res = 1;
  for (int i = 2; i <= n; i++) {
    res = (res*i) % m;
  }
  return res % m;
}

int64 factorialp(int64 n, int64 p = 1000000007) {
  int64 res = 1;
  while (n > 1) {
    if (n / p % 2 == 1) {
      res = res*(p - 1) % p;
    }
    int max = n % p;
    for (int i = 2; i <= max; i++) {
      res = (res*i) % p;
    }
    n /= p;
  }
  return res % p;
}

table binomial_table(int n, int64 m = 1000000007) {
  table t(n + 1);
  for (int i = 0; i <= n; i++) {
    for (int j = 0; j <= i; j++) {
      if (i < 2 || j == 0 || i == j) {
        t[i].push_back(1);
      } else {
        t[i].push_back((t[i - 1][j - 1] + t[i - 1][j]) % m);
      }
    }
  }
  return t;
}

int64 permute(int n, int k, int64 m = 1000000007) {
  if (n < k) {
    return 0;
  }
  int64 res = 1;
  for (int i = 0; i < k; i++) {
    res = res*(n - i) % m;
  }
  return res % m;
}

int64 mulmod(int64 x, int64 n, int64 m) {
  int64 a = 0, b = x % m;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      a = (a + b) % m;
    }
    b = (b << 1) % m;
  }
  return a % m;
}

int64 powmod(int64 x, int64 n, int64 m) {
  int64 a = 1, b = x;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      a = mulmod(a, b, m);
    }
    b = mulmod(b, b, m);
  }
  return a % m;
}

int64 choose(int n, int k, int64 p = 1000000007) {
  if (n < k) {
    return 0;
  }
  if (k > n - k) {
    k = n - k;
  }
  int64 num = 1, den = 1;
  for (int i = 0; i < k; i++) {
    num = num*(n - i) % p;
  }
  for (int i = 1; i <= k; i++) {
    den = den*i % p;
  }
  return num*powmod(den, p - 2, p) % p;
}

int64 multichoose(int n, int k, int64 p = 1000000007) {
  return choose(n + k - 1, k, p);
}

int64 catalan(int n, int64 p = 1000000007) {
  return choose(2*n, n, p)*powmod(n + 1, p - 2, p) % p;
}

int64 partitions(int n, int64 m = 1000000007) {
  std::vector<int64> t(n + 1, 0);
  t[0] = 1;
  for (int i = 1; i <= n; i++) {
    for (int j = i; j <= n; j++) {
      t[j] = (t[j] + t[j - i]) % m;
    }
  }
  return t[n] % m;
}

int64 partitions(int n, int k, int64 m = 1000000007) {
  table t(n + 1, std::vector<int64>(k + 1, 0));
  t[0][1] = 1;
  for (int i = 1; i <= n; i++) {
    for (int j = 1, h = k < i ? k : i; j <= h; j++) {
      t[i][j] = (t[i - 1][j - 1] + t[i - j][j]) % m;
    }
  }
  return t[n][k] % m;
}

int64 stirling1(int n, int k, int64 m = 1000000007) {
  table t(n + 1, std::vector<int64>(k + 1, 0));
  t[0][0] = 1;
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= k; j++) {
      t[i][j] = (i - 1)*t[i - 1][j] % m;
      t[i][j] = (t[i][j] + t[i - 1][j - 1]) % m;
    }
  }
  return t[n][k] % m;
}

int64 stirling2(int n, int k, int64 m = 1000000007) {
  table t(n + 1, std::vector<int64>(k + 1, 0));
  t[0][0] = 1;
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= k; j++) {
      t[i][j] = j*t[i - 1][j] % m;
      t[i][j] = (t[i][j] + t[i - 1][j - 1]) % m;
    }
  }
  return t[n][k] % m;
}

int64 eulerian1(int n, int k, int64 m = 1000000007) {
  if (k > n - 1 - k) {
    k = n - 1 - k;
  }
  table t(n + 1, std::vector<int64>(k + 1, 1));
  for (int j = 1; j <= k; j++) {
    t[0][j] = 0;
  }
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= k; j++) {
      t[i][j] = (i - j)*t[i - 1][j - 1] % m;
      t[i][j] = (t[i][j] + ((j + 1)*t[i - 1][j] % m)) % m;
    }
  }
  return t[n][k] % m;
}

int64 eulerian2(int n, int k, int64 m = 1000000007) {
  table t(n + 1, std::vector<int64>(k + 1, 1));
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= k; j++) {
      if (i == j) {
        t[i][j] = 0;
      } else {
        t[i][j] = (j + 1)*t[i - 1][j] % m;
        t[i][j] = ((2*i - 1 - j)*t[i - 1][j - 1] % m + t[i][j]) % m;
      }
    }
  }
  return t[n][k] % m;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  table t = binomial_table(10);
  for (int i = 0; i < (int)t.size(); i++) {
    for (int j = 0; j < (int)t[i].size(); j++) {
      assert(t[i][j] == choose(i, j));
    }
  }
  assert(factorial(10) == 3628800);
  assert(factorialp(123456) == 639390503);
  assert(permute(10, 4) == 5040);
  assert(choose(20, 7) == 77520);
  assert(multichoose(20, 7) == 657800);
  assert(catalan(10) == 16796);
  assert(partitions(4) == 5);
  assert(partitions(100, 5) == 38225);
  assert(stirling1(4, 2) == 11);
  assert(stirling2(4, 3) == 6);
  assert(eulerian1(9, 5) == 88234);
  assert(eulerian2(8, 3) == 195800);
  return 0;
}
