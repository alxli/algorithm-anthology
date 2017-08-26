/*

Euler's totient function phi(n) returns the number of positive integers less
than or equal to n that are relatively prime to n. That is, phi(n) is the number
of integers k in the range [1, n] for which gcd(n, k) = 1. The computation of
phi(1..n) can be performed simultaneously, as done so by phi_table(n) which
returns a vector v such that v[i] stores phi(i) for i in the range [0, n].

Time Complexity:
- O(n log(log(n))) per call to phi(n) and phi_table(n).

Space Complexity:
- O(1) auxiliary space for phi(n).
- O(n) auxiliary heap space for phi_table(n).

*/

#include <vector>

int phi(int n) {
  int res = n;
  for (int i = 2; i*i <= n; i++) {
    if (n % i == 0) {
      while (n % i == 0) {
        n /= i;
      }
      res -= res/i;
    }
  }
  if (n > 1) {
    res -= res/n;
  }
  return res;
}

std::vector<int> phi_table(int n) {
  std::vector<int> res(n + 1);
  for (int i = 0; i <= n; i++) {
    res[i] = i;
  }
  for (int i = 1; i <= n; i++) {
    for (int j = 2*i; j <= n; j += i) {
      res[j] -= res[i];
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(phi(1) == 1);
  assert(phi(9) == 6);
  assert(phi(1234567) == 1224720);
  const int n = 1000;
  vector<int> v = phi_table(n);
  for (int i = 0; i <= n; i++) {
    assert(v[i] == phi(i));
  }
  return 0;
}
