/*

Small contest-template helpers that are useful across many algorithms. These snippets avoid
algorithm-specific policy and are meant to be pasted near the top of a solution file.

- `y_combinator(f)` wraps a recursive lambda so that the lambda can call itself as its first
  argument.
- `ckmin(a, b)` assigns `a = b` and returns true if `b < a` before the assignment.
- `ckmax(a, b)` assigns `a = b` and returns true if `a < b` before the assignment.
- `ssize(c)` returns the signed size of a container.
- `between(x, lo, hi)` returns whether `lo` $\leq$ `x` $\leq$ `hi`.
- `clamped(x, lo, hi)` returns `x` clamped into the interval `[lo, hi]`.
- `RNG()` constructs a 64-bit Mersenne Twister seeded from the steady clock.
- `RNG(seed)` constructs a reproducible 64-bit Mersenne Twister.
- `rng.uniform_int(lo, hi)` returns a random integer in the inclusive range `[lo, hi]`.
- `rng.uniform_real(lo, hi)` returns a random real number in the half-open range `[lo, hi)`.
- `rng.shuffle(lo, hi)` randomly shuffles the iterator range `[lo, hi)`.

*/

#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <random>
#include <type_traits>
#include <vector>

template<class Fun>
class y_combinator_result {
  Fun fun;

 public:
  template<class T>
  explicit y_combinator_result(T &&fun_) : fun(std::forward<T>(fun_)) {}

  template<class... Args>
  decltype(auto) operator()(Args &&...args) {
    return fun(std::ref(*this), std::forward<Args>(args)...);
  }
};

template<class Fun>
decltype(auto) y_combinator(Fun &&fun) {
  return y_combinator_result<std::decay_t<Fun>>(std::forward<Fun>(fun));
}

template<class T, class U>
bool ckmin(T &a, const U &b) {
  if (b < a) {
    a = b;
    return true;
  }
  return false;
}

template<class T, class U>
bool ckmax(T &a, const U &b) {
  if (a < b) {
    a = b;
    return true;
  }
  return false;
}

template<class C>
int ssize(const C &c) {
  return static_cast<int>(c.size());
}

template<class T>
bool between(const T &x, const T &lo, const T &hi) {
  return !(x < lo) && !(hi < x);
}

template<class T>
T clamped(const T &x, const T &lo, const T &hi) {
  return std::min(std::max(x, lo), hi);
}

struct RNG {
  std::mt19937_64 gen;

  RNG() : RNG(std::chrono::steady_clock::now().time_since_epoch().count()) {}
  explicit RNG(uint64_t seed) : gen(seed) {}

  template<class T>
  T uniform_int(T lo, T hi) {
    static_assert(std::is_integral<T>::value, "uniform_int() requires an integral type");
    return std::uniform_int_distribution<T>(lo, hi)(gen);
  }

  double uniform_real(double lo = 0.0, double hi = 1.0) {
    return std::uniform_real_distribution<double>(lo, hi)(gen);
  }

  template<class It>
  void shuffle(It lo, It hi) {
    std::shuffle(lo, hi, gen);
  }
};

/*** Example Usage ***/

int main() {
  auto fib =
      y_combinator([](auto self, int n) -> int { return n < 2 ? n : self(n - 1) + self(n - 2); });
  assert(fib(10) == 55);

  int x = 10;
  assert(ckmin(x, 7) && x == 7);
  assert(!ckmin(x, 9) && x == 7);
  assert(ckmax(x, 11) && x == 11);

  std::vector<int> v = {1, 2, 3};
  assert(ssize(v) == 3);
  assert(between(2, 1, 3));
  assert(clamped(10, 1, 3) == 3);

  RNG rng(123);
  int r = rng.uniform_int(1, 6);
  assert(1 <= r && r <= 6);
  rng.shuffle(v.begin(), v.end());
  assert(ssize(v) == 3);
  return 0;
}
