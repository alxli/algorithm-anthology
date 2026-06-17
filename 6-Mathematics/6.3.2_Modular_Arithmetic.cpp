/*

Wraps arithmetic modulo a compile-time constant in a small value type. This is a common contest
helper for dynamic programming, combinatorics, polynomial operations, and any calculation where all
answers are taken modulo some number $p$ such as $10^9 + 7$.

The implementation is intentionally close to common contest "Mint" templates: normalization happens
at construction, arithmetic operators are overloaded, mixed integer operations are supported through
implicit construction, and combinations can be computed from lazy factorial tables. The operators
use the hidden friend idiom: each `friend` defined inside the class is a non-template function, so
mixed expressions like `2 + Mint(3)` convert the integer operand through the implicit constructor.
(Free function template operators cannot do this - template argument deduction ignores implicit
conversions - which is why some contest templates carry per-operator `Modular op U` and
`U op Modular` overloads instead.)

The modulus is supplied directly as the `auto` template argument, so `Modular<1000000007>` and
`Modular<998244353>` are all that is needed. Its literal type determines the storage type, so a
64-bit modulus (e.g. `Modular<(1LL << 61) - 1>`) is stored in 64 bits without any extra annotation.
The modulus must be a compile-time constant; a runtime-chosen modulus is not supported by this
design.

Two integer types are at play: the type of the modulus stores the representative, while
construction, multiplication, and inverses widen through an intermediate type chosen automatically
from the storage width - `int64_t` for 32-bit storage, or `__int128` (a GCC/Clang extension, in
line with this book's use of `__builtin` functions) for 64-bit storage; on compilers without
`__int128`, only 32-bit moduli are supported. The requirements are that $2p$ fits the storage type
(for addition) and $(p - 1)^2$ fits the intermediate type (for the widening multiply), so any
modulus up to half the storage type's range works out of the box; 64-bit moduli simply pay the cost
of slower 128-bit multiplies. (Some contest templates instead estimate the multiply's quotient in
`long double` to avoid `__int128`; beware that this requires x86's 80-bit `long double` and silently
breaks for large moduli on ARM, where `long double` is only 64-bit.)

Division uses the extended Euclidean algorithm, so the divisor only needs to be coprime to the
modulus. For the factorial-table combination helper, the usual contest assumption is that $p$ is
prime and the requested factorials are invertible modulo $p$.

- `Modular<MOD>(x)` constructs the residue class of integer `x` modulo `MOD`.
- `value()` and `operator()()` return the stored representative in $[0, `MOD`)$.
- `pow(n)` returns this value raised to nonnegative integer exponent `n`.
- `inv()` returns the multiplicative inverse, asserting it exists.
- Operators `+`, `-`, `*`, `/`, comparison, increment, decrement, and stream I/O are overloaded.
- `ModCombinatorics<Mint>::factorial(n)` returns $n!$ using a lazy factorial table.
- `ModCombinatorics<Mint>::choose(n, k)` returns $\binom n k$ using lazy factorial and
  inverse-factorial tables.
- `ModCombinatorics<Mint>::permute(n, k)` returns the number of ordered selections of `k` distinct
  elements from `n`.
- `ModCombinatorics<Mint>::multichoose(n, k)` returns the number of size-`k` multisets drawn from
  `n` types.

Time Complexity:
- O(1) per addition, subtraction, multiplication, comparison, and stream output.
- O(log n) per call to `pow(n)`.
- O(log `MOD`) per call to `inv()` and division.
- O(n) total table growth to answer factorials and combinations up to size `n`.

Space Complexity:
- O(1) auxiliary for Modular arithmetic.
- O(n) for factorial and inverse-factorial tables grown through `choose(n, k)`.

*/

#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

template<class T>
T inverse(T a, T m) {
  T original_m = m;
  T u = 0, v = 1;
  while (a != 0) {
    T q = m / a;
    m -= q * a;
    std::swap(a, m);
    u -= q * v;
    std::swap(u, v);
  }
  assert(m == 1);
  u %= original_m;
  return u < 0 ? u + original_m : u;
}

template<auto MOD>
class Modular {
  using value_t = decltype(MOD);

  // Widen through int64_t for 32-bit storage, or __int128 for 64-bit storage where it exists
  // (__extension__ silences -pedantic). Without __int128, only 32-bit moduli are supported.
#if defined(__SIZEOF_INT128__)
  __extension__ typedef std::conditional_t<sizeof(value_t) <= 4, int64_t, __int128> wide_t;
#else
  using wide_t = int64_t;
  static_assert(sizeof(value_t) <= 4, "64-bit moduli require __int128, which is unavailable");
#endif

  value_t v;

  static value_t normalize(wide_t x) {
    if (-static_cast<wide_t>(mod()) <= x && x < static_cast<wide_t>(mod())) {
      value_t y = static_cast<value_t>(x);
      return y < 0 ? y + mod() : y;
    }
    x %= mod();
    if (x < 0) {
      x += mod();
    }
    return static_cast<value_t>(x);
  }

 public:
  Modular(wide_t x = 0) { v = normalize(x); }

  static value_t mod() { return MOD; }
  value_t value() const { return v; }
  value_t operator()() const { return v; }

  Modular pow(int64_t n) const {
    assert(n >= 0);
    Modular base = *this, res = 1;
    while (n > 0) {
      if (n & 1) {
        res *= base;
      }
      base *= base;
      n >>= 1;
    }
    return res;
  }

  Modular inv() const {
    assert(v != 0);
    return Modular(inverse(static_cast<wide_t>(v), static_cast<wide_t>(mod())));
  }

  Modular &operator+=(const Modular &other) {
    v += other.v;
    if (v >= mod()) {
      v -= mod();
    }
    return *this;
  }

  Modular &operator-=(const Modular &other) {
    v -= other.v;
    if (v < 0) {
      v += mod();
    }
    return *this;
  }

  Modular &operator*=(const Modular &other) {
    v = normalize(static_cast<wide_t>(v) * other.v);
    return *this;
  }

  // clang-format off
  Modular operator-() const { return Modular(-v); }
  Modular &operator++() { return *this += 1; }
  Modular &operator--() { return *this -= 1; }
  Modular &operator/=(const Modular &other) { return *this *= other.inv(); }
  Modular operator++(int) { Modular res = *this; ++*this; return res; }
  Modular operator--(int) { Modular res = *this; --*this; return res; }
  friend Modular operator+(Modular a, const Modular &b) { return a += b; }
  friend Modular operator-(Modular a, const Modular &b) { return a -= b; }
  friend Modular operator*(Modular a, const Modular &b) { return a *= b; }
  friend Modular operator/(Modular a, const Modular &b) { return a /= b; }
  friend bool operator==(const Modular &a, const Modular &b) { return a.v == b.v; }
  friend bool operator!=(const Modular &a, const Modular &b) { return !(a == b); }
  friend bool operator<(const Modular &a, const Modular &b) { return a.v < b.v; }
  friend std::ostream &operator<<(std::ostream &os, const Modular &x) { return os << x.v; }
  // clang-format on

  friend std::istream &operator>>(std::istream &is, Modular &x) {
    int64_t value;
    is >> value;
    x = Modular(value);
    return is;
  }
};

template<class Mint>
class ModCombinatorics {
  std::vector<Mint> fact, inv_fact;

  void ensure(int n) {
    auto old = static_cast<int>(fact.size());
    if (old > n) {
      return;
    }
    fact.resize(n + 1);
    inv_fact.resize(n + 1);
    for (int i = old; i <= n; i++) {
      fact[i] = fact[i - 1] * i;
    }
    inv_fact[n] = fact[n].inv();
    for (int i = n; i > old; i--) {
      inv_fact[i - 1] = inv_fact[i] * i;
    }
  }

 public:
  ModCombinatorics() : fact(1, Mint(1)), inv_fact(1, Mint(1)) {}

  Mint factorial(int n) {
    ensure(n);
    return fact[n];
  }

  Mint choose(int n, int k) {
    if (k < 0 || k > n) {
      return 0;
    }
    ensure(n);
    return fact[n] * inv_fact[k] * inv_fact[n - k];
  }

  Mint permute(int n, int k) {
    if (k < 0 || k > n) {
      return 0;
    }
    ensure(n);
    return fact[n] * inv_fact[n - k];
  }

  Mint multichoose(int n, int k) { return choose(n + k - 1, k); }
};

using Mint = Modular<1000000007>;

/*** Example Usage ***/

#include <cassert>
#include <sstream>
using namespace std;

int main() {
  Mint a = 1000000008LL;
  Mint b = -2;
  assert(a.value() == 1);
  assert(b() == Mint::mod() - 2);
  assert(a + b == -1);
  assert(2 + Mint(3) == 5);
  assert(Mint(2) * 3 == 6);
  assert(Mint(2).pow(10) == 1024);
  assert(Mint(5) / 5 == 1);

  Mint x = 0;
  x++;
  ++x;
  assert(x == 2);
  stringstream ss("1000000008");
  ss >> x;
  assert(x == 1);

  ModCombinatorics<Mint> comb;
  assert(comb.factorial(5) == 120);
  assert(comb.choose(5, 2) == 10);
  assert(comb.permute(5, 2) == 20);
  assert(comb.multichoose(3, 2) == 6);

  // Each distinct modulus is just another instantiation.
  using Mint2 = Modular<998244353>;
  assert(Mint2(2).pow(10) == 1024);
  assert(Mint2(-1) == Mint2::mod() - 1);

  // A 64-bit modulus (deduced as long long) automatically widens through __int128.
  using Mint3 = Modular<(1LL << 61) - 1>;
  assert(Mint3(2).pow(62) == 2);  // 2^62 mod (2^61 - 1)
  assert(Mint3(3).inv() * 3 == 1);
  return 0;
}
