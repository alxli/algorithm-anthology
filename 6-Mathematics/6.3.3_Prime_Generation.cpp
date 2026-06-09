/*

Generate prime numbers using the Sieve of Eratosthenes.

- `sieve(n)` returns a vector of all the primes less than or equal to `n`.
- `sieve(lo, hi)` returns a vector of all the primes in the range `[lo, hi]`.

Time Complexity:
- O(n log(log(n))) per call to `sieve(n)`.
- O(sqrt(`hi`)*log(log(`hi` - `lo`))) per call to `sieve(lo, hi)`.

Space Complexity:
- O(n) auxiliary heap space per call to `sieve(n)`.
- O(`hi` - `lo` + sqrt(`hi`)) auxiliary heap space per call to `sieve(lo, hi)`.

*/

#include <algorithm>
#include <cmath>
#include <vector>

std::vector<int> sieve(int n) {
  if (n < 2) {
    return {};
  }
  std::vector<bool> prime(n + 1, true);
  for (int i = 2; i <= n / i; i++) {
    if (prime[i]) {
      for (long long j = 1LL * i * i; j <= n; j += i) {
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
  if (hi < 2 || lo > hi) {
    return {};
  }
  int sqrt_hi = sqrt(hi);
  while (1LL * (sqrt_hi + 1) * (sqrt_hi + 1) <= hi) {
    sqrt_hi++;
  }
  int fourth_root_hi = sqrt(sqrt_hi);
  while (1LL * (fourth_root_hi + 1) * (fourth_root_hi + 1) <= sqrt_hi) {
    fourth_root_hi++;
  }
  std::vector<bool> prime1(sqrt_hi + 1, true), prime2(hi - lo + 1, true);
  for (int i = 2; i <= fourth_root_hi; i++) {
    if (prime1[i]) {
      for (long long j = 1LL * i * i; j <= sqrt_hi; j += i) {
        prime1[j] = false;
      }
    }
  }
  for (int i = 2; i <= sqrt_hi; i++) {
    if (prime1[i]) {
      long long start = std::max(1LL * i * i, ((static_cast<long long>(lo) + i - 1) / i) * i);
      for (long long j = start; j <= hi; j += i) {
        prime2[j - lo] = false;
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

sieve(n=10000000): 0.042641s
sieve([1000000000, 1005000000]): 0.01969s

***/

#include <ctime>
#include <iostream>
using namespace std;

int main() {
  int pmax = 10000000;
  time_t start;
  double delta;

  start = clock();
  auto p = sieve(pmax);
  delta = static_cast<double>((clock() - start)) / CLOCKS_PER_SEC;
  cout << "sieve(n=" << pmax << "): " << delta << "s" << endl;

  int l = 1000000000, h = 1005000000;
  start = clock();
  p = sieve(l, h);
  delta = static_cast<double>((clock() - start)) / CLOCKS_PER_SEC;
  cout << "sieve([" << l << ", " << h << "]): " << delta << "s" << endl;
  return 0;
}
