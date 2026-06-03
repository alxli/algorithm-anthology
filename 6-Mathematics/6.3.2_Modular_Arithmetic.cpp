/*

Wraps arithmetic modulo a compile-time constant in a small value type. This is a common contest
helper for dynamic programming, combinatorics, polynomial operations, and any calculation where all
answers are taken modulo a number such as `1000000007`.

The implementation is intentionally close to common contest "Mint" templates: normalization happens
at construction, arithmetic operators are overloaded, mixed integer operations are supported through
implicit construction, and combinations can be computed from lazy factorial tables.

Division uses the extended Euclidean algorithm, so the divisor only needs to be coprime to the
modulus. For the factorial-table combination helper, the usual contest assumption is that `MOD` is
prime and the requested factorials are invertible modulo `MOD`.

- `modular<T>(x)` constructs the residue class of integer `x` modulo `T::value`.
- `value()` and `operator()()` return the stored representative in `[0, MOD)`.
- `pow(e)` returns this value raised to nonnegative exponent `e`.
- `inv()` returns the multiplicative inverse, asserting it exists.
- Operators `+`, `-`, `*`, `/`, comparison, increment, decrement, and stream I/O are overloaded.
- `mod_combinatorics<Mint>::choose(n, k)` returns $\binom n k$ using lazy factorial and
  inverse-factorial tables.

Time Complexity:
- O(1) per addition, subtraction, multiplication, comparison, and stream output.
- O(log e) per call to `pow(e)`.
- O(log MOD) per call to `inv()` and division.
- O(n) total table growth to answer combinations up to size `n`.

Space Complexity:
- O(1) auxiliary for modular arithmetic.
- O(n) for factorial and inverse-factorial tables grown through `choose(n, k)`.

*/

#include <cassert>
#include <iostream>
#include <vector>

template<class T>
T inverse(T a, T m) {
  T original_m = m;
  T u = 0, v = 1;
  while (a != 0) {
    T q = m / a;
    m -= q * a;
    T tmp = a;
    a = m;
    m = tmp;
    u -= q * v;
    tmp = v;
    v = u;
    u = tmp;
  }
  assert(m == 1);
  u %= original_m;
  return u < 0 ? u + original_m : u;
}

template<class T>
class modular {
 public:
  typedef typename T::value_type value_type;

 private:
  value_type v;

 public:
  modular(long long x = 0) { v = normalize(x); }

  static value_type mod() { return T::value; }

  static value_type normalize(long long x) {
    if (-(long long)mod() <= x && x < (long long)mod()) {
      value_type y = (value_type)x;
      return y < 0 ? y + mod() : y;
    }
    x %= mod();
    if (x < 0) {
      x += mod();
    }
    return (value_type)x;
  }

  value_type value() const { return v; }
  value_type operator()() const { return v; }

  modular pow(long long e) const {
    assert(e >= 0);
    modular base = *this, res = 1;
    while (e > 0) {
      if (e & 1) {
        res *= base;
      }
      base *= base;
      e >>= 1;
    }
    return res;
  }

  modular inv() const {
    assert(v != 0);
    return modular(inverse((long long)v, (long long)mod()));
  }

  modular &operator+=(const modular &other) {
    v += other.v;
    if (v >= mod()) {
      v -= mod();
    }
    return *this;
  }

  modular &operator-=(const modular &other) {
    v -= other.v;
    if (v < 0) {
      v += mod();
    }
    return *this;
  }

  modular &operator*=(const modular &other) {
    v = normalize((long long)v * other.v);
    return *this;
  }

  modular operator-() const { return modular(-v); }
  modular &operator++() { return *this += 1; }
  modular &operator--() { return *this -= 1; }
  modular &operator/=(const modular &other) { return *this *= other.inv(); }

  modular operator++(int) {
    modular res = *this;
    ++*this;
    return res;
  }

  modular operator--(int) {
    modular res = *this;
    --*this;
    return res;
  }

  friend modular operator+(modular a, const modular &b) { return a += b; }

  friend modular operator-(modular a, const modular &b) { return a -= b; }

  friend modular operator*(modular a, const modular &b) { return a *= b; }

  friend modular operator/(modular a, const modular &b) { return a /= b; }

  friend bool operator==(const modular &a, const modular &b) { return a.v == b.v; }

  friend bool operator!=(const modular &a, const modular &b) { return !(a == b); }

  friend bool operator<(const modular &a, const modular &b) { return a.v < b.v; }

  friend std::ostream &operator<<(std::ostream &os, const modular &x) { return os << x.v; }

  friend std::istream &operator>>(std::istream &is, modular &x) {
    long long value;
    is >> value;
    x = modular(value);
    return is;
  }
};

template<class Mint>
class mod_combinatorics {
  std::vector<Mint> fact, inv_fact;

  void ensure(int n) {
    int old = fact.size();
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
  mod_combinatorics() : fact(1, Mint(1)), inv_fact(1, Mint(1)) {}

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

struct mod_1000000007 {
  typedef int value_type;
  static const int value = 1000000007;
};

typedef modular<mod_1000000007> Mint;

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

  mod_combinatorics<Mint> comb;
  assert(comb.factorial(5) == 120);
  assert(comb.choose(5, 2) == 10);
  assert(comb.permute(5, 2) == 20);
  assert(comb.multichoose(3, 2) == 6);
  return 0;
}
