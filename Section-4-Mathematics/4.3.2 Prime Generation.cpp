/*

Generate prime numbers using the Sieve of Eratosthenes.

- sieve(n) returns a vector of all the primes less than or equal to n.
- sieve(lo, hi) returns a vector of all the primes in the range [lo, hi].

Time Complexity:
- O(n log(log(n))) per call to sieve(n).
- O(sqrt(hi)*log(log(hi - lo))) per call to sieve(lo, hi).

Space Complexity:
- O(n) auxiliary heap space per call to sieve(n).
- O(hi - lo + sqrt(hi)) auxiliary heap space per call to sieve(lo, hi).

*/

#include <cmath>
#include <vector>

std::vector<int> sieve(int n) {
  std::vector<bool> prime(n + 1, true);
  int sqrtn = ceil(sqrt(n));
  for (int i = 2; i <= sqrtn; i++) {
    if (prime[i]) {
      for (int j = i*i; j <= n; j += i) {
        prime[j] = false;
      }
    }
  }
  std::vector<int> res;
  for (int i = 2; i <= n; i++) {
    if (prime[i]) {
      res.push_back(i);
    }
  }
  return res;
}

std::vector<int> sieve(int lo, int hi) {
  int sqrt_hi = ceil(sqrt(hi)), fourth_root_hi = ceil(sqrt(sqrt_hi));
  std::vector<bool> prime1(sqrt_hi + 1, true), prime2(hi - lo + 1, true);
  for (int i = 2; i <= fourth_root_hi; i++) {
    if (prime1[i]) {
      for (int j = i*i; j <= sqrt_hi; j += i) {
        prime1[j] = false;
      }
    }
  }
  for (int i = 2, n = hi - lo; i <= sqrt_hi; i++) {
    if (prime1[i]) {
      for (int j = (lo / i)*i - lo; j <= n; j += i) {
        if (j >= 0 && j + lo != i) {
          prime2[j] = false;
        }
      }
    }
  }
  std::vector<int> res;
  for (int i = (lo > 1) ? lo : 2; i <= hi; i++) {
    if (prime2[i - lo]) {
      res.push_back(i);
    }
  }
  return res;
}

/*** Example Usage and Output:

sieve(n=10000000): 0.059s
atkins(n=10000000): 0.08s
sieve([1000000000, 1005000000]): 0.034s

***/

#include <ctime>
#include <iostream>
using namespace std;

int main() {
  int pmax = 10000000;
  vector<int> p;
  time_t start;
  double delta;

  start = clock();
  p = sieve(pmax);
  delta = (double)(clock() - start)/CLOCKS_PER_SEC;
  cout << "sieve(n=" << pmax << "): " << delta << "s" << endl;

  int l = 1000000000, h = 1005000000;
  start = clock();
  p = sieve(l, h);
  delta = (double)(clock() - start)/CLOCKS_PER_SEC;
  cout << "sieve([" << l << ", " << h << "]): " << delta << "s" << endl;
  return 0;
}
