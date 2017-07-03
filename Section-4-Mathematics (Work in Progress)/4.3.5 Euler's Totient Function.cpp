/*

Euler's totient function (or Euler's phi function) counts
the positive integers less than or equal to n that are
relatively prime to n. (These integers are sometimes
referred to as totatives of n.) Thus, phi(n) is the number
of integers k in the range [1, n] for which gcd(n, k) = 1.

E.g. if n = 9. Then gcd(9, 3) = gcd(9, 6) = 3 and gcd(9, 9)
= 9. The other six numbers in the range [1, 9], i.e. 1, 2,
4, 5, 7 and 8 are relatively prime to 9. Thus, phi(9) = 6.

*/

#include <vector>

int phi(int n) {
  int res = n;
  for (int i = 2; i * i <= n; i++)
    if (n % i == 0) {
      while (n % i == 0) n /= i;
      res -= res / i;
    }
  if (n > 1) res -= res / n;
  return res;
}

std::vector<int> phi_table(int n) {
  std::vector<int> res(n + 1);
  for (int i = 1; i <= n; i++)
    res[i] = i;
  for (int i = 1; i <= n; i++)
    for (int j = i + i; j <= n; j += i)
      res[j] -= res[i];
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  cout << phi(1) << "\n";       //1
  cout << phi(9) << "\n";       //6
  cout << phi(1234567) << "\n"; //1224720

  int n = 1000;
  vector<int> v = phi_table(n);
  for (int i = 0; i <= n; i++)
    assert(v[i] == phi(i));
  return 0;
}
