/*

Common number theory operations relating to modular arithmetic.

- `gcd(a, b)` and `gcd2(a, b)` both return the greatest common divisor of `a` and `b` using the
  Euclidean algorithm. The implementations are mainly for educational purposes, as `std::gcd(a, b)`
  from `<numeric>` is available as of C++17 (`__gcd(a, b)` from `<algorithm>` in C++14 and earlier).
- `lcm(a, b)` returns the lowest common multiple of `a` and `b`. This implemention is mainly for
  educational purposes, as `std::lcm(a, b)` from `<numeric>` is available as of C++17.
- `extended_euclid(a, b)` and `extended_euclid2(a, b)` both return a pair $(x, y)$ of integers such
  that $\gcd(a, b) = ax + by$.
- `mod(a, b)` returns the value of `a` mod `b` under the true Euclidean definition of modulo, that
  is, the smallest nonnegative integer $m$ satisfying $a + bn = m$ for some integer $n$. Note that
  this is identical to the remainder operator `%` in C++ for nonnegative operands `a` and `b`, but
  the result will differ when an operand is negative.
- `mod_inverse(a, m)` and `mod_inverse2(a, m)` both return an integer $x$ such that
  $ax \equiv 1 \pmod m$, where the arguments must satisfy $m > 0$ and $\gcd(a, m) = 1$.
- `generate_inverse(p)` returns a vector $v$ of integers where for each index $i$ in the vector, $i
  \cdot \text{v[i]} \equiv 1 \pmod p$, where the argument $p$ is prime.
- `simple_restore(a, p)` and `garner_restore(a, p)` both return the solution $x$ for the system of
  simultaneous congruences $x \equiv \text{a[i]} \pmod{\text{p[i]}}$ for all indices $i$ in
  $[0, n)$, where `p` consists of pairwise coprime integers. The solution $x$ is guaranteed to be
  unique by the Chinese remainder theorem.

Time Complexity:
- O(log(a + b)) per call to `gcd(a, b)`, `gcd2(a, b)`, `lcm(a, b)`, `extended_euclid(a, b)`,
  `extended_euclid2(a, b)`, `mod_inverse(a, b)`, and `mod_inverse2(a, b)`.
- O(1) for `mod(a, b)`.
- O(p) for `generate_inverse(p)`.
- Exponential for `simple_restore(a, p)`.
- O(n^2) for `garner_restore(a, p)`.

Space Complexity:
- O(log(a + b)) auxiliary stack space for `gcd2(a, b)`, `extended_euclid2(a, b)`, and
  `mod_inverse2(a, b)`.
- O(p) auxiliary heap space for `generate_inverse(p)`.
- O(n) auxiliary heap space for `garner_restore(a, p)`.
- O(1) auxiliary space for all other operations.

*/

#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>

template<class Int>
Int gcd(Int a, Int b) {
  while (b != 0) {
    Int t = b;
    b = a % b;
    a = t;
  }
  return (a < 0 ? -a : a);
}

template<class Int>
Int gcd2(Int a, Int b) {
  return (b == 0) ? (a < 0 ? -a : a) : gcd2(b, a % b);
}

template<class Int>
Int lcm(Int a, Int b) {
  if (a == 0 || b == 0) {
    return 0;
  }
  Int res = a / gcd(a, b) * b;
  return (res < 0 ? -res : res);
}

template<class Int>
std::pair<Int, Int> extended_euclid(Int a, Int b) {
  Int x = 1, y = 0, x1 = 0, y1 = 1;
  while (b != 0) {
    Int q = a / b, prev_x1 = x1, prev_y1 = y1, prev_b = b;
    x1 = x - q * x1;
    y1 = y - q * y1;
    b = a - q * b;
    x = prev_x1;
    y = prev_y1;
    a = prev_b;
  }
  return (a > 0) ? std::pair<Int, Int>{x, y} : std::pair<Int, Int>{-x, -y};
}

template<class Int>
std::pair<Int, Int> extended_euclid2(Int a, Int b) {
  if (b == 0) {
    return (a > 0) ? std::pair<Int, Int>{1, 0} : std::pair<Int, Int>{-1, 0};
  }
  std::pair<Int, Int> r = extended_euclid2(b, a % b);
  return {r.second, r.first - a / b * r.second};
}

template<class Int>
Int mod(Int a, Int m) {
  Int r = a % m;
  return (r >= 0) ? r : (r + m);
}

template<class Int>
Int mod_inverse(Int a, Int m) {
  a = mod(a, m);
  return (a == 0) ? 0 : mod((1 - m * mod_inverse(m % a, a)) / a, m);
}

template<class Int>
Int mod_inverse2(Int a, Int m) {
  return mod(extended_euclid(a, m).first, m);
}

std::vector<int> generate_inverse(int p) {
  std::vector<int> res(p);
  res[1] = 1;
  for (int i = 2; i < p; i++) {
    res[i] = (p - (p / i) * res[p % i] % p) % p;
  }
  return res;
}

int64_t simple_restore(const std::vector<int> &a, const std::vector<int> &p) {
  int n = static_cast<int>(a.size());
  int64_t res = 0, m = 1;
  for (int i = 0; i < n; i++) {
    while (res % p[i] != a[i]) {
      res += m;
    }
    m *= p[i];
  }
  return res;
}

int64_t garner_restore(const std::vector<int> &a, const std::vector<int> &p) {
  int n = static_cast<int>(a.size());
  if (n == 0) {
    return 0;
  }
  std::vector<int64_t> x(a.begin(), a.end());
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < i; j++) {
      x[i] = mod_inverse(static_cast<int64_t>(p[j]), static_cast<int64_t>(p[i])) * (x[i] - x[j]);
    }
    x[i] = (x[i] % p[i] + p[i]) % p[i];
  }
  int64_t res = x[0], m = 1;
  for (int i = 1; i < n; i++) {
    m *= p[i - 1];
    res += x[i] * m;
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  {
    for (int steps = 0; steps < 10000; steps++) {
      int a = rand() % 200 - 10, b = rand() % 200 - 10, g = gcd(a, b);
      assert(g == gcd2(a, b));
      if (g == 1 && b > 1) {
        int inv = mod_inverse(a, b);
        assert(inv == mod_inverse2(a, b) && mod(a * inv, b) == 1);
      }
      auto res = extended_euclid(a, b);
      assert(res == extended_euclid2(a, b));
      auto [rx, ry] = res;
      assert(g == a * rx + b * ry);
    }
  }
  {
    int p = 17;
    auto res = generate_inverse(p);
    for (int i = 0; i < p; i++) {
      if (i > 0) {
        assert(mod(i * res[i], p) == 1);
      }
    }
  }
  {
    vector<int> a{2, 3, 1}, m{3, 4, 5};
    int n = static_cast<int>(a.size());
    int x = simple_restore(a, m);
    assert(x == garner_restore(a, m));
    for (int i = 0; i < n; i++) {
      assert(mod(x, m[i]) == a[i]);
    }
    assert(x == 11);
  }
  return 0;
}
