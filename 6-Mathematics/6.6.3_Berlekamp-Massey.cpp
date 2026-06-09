/*

The Berlekamp-Massey algorithm finds the shortest linear recurrence that generates a given sequence
over a field. Given the first terms of a sequence taken modulo a prime, it returns coefficients
$\Lambda_0, \dots, \Lambda_{L-1}$ of the minimal recurrence
$s_i = \Lambda_0 \, s_{i-1} + \Lambda_1 \, s_{i-2} + \dots + \Lambda_{L-1} \, s_{i-L} \pmod{p}$,
holding for every $i \geq L$. It scans the sequence once, maintaining a current candidate recurrence
and correcting it whenever a term fails to match the prediction, using the last position where a
correction was needed. The order $L$ never decreases, and $2L + 1$ terms suffice to certify a
recurrence of order $L$. This is the standard tool for guessing a linear recurrence from sampled
values, after which the sequence can be extended or its $n$-th term found by fast methods.

The code operates modulo the prime 998244353; change `MOD` to use a different prime field.

- `berlekamp_massey(s)` returns the coefficients of the shortest recurrence that the sequence `s`
  satisfies, as described above. The returned length is the order $L$ of the recurrence. An empty
  vector is returned when `s` is entirely zero (no recurrence is needed).

Time Complexity:
- O(n^2 + n log p) per call, where $n$ is the length of `s` and $p$ is the modulus.

Space Complexity:
- O(n) auxiliary heap space.

*/

#include <vector>

const long long MOD = 998244353;

long long powmod(long long b, long long e, long long m) {
  long long res = 1;
  for (b %= m; e > 0; e >>= 1) {
    if (e & 1) {
      res = res * b % m;
    }
    b = b * b % m;
  }
  return res;
}

std::vector<long long> berlekamp_massey(const std::vector<long long> &s) {
  int n = static_cast<int>(s.size()), len = 0, m = 0;
  std::vector<long long> cur(n), last(n), prev;
  cur[0] = last[0] = 1;
  long long last_delta = 1;
  for (int i = 0; i < n; i++) {
    m++;
    long long delta = s[i] % MOD;
    for (int j = 1; j <= len; j++) {
      delta = (delta + cur[j] * (s[i - j] % MOD)) % MOD;
    }
    if (delta == 0) {
      continue;
    }
    prev = cur;
    long long coef = delta * powmod(last_delta, MOD - 2, MOD) % MOD;
    for (int j = m; j < n; j++) {
      cur[j] = (cur[j] - coef * last[j - m] % MOD + MOD) % MOD;
    }
    if (2 * len > i) {
      continue;
    }
    len = i + 1 - len;
    last = prev;
    last_delta = delta;
    m = 0;
  }
  cur.resize(len + 1);
  cur.erase(cur.begin());
  for (long long &x : cur) {
    x = (MOD - x) % MOD;
  }
  return cur;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Fibonacci: s[i] = s[i-1] + s[i-2].
  vector<long long> fib{1, 1, 2, 3, 5, 8, 13, 21};
  assert((berlekamp_massey(fib) == vector<long long>{1, 1}));

  // Geometric: s[i] = 2 * s[i-1].
  vector<long long> geo{1, 2, 4, 8, 16, 32};
  assert((berlekamp_massey(geo) == vector<long long>{2}));

  // Tribonacci: s[i] = s[i-1] + s[i-2] + s[i-3].
  vector<long long> trib{0, 1, 1, 2, 4, 7, 13, 24, 44};
  assert((berlekamp_massey(trib) == vector<long long>{1, 1, 1}));
  return 0;
}
