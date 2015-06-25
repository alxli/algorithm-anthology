/*

4.2.1 - Combinatorial Calculations

The meanings of the following functions can respectively be
found with quick searches online. All of them computes the
answer modulo m, since contest problems typically ask us for
this due to the actual answer being potentially very large.
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
long long factorialp(int n, int p) {
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

//n choose k (mod m) in O(min(k, n - k))
//powmod is used to find the mod inverse to get num / den % m
long long n_choose_k(int n, int k, long long m = 1000000007) {
  if (n < k) return 0;
  if (k > n - k) k = n - k;
  long long num = 1, den = 1;
  for (int i = 0; i < k; i++)
    num = (num * (n - i)) % m;
  for (int i = 1; i <= k; i++)
    den = (den * i) % m;
  return num * powmod(den, m - 2, m) % m;
}

//n multichoose k (mod m) in O(k)
long long n_multichoose_k(int n, int k, long long m = 1000000007) {
  return n_choose_k(n + k - 1, k, m);
}

//n permute k (mod m) on O(k)
long long n_permute_k(int n, int k, long long m = 1000000007) {
  if (n < k) return 0;
  long long res = 1;
  for (int i = 0; i < k; i++)
    res = (res * (n - i)) % m;
  return res % m;
}

//number of partitions of n in O(n^2)
long long partitions(int n, long long m = 1000000007) {
  std::vector<long long> p(n + 1, 0);
  p[0] = 1;
  for (int i = 1; i <= n; i++)
    for (int j = i; j <= n; j++)
      p[j] = (p[j] + p[j - i]) % m;
  return p[n] % m;
}


const int MAXN = 500, MAXK = 500;
int npartk[MAXN][MAXK] = {0}; //memoization

//size k partitions of n in O(min(n, k))
//see: http://wcipeg.com/problem/ccc15j5
int partitions(int n, int k, long long m = 1000000007) {
  if (n < k) return 0;
  if (n == k || k < 2) return 1;
  if (npartk[n][k] != 0) return npartk[n][k];
  npartk[n][k] = partitions(n - 1, k - 1, m);
  npartk[n][k] += partitions(n - k, k, m);
  return (npartk[n][k] %= m);
}


/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  table t = binomial_table(10);
  for (int i = 0; i < t.size(); i++) {
    for (int j = 0; j < t[i].size(); j++)
      cout << t[i][j] << " ";
    cout << "\n";
  }
  cout << factorial(10, 10000000) << "\n"; //3628800
  cout << factorialp(1234, 10007) << "\n"; //4965
  cout << n_choose_k(20, 7) << "\n";       //77520
  cout << n_multichoose_k(20, 7) << "\n";  //657800
  cout << n_permute_k(10, 4) << "\n";      //5040
  cout << partitions(4) << "\n";           //5
  cout << partitions(100, 5) << "\n";      //38225
  return 0;
}
