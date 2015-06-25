/*

4.2.1 - Combinatorial Calculations

The meanings of the following functions can respectively be
found with quick searches online. All of them computes the
answer modulo m, since contest problems typically ask us for
this due to the actual answer being potentially very large.
All functions using tables to generate every answer below
n and k can be optimized using recursion and memoization.

Note: The following are only defined for nonnegative inputs.

*/

#include <vector>

typedef std::vector<std::vector<long long> > table;

//n! mod m in O(n)
long long factorial(int n, int m = 1000000007) {
  long long res = 1;
  for (int i = 2; i <= n; i++) res = (res * i) % m;
  return res % m;
}

//n! mod p, where p is a prime number, in O(p log n)
long long factorialp(int n, int p = 1000000007) {
  long long res = 1, h;
  while (n > 1) {
    res = (res * ((n / p) % 2 == 1 ? p - 1 : 1)) % p;
    h = n % p;
    for (int i = 2; i <= h; i++) res = (res * i) % p;
    n /= p;
  }
  return res % p;
}

//first n rows of pascal's triangle (mod m) in O(n^2)
table binomial_table(int n, long long m = 1000000007) {
  table t(n + 1);
  for (int i = 0; i <= n; i++)
    for (int j = 0; j <= i; j++)
      if (i < 2 || j == 0 || i == j)
        t[i].push_back(1);
      else
        t[i].push_back((t[i - 1][j - 1] + t[i - 1][j]) % m);
  return t;
}

//if the product of two 64-bit ints (a*a, a*b, or b*b) can
//overflow, you must use mulmod (multiplication by adding)
long long powmod(long long a, long long b, long long m) {
  long long x = 1, y = a;
  for (; b > 0; b >>= 1) {
    if (b & 1) x = (x * y) % m;
    y = (y * y) % m;
  }
  return x % m;
}

//n choose k (mod a prime number p) in O(min(k, n - k))
//powmod is used to find the mod inverse to get num / den % m
long long choose(int n, int k, long long p = 1000000007) {
  if (n < k) return 0;
  if (k > n - k) k = n - k;
  long long num = 1, den = 1;
  for (int i = 0; i < k; i++)
    num = (num * (n - i)) % p;
  for (int i = 1; i <= k; i++)
    den = (den * i) % p;
  return num * powmod(den, p - 2, p) % p;
}

//n multichoose k (mod a prime number p) in O(k)
long long multichoose(int n, int k, long long p = 1000000007) {
  return choose(n + k - 1, k, p);
}

//n permute k (mod m) on O(k)
long long permute(int n, int k, long long m = 1000000007) {
  if (n < k) return 0;
  long long res = 1;
  for (int i = 0; i < k; i++)
    res = (res * (n - i)) % m;
  return res % m;
}

//number of partitions of n (mod m) in O(n^2)
long long partitions(int n, long long m = 1000000007) {
  std::vector<long long> p(n + 1, 0);
  p[0] = 1;
  for (int i = 1; i <= n; i++)
    for (int j = i; j <= n; j++)
      p[j] = (p[j] + p[j - i]) % m;
  return p[n] % m;
}

//partitions of n into exactly k parts (mod m) in O(n * k)
long long partitions(int n, int k, long long m = 1000000007) {
  table t(n + 1, std::vector<long long>(k + 1, 0));
  t[0][1] = 1;
  for (int i = 1; i <= n; i++)
    for (int j = 1, h = k < i ? k : i; j <= h; j++)
      t[i][j] = (t[i - 1][j - 1] + t[i - j][j]) % m;
  return t[n][k] % m;
}

//unsigned Stirling numbers of the 1st kind (mod m) in O(n * k)
long long stirling1(int n, int k, long long m = 1000000007) {
  table t(n + 1, std::vector<long long>(k + 1, 0));
  t[0][0] = 1;
  for (int i = 1; i <= n; i++)
    for (int j = 1; j <= k; j++) {
      t[i][j] = ((i - 1) * t[i - 1][j]) % m;
      t[i][j] = (t[i][j] + t[i - 1][j - 1]) % m;
    }
  return t[n][k] % m;
}

//Stirling numbers of the 2nd kind (mod m) in O(n * k)
long long stirling2(int n, int k, long long m = 1000000007) {
  table t(n + 1, std::vector<long long>(k + 1, 0));
  t[0][0] = 1;
  for (int i = 1; i <= n; i++)
    for (int j = 1; j <= k; j++) {
      t[i][j] = (j * t[i - 1][j]) % m;
      t[i][j] = (t[i][j] + t[i - 1][j - 1]) % m;
    }
  return t[n][k] % m;
}

//Eulerian numbers of the 1st kind (mod m) in O(n * k)
//precondition: n > k
long long eulerian1(int n, int k, long long m = 1000000007) {
  if (k > n - 1 - k) k = n - 1 - k;
  table t(n + 1, std::vector<long long>(k + 1, 1));
  for (int j = 1; j <= k; j++) t[0][j] = 0;
  for (int i = 1; i <= n; i++)
    for (int j = 1; j <= k; j++) {
      t[i][j] = ((i - j) * t[i - 1][j - 1]) % m;
      t[i][j] = (t[i][j] + ((j + 1) * t[i - 1][j]) % m) % m;
    }
  return t[n][k] % m;
}

//Eulerian numbers of the 2nd kind (mod m) in O(n * k)
//precondition: n > k
long long eulerian2(int n, int k, long long m = 1000000007) {
  table t(n + 1, std::vector<long long>(k + 1, 1));
  for (int i = 1; i <= n; i++)
    for (int j = 1; j <= k; j++) {
      if (i == j) {
        t[i][j] = 0;
      } else {
        t[i][j] = ((j + 1) * t[i - 1][j]) % m;
        t[i][j] = (((2 * i - 1 - j) * t[i - 1][j - 1]) % m
                  + t[i][j]) % m;
      }
    }
  return t[n][k] % m;
}

//nth Catalan number (mod a prime number p) in O(n)
long long catalan(int n, long long p = 1000000007) {
  return choose(2 * n, n, p) * powmod(n + 1, p - 2, p) % p;
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  table t = binomial_table(10);
  for (int i = 0; i < t.size(); i++) {
    for (int j = 0; j < t[i].size(); j++)
      cout << t[i][j] << " ";
    cout << "\n";
  }
  assert(factorial(10)      == 3628800);
  assert(factorialp(123456) == 639390503);
  assert(choose(20, 7)      == 77520);
  assert(multichoose(20, 7) == 657800);
  assert(permute(10, 4)     == 5040);
  assert(partitions(4)      == 5);
  assert(partitions(100, 5) == 38225);
  assert(stirling1(4, 2)    == 11);
  assert(stirling2(4, 3)    == 6);
  assert(eulerian1(9, 5)    == 88234);
  assert(eulerian2(8, 3)    == 195800);
  assert(catalan(10)        == 16796);
  return 0;
}
