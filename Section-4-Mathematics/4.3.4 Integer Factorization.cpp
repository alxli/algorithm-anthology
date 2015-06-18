/* 4.3.4 - Integer Factorization */

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <map>
#include <stdint.h>
#include <vector>

/*

Trial division in O(sqrt(n))

returns: map of prime_divisor to exponent
e.g. prime_factorize(15435) => {(3,2),(5,1),(7,3)}
     because 3^2 * 5^1 * 7^3 = 15435

*/

template<class Int>
std::map<Int, int> prime_factorize(Int n) {
  std::map<Int, int> res;
  for (Int d = 2; d > 1; ) {
    int power = 0;
    while (n % d == 0) {
      power++;
      n /= d;
    }
    if (power > 0) res[d] = power;
    d++;
    if (d * d > n) d = n;
  }
  return res;
}


/*

Trial division in O(sqrt(n))

Returns a sorted vector of all divisors of n.
e.g. get_all_divisors(28) => {1, 2, 4, 7, 14, 28}

*/

template<class Int>
std::vector<Int> get_all_divisors(Int n) {
  std::vector<Int> res;
  for (int d = 1; d * d <= n; d++) {
    if (n % d == 0) {
      res.push_back(d);
      if (d * d != n)
        res.push_back(n / d);
    }
  }
  std::sort(res.begin(), res.end());
  return res;
}


/*

Fermat's Method ~ O(sqrt(N))

Given a number n, returns one factor of n that is
not necessary prime. Fermat's algorithm is pretty
good when the number you wish to factor has two
factors very near to sqrt(n). Otherwise, it is just
as slow as the basic trial division algorithm.

e.g. 14917627 => 1 (it's a prime), or
     1234567 => 127 (because 127*9721 = 1234567)

*/

long long fermat(long long n) {
  if (n % 2 == 0) return 2;
  long long x = sqrt(n), y = 0;
  long long r = x * x - y * y - n;
  while (r != 0) {
    if (r < 0) {
      r += x + x + 1;
      x++;
    } else {
      r -= y + y + 1;
      y++;
    }
  }
  return x != y ? x - y : x + y;
}


/*

Pollard's rho Algorithm with Brent's Optimization

Brent's algorithm is a much faster variant of Pollard's
rho algorithm using Brent's cycle-finding method. The
following function returns a (not necessarily prime) factor
of n, or n if n is prime. Note that this is not necessarily
guaranteed to always work perfectly. brent(9) may return 9
instead of 3. However, it works well when coupled with trial
division in the function prime_factorize_big() below.

*/

uint64_t mulmod(uint64_t a, uint64_t b, uint64_t m) {
  uint64_t x = 0, y = a % m;
  for (; b > 0; b >>= 1) {
    if (b & 1) x = (x + y) % m;
    y = (y << 1) % m;
  }
  return x % m;
}

//5 calls to rand() is unnecessary if RAND_MAX is 2^31-1
uint64_t rand64u() {
  return ((uint64_t)(rand() & 0xf) << 60) |
         ((uint64_t)(rand() & 0x7fff) << 45) |
         ((uint64_t)(rand() & 0x7fff) << 30) |
         ((uint64_t)(rand() & 0x7fff) << 15) |
         ((uint64_t)(rand() & 0x7fff));
}

long long gcd(long long a, long long b) {
  return b == 0 ? a : gcd(b, a % b);
}

long long brent(long long n) {
  if (n % 2 == 0) return 2;
  long long y = rand64u() % (n - 1) + 1;
  long long c = rand64u() % (n - 1) + 1;
  long long m = rand64u() % (n - 1) + 1;
  long long g = 1, r = 1, q = 1, ys = 0, hi = 0, x = 0;
  while (g == 1) {
    x = y;
    for (int i = 0; i < r; i++)
      y = (mulmod(y, y, n) + c) % n;
    for (long long k = 0; k < r && g == 1; k += m) {
      ys = y;
      hi = std::min(m, r - k);
      for (int j = 0; j < hi; j++) {
        y = (mulmod(y, y, n) + c) % n;
        q = mulmod(q, abs(x - y), n);
      }
      g = gcd(q, n);
    }
    r *= 2;
  }
  if (g == n) do {
    ys = (mulmod(ys, ys, n) + c) % n;
    g = gcd(abs(x - ys), n);
  } while (g <= 1);
  return g;
}


/*

Combines Brent's method with trial division to efficiently
generate the prime factorization of large integers.

returns: map of prime_divisor to exponent

e.g. prime_factorize(15435) => {(3,2),(5,1),(7,3)}
     because 3^2 * 5^1 * 7^3 = 15435

*/

std::map<long long, int> prime_factorize_big(long long n) {
  std::map<long long, int> res;
  if (n <= 0) return res;
  if (n == 1) { res[1] = 1; return res; }
  int power;
  for (power = 0; n % 2 == 0; n /= 2) power++;
  if (power > 0) res[2] = power;
  for (power = 0; n % 3 == 0; n /= 3) power++;
  if (power > 0) res[3] = power;
  int mx = 1000000; //trial division for factors <= 1M
  for (int i = 5, w = 2; i <= mx; i += w, w = 6 - w) {
    for (power = 0; n % i == 0; n /= i) power++;
    if (power > 0) res[i] = power;
  }
  for (long long p = 0, p1; n > mx; n /= p1) { //brent
    for (p1 = n; p1 != p; p1 = brent(p)) p = p1;
    res[p1]++;
  }
  if (n != 1) res[n]++;
  return res;
}


/*** Example Usage ***/

#include <cassert>
#include <iostream>
#include <ctime>
using namespace std;

template<class It> void print(It lo, It hi) {
  while (lo != hi) cout << *(lo++) << " ";
  cout << "\n";
}

template<class It> void printp(It lo, It hi) {
  for (; lo != hi; ++lo)
    cout << "(" << lo->first << "," << lo->second << ") ";
  cout << "\n";
}

int main() {
  srand(time(0));

  map<int, int> m = prime_factorize(15435);
  printp(m.begin(), m.end());

  vector<int> d;

  d = get_all_divisors(28);
  print(d.begin(), d.end());

  long long n = 1000003ll*100000037ll;
  assert(fermat(n) == 1000003ll);

  map<long long, int> f = prime_factorize_big(n);
  printp(f.begin(), f.end());

  return 0;
}
