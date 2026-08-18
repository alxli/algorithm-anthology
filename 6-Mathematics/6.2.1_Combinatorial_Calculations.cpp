/*

The following functions implement common operations in combinatorics. All size and count inputs must
be nonnegative, and all moduli must be positive. All return values and table entries are computed as
64-bit integers modulo an input argument $m$ or $p$.

- `factorial(n, m = MOD)` returns $n! \bmod m$.
- `factorial_without_p(n, p = MOD)` returns $n!$ modulo the prime $p$ after removing every factor of
  $p$ from the factorial.
- `legendre(n, p)` returns the exponent of the prime $p$ in the factorization of $n!$, where $p$
  must be at least $2$.
- `binomial_table(n, m = MOD)` returns the first $n + 1$ rows of Pascal's triangle as a
  two-dimensional vector $t$ such that $t[i][j] = \binom{i}{j} \bmod m$.
- `permute(n, k, m = MOD)` returns $(n \mathbin{\text{permute}} k) \bmod m$.
- `choose(n, k, p = MOD)` returns $\binom{n}{k} \bmod p$, where $p$ is prime and $n < p$.
- `choose_lucas(n, k, p = MOD)` returns $\binom{n}{k} \bmod p$ for any 64-bit `n` and `k`, where $p$
  is prime. It is only practical for a small $p$, since each base-$p$ digit costs a call to
  `choose()`.
- `multichoose(n, k, p = MOD)` returns $(n \mathbin{\text{multichoose}} k) \bmod p$, where $p$ is
  prime and $n + k - 1 < p$ when $n > 0$.
- `catalan(n, p = MOD)` returns the $n$-th Catalan number mod $p$, where $p$ is prime and $2n < p$.
- `partitions(n, m = MOD)` returns the number of partitions of $n$, mod $m$.
- `partitions(n, k, m = MOD)` returns the number of partitions of $n$ into $k$ parts, mod $m$.
- `stirling1(n, k, m = MOD)` returns the $(n, k)$ unsigned Stirling number of the 1st kind mod $m$.
- `stirling2(n, k, m = MOD)` returns the $(n, k)$ Stirling number of the 2nd kind mod $m$.
- `eulerian1(n, k, m = MOD)` returns the $(n, k)$ Eulerian number of the 1st kind mod $m$, where
  $n > k$.
- `eulerian2(n, k, m = MOD)` returns the $(n, k)$ Eulerian number of the 2nd kind mod $m$, where
  $n > k$.

Legendre's formula sums $\lfloor n/p^i \rfloor$ over every $i \geq 1$, counting how many of
$1, \dots, n$ are divisible by each power of $p$, which totals the exponent of $p$ in $n!$. It
answers questions such as how many trailing zeros $n!$ has in base $p$. Paired with Kummer's
theorem, which equates the exponent of $p$ in $\binom{n}{k}$ with the number of carries when adding
$k$ and $n - k$ in base $p$, it also decides when a binomial coefficient is divisible by $p$.

Lucas' theorem reduces a binomial coefficient modulo a prime to a product over base-$p$ digits:
$\binom{n}{k} \equiv \prod_i \binom{n_i}{k_i} \pmod p$, where $n_i$ and $k_i$ are the $i$-th digits
of $n$ and $k$ in base $p$. The product is $0$ as soon as some $k_i$ exceeds $n_i$. For a composite
modulus, factor it into prime powers, apply the generalized version of the theorem to each factor,
and recombine the residues with the Chinese remainder theorem of section 6.3.2.

Every routine here recomputes its answer from scratch, which is what suits a handful of values or a
modulus that varies between calls. When a solution instead needs thousands of binomial coefficients
against one fixed modulus, precompute factorials and inverse factorials: the combinatorics tables of
section 6.3.3 amortize O(n) of table growth and then answer each query in O(1).

Overflow warning: Modular products use ordinary `int64_t` multiplication, so the square of the
chosen modulus must fit in `int64_t`.

Time Complexity:
- O(n) per call to `factorial()`.
- O(p log_p(n)) per call to `factorial_without_p()`.
- O(log_p(n)) per call to `legendre()`.
- O(n^2) per call to `binomial_table()`.
- O(k) per call to `permute()`.
- O(min(k, n - k)) per call to `choose()`.
- O(p log_p(n)) per call to `choose_lucas()`.
- O(k) per call to `multichoose()`.
- O(n) per call to `catalan()`.
- O(n^2) per call to `partitions(n, m)`.
- O(n*k) per call to `partitions(n, k, m)`, `stirling1()`, `stirling2()`, `eulerian1()`, and
  `eulerian2()`.

Space Complexity:
- O(n^2) auxiliary for `binomial_table(n, m)`.
- O(n*k) auxiliary for `partitions(n, k, m)`, `stirling1(n, k, m)`, `stirling2(n, k, m)`,
  `eulerian1(n, k, m)`, and `eulerian2(n, k, m)`.
- O(1) auxiliary for all other operations.

*/

#include <cassert>
#include <cstdint>
#include <vector>

const int64_t MOD = 1000000007;

int64_t factorial(int n, int64_t m = MOD) {
  int64_t res = 1;
  for (int i = 2; i <= n; i++) {
    res = (res * i) % m;
  }
  return res % m;
}

int64_t factorial_without_p(int64_t n, int64_t p = MOD) {
  int64_t res = 1;
  while (n > 1) {
    if (n / p % 2 == 1) {
      res = res * (p - 1) % p;
    }
    int max = n % p;
    for (int i = 2; i <= max; i++) {
      res = (res * i) % p;
    }
    n /= p;
  }
  return res % p;
}

int64_t legendre(int64_t n, int64_t p) {
  assert(p >= 2);
  int64_t res = 0;
  for (int64_t q = n / p; q > 0; q /= p) {  // Dividing repeatedly avoids overflowing p^i.
    res += q;
  }
  return res;
}

std::vector<std::vector<int64_t>> binomial_table(int n, int64_t m = MOD) {
  std::vector<std::vector<int64_t>> t(n + 1);
  for (int i = 0; i <= n; i++) {
    for (int j = 0; j <= i; j++) {
      if (i < 2 || j == 0 || i == j) {
        t[i].push_back(1);
      } else {
        t[i].push_back((t[i - 1][j - 1] + t[i - 1][j]) % m);
      }
    }
  }
  return t;
}

int64_t permute(int n, int k, int64_t m = MOD) {
  if (n < k) {
    return 0;
  }
  int64_t res = 1;
  for (int i = 0; i < k; i++) {
    res = res * (n - i) % m;
  }
  return res % m;
}

int64_t powmod(int64_t x, int64_t n, int64_t m) {
  int64_t res = 1 % m;
  for (x %= m; n > 0; n >>= 1) {
    if (n & 1) {
      res = res * x % m;
    }
    x = x * x % m;
  }
  return res;
}

int64_t choose(int n, int k, int64_t p = MOD) {
  if (n < k) {
    return 0;
  }
  if (k > n - k) {
    k = n - k;
  }
  int64_t num = 1, den = 1;
  for (int i = 0; i < k; i++) {
    num = num * (n - i) % p;
  }
  for (int i = 1; i <= k; i++) {
    den = den * i % p;
  }
  return num * powmod(den, p - 2, p) % p;
}

int64_t choose_lucas(int64_t n, int64_t k, int64_t p = MOD) {
  if (k < 0 || k > n) {
    return 0;
  }
  int64_t res = 1;
  while (n > 0) {  // Since k never exceeds n, k is also exhausted when n reaches 0.
    int64_t ni = n % p, ki = k % p;
    if (ki > ni) {
      return 0;
    }
    res = res * choose(static_cast<int>(ni), static_cast<int>(ki), p) % p;
    n /= p;
    k /= p;
  }
  return res;
}

int64_t multichoose(int n, int k, int64_t p = MOD) {
  if (n == 0) {
    return k == 0;
  }
  return choose(n + k - 1, k, p);
}

int64_t catalan(int n, int64_t p = MOD) {
  return choose(2 * n, n, p) * powmod(n + 1, p - 2, p) % p;
}

int64_t partitions(int n, int64_t m = MOD) {
  std::vector<int64_t> t(n + 1);
  t[0] = 1;
  for (int i = 1; i <= n; i++) {
    for (int j = i; j <= n; j++) {
      t[j] = (t[j] + t[j - i]) % m;
    }
  }
  return t[n] % m;
}

int64_t partitions(int n, int k, int64_t m = MOD) {
  std::vector<std::vector<int64_t>> t(n + 1, std::vector<int64_t>(k + 1));
  t[0][0] = 1;
  for (int i = 1; i <= n; i++) {
    for (int j = 1, h = k < i ? k : i; j <= h; j++) {
      t[i][j] = (t[i - 1][j - 1] + t[i - j][j]) % m;
    }
  }
  return t[n][k] % m;
}

int64_t stirling1(int n, int k, int64_t m = MOD) {
  std::vector<std::vector<int64_t>> t(n + 1, std::vector<int64_t>(k + 1));
  t[0][0] = 1;
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= k; j++) {
      t[i][j] = (i - 1) * t[i - 1][j] % m;
      t[i][j] = (t[i][j] + t[i - 1][j - 1]) % m;
    }
  }
  return t[n][k] % m;
}

int64_t stirling2(int n, int k, int64_t m = MOD) {
  std::vector<std::vector<int64_t>> t(n + 1, std::vector<int64_t>(k + 1));
  t[0][0] = 1;
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= k; j++) {
      t[i][j] = j * t[i - 1][j] % m;
      t[i][j] = (t[i][j] + t[i - 1][j - 1]) % m;
    }
  }
  return t[n][k] % m;
}

int64_t eulerian1(int n, int k, int64_t m = MOD) {
  if (k > n - 1 - k) {
    k = n - 1 - k;
  }
  std::vector<std::vector<int64_t>> t(n + 1, std::vector<int64_t>(k + 1, 1));
  for (int j = 1; j <= k; j++) {
    t[0][j] = 0;
  }
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= k; j++) {
      t[i][j] = (i - j) * t[i - 1][j - 1] % m;
      t[i][j] = (t[i][j] + ((j + 1) * t[i - 1][j] % m)) % m;
    }
  }
  return t[n][k] % m;
}

int64_t eulerian2(int n, int k, int64_t m = MOD) {
  std::vector<std::vector<int64_t>> t(n + 1, std::vector<int64_t>(k + 1, 1));
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= k; j++) {
      if (i == j) {
        t[i][j] = 0;
      } else {
        t[i][j] = (j + 1) * t[i - 1][j] % m;
        t[i][j] = ((2 * i - 1 - j) * t[i - 1][j - 1] % m + t[i][j]) % m;
      }
    }
  }
  return t[n][k] % m;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  auto t = binomial_table(10);
  for (int i = 0; i < static_cast<int>(t.size()); i++) {
    for (int j = 0; j < static_cast<int>(t[i].size()); j++) {
      assert(t[i][j] == choose(i, j));
    }
  }
  assert(factorial(10) == 3628800);
  assert(factorial_without_p(123456) == 639390503);
  assert(factorial_without_p(7, 5) == 3);  // 7! / 5 = 1008 = 3 (mod 5).
  assert(legendre(10, 2) == 8);            // 10! = 2^8 * 3^4 * 5^2 * 7.
  assert(legendre(100, 5) == 24);          // 100! ends in 24 zeros.
  assert(permute(10, 4) == 5040);
  assert(choose(20, 7) == 77520);
  assert(choose_lucas(20, 7) == 77520);
  auto mod7 = binomial_table(30, 7);
  for (int i = 0; i <= 30; i++) {
    for (int j = 0; j <= i; j++) {
      assert(choose_lucas(i, j, 7) == mod7[i][j]);
    }
  }
  // By Lucas' theorem mod 2, a binomial coefficient is odd exactly when k is a submask of n.
  assert(choose_lucas((1LL << 40) - 1, 12345, 2) == 1);
  assert(choose_lucas(1LL << 40, 1, 2) == 0);
  assert(multichoose(20, 7) == 657800);
  assert(catalan(10) == 16796);
  assert(partitions(4) == 5);
  assert(partitions(0, 0) == 1);
  assert(partitions(100, 5) == 38225);
  assert(stirling1(4, 2) == 11);
  assert(stirling2(4, 3) == 6);
  assert(eulerian1(9, 5) == 88234);
  assert(eulerian2(8, 3) == 195800);
  return 0;
}
