/*

Provides a fixed-modulus value type and lazy factorial-based combinatorics tables. These are common
contest helpers for dynamic programming, counting, polynomial operations, and any calculation whose
answers are taken modulo some number such as $10^9 + 7$. Runtime-chosen moduli are not supported.

The `Modular<MOD>` value type wraps arithmetic modulo the positive compile-time constant `MOD`. Its
signed `auto` argument determines the storage type: `Modular<1000000007>` uses 32-bit storage, while
`Modular<(1LL << 61) - 1>` uses 64-bit storage. The implementation follows the conventional `Mint`
wrapper: construction normalizes values, while hidden friend operators support mixed expressions
such as `2 + Mint(3)` through implicit conversion.

- `Modular<MOD>(x = 0)` constructs the residue class of integer `x` modulo `MOD`.
- `value()` and `operator()()` return the stored representative in $[0, `MOD`)$.
- Explicit casts to `int`, `long long`, `double`, and `long double` convert that stored
  representative to the corresponding primitive.
- `pow(n)` returns this value raised to nonnegative integer exponent `n`.
- `inv()` returns the multiplicative inverse, asserting the value is coprime to `MOD`.
- Operators `+`, `-`, `*`, `/`, comparison, increment, decrement, and stream I/O are overloaded.
  Division likewise requires the divisor to be coprime to `MOD`.

Overflow warning: Construction, multiplication, and inverses widen through `int64_t` for 32-bit
storage or `__int128` for 64-bit storage. Thus $2p$ must fit the storage type and $(p - 1)^2$ must
fit the intermediate type. On compilers without `__int128`, only 32-bit moduli are supported.
Estimating a 64-bit product with `long double` is not used because it silently loses the needed
precision on platforms where `long double` is only 64 bits.

Time Complexity:
- O(1) per addition, subtraction, multiplication, comparison, and stream output.
- O(log n) per call to `pow()`.
- O(log m) per call to `inv()` and division, where $m = `MOD`$.

Space Complexity:
- O(1) auxiliary for Modular arithmetic.

*/

#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template<typename T>
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
  using T = decltype(MOD);

  static_assert(std::is_integral_v<T> && std::is_signed_v<T>);
  static_assert(MOD > 0 && MOD <= std::numeric_limits<T>::max() / 2);

  // Widen through int64_t for 32-bit storage, or __int128 for 64-bit storage where it exists
  // (__extension__ silences -pedantic). Without __int128, only 32-bit moduli are supported.
#if defined(__SIZEOF_INT128__)
  __extension__ typedef std::conditional_t<sizeof(T) <= 4, int64_t, __int128> wide_t;
#else
  using wide_t = int64_t;
  static_assert(sizeof(T) <= 4, "64-bit moduli require __int128, which is unavailable");
#endif

  T v;

  static T normalize(wide_t x) {
    if (-static_cast<wide_t>(mod()) <= x && x < static_cast<wide_t>(mod())) {
      T y = static_cast<T>(x);
      return y < 0 ? y + mod() : y;
    }
    x %= mod();
    if (x < 0) {
      x += mod();
    }
    return static_cast<T>(x);
  }

 public:
  Modular(wide_t x = 0) : v(normalize(x)) {}

  static T mod() { return MOD; }
  T value() const { return v; }
  T operator()() const { return v; }
  explicit operator int() const { return static_cast<int>(v); }
  explicit operator long long() const { return static_cast<long long>(v); }
  explicit operator double() const { return static_cast<double>(v); }
  explicit operator long double() const { return static_cast<long double>(v); }

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

/*

`ModCombinatorics<Mint>` maintains lazy factorial and inverse-factorial tables for a modular type
`Mint`. It assumes the modulus is prime and all requested factorials are invertible.

- `factorial(n)` returns $n!$ using a lazy factorial table.
- `choose(n, k)` returns $\binom n k$ using lazy factorial and inverse-factorial tables.
- `permute(n, k)` returns the number of ordered selections of `k` distinct elements from `n`.
- `multichoose(n, k)` returns the number of size-`k` multisets drawn from `n` types.

Time Complexity:
- O(n) total table growth to answer calls with arguments up to $n$, then O(1) per call.

Space Complexity:
- O(n) for the factorial and inverse-factorial tables.

*/

template<typename Mint>
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
  ModCombinatorics() : fact(1, Mint{1}), inv_fact(1, Mint{1}) {}

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

  Mint multichoose(int n, int k) { return n == 0 ? Mint(k == 0) : choose(n + k - 1, k); }
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
  assert(static_cast<int>(a) == 1);
  assert(static_cast<long long>(a) == 1LL);
  assert(static_cast<double>(a) == 1.0);
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
  assert(comb.multichoose(0, 0) == 1);

  // Each distinct modulus is just another instantiation.
  using Mint2 = Modular<998244353>;
  assert(Mint2(2).pow(10) == 1024);
  assert(Mint2(-1) == Mint2::mod() - 1);

#if defined(__SIZEOF_INT128__)
  // A 64-bit modulus (deduced as long long) automatically widens through __int128.
  using Mint3 = Modular<(1LL << 61) - 1>;
  assert(Mint3(2).pow(62) == 2);  // 2^62 mod (2^61 - 1)
  assert(Mint3(3).inv() * 3 == 1);
#endif
  return 0;
}
