/* 4.2.1 - Binomial Coefficients and Factorials */

#include <vector>

typedef std::vector<std::vector<long long> > table;

//first n rows of pascal's triangle in O(n^2)
table binomial_table(int n) {
  table t(n + 1);
  for (int i = 0; i <= n; i++)
    for (int j = 0; j <= i; j++)
      if (i < 2 || j == 0 || i == j)
        t[i].push_back(1);
      else
        t[i].push_back(t[i - 1][j - 1] + t[i - 1][j]);
  return t;
}

//n choose k in O(min(k, n - k))
long long n_choose_k(long long n, long long k) {
  if (k > n - k) k = n - k;
  long long res = 1;
  for (int i = 0; i < k; i++)
    res = res * (n - i) / (i + 1);
  return res;
}

//n! % m in O(n)
int factorial(int n, int m) {
  long long res = 1;
  for (int i = 2; i <= n; i++)
    res = (res * i) % m;
  return (int)(res % m);
}

//n! % p where p is prime in O(p*log(n))
int factorialp(int n, int p) {
  int res = 1;
  while (n > 1) {
    res = (res * ((n / p) % 2 == 1 ? p - 1 : 1)) % p;
    for (int i = 2; i <= n % p; i++)
      res = (res * i) % p;
    n /= p;
  }
  return res % p;
}

/* Example Usage */

#include <iostream>
using namespace std;

int main() {
  table t = binomial_table(10);
  for (int i = 0; i < t.size(); i++) {
    for (int j = 0; j < t[i].size(); j++)
      cout << t[i][j] << " ";
    cout << "\n";
  }
  cout << n_choose_k(20, 7) << "\n";       //77520
  cout << factorial(10, 10000000) << "\n"; //3628800
  cout << factorialp(1234, 10007) << "\n"; //4965
  return 0;
}
