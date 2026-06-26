/*

Small contest-template helpers that are useful across many algorithms. These snippets avoid
algorithm-specific policy and are meant to be pasted near the top of a solution file.

- `Rep(i,N)`, `For(i,L,H)`, `Rev(i,N)`, `Dwn(i,H,L)`, and `Each(x,C)` are compact loop macros.
- `sz(c)` returns the signed size of a container.
- `ckmin(a, b)` assigns `a = b` and returns true if `b` < `a` before the assignment.
- `ckmax(a, b)` assigns `a = b` and returns true if `a` < `b` before the assignment.
- `between(x, a, b)` returns whether `a` $\leq$ `x` $\leq$ `b`.
- `clmp(x, a, b)` returns `x` clamped into the interval $[`a`, `b`]$. This is not needed on C++17
  and later, where std::clamp() is available.
- `floor_div(a, b)` and `ceil_div(a, b)` divide signed integers with mathematical rounding toward
  negative or positive infinity. Requires `b != 0`.
- `sort_unique(v)` sorts a vector and removes duplicates.
- `erase_one(c, x)` erases one existing value from an associative container, asserting that it is
  present.
- `min_priority_queue<T>` is a min-heap alias.
- `RNG()` constructs a 64-bit Mersenne Twister seeded from the steady clock.
- `RNG(seed)` constructs a reproducible 64-bit Mersenne Twister.
- `rng.uniform_int(lo, hi)` returns a random integer in the inclusive range $[`lo`, `hi`]$.
- `rng.uniform_real(lo, hi)` returns a random real number in the half-open range $[`lo`, `hi`)$.
- `rng.shuffle(lo, hi)` randomly shuffles the range $[`lo`, `hi`)$.
- `y_combinator(f)` wraps a recursive lambda so that the lambda can call itself as its first
  argument.

*/

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <queue>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

#define Rep(i, N) for (int i = 0, _##i = (N); i < _##i; ++i)        //   0 to N-1
#define For(i, L, H) for (int i = (L), _##i = (H); i <= _##i; ++i)  //   L to H
#define Rev(i, N) for (int i = (N); --i >= 0;)                      // N-1 to 0
#define Dwn(i, H, L) for (int i = (H), _##i = (L); i >= _##i; --i)  //   H to L
#define Each(x, C) for (auto &x : (C))

// clang-format off
template<typename C> int sz(const C &c) { return static_cast<int>(c.size()); }
template<typename T, typename U>
bool ckmin(T &a, const U &b) { return b < a ? a = b, true : false; }
template<typename T, typename U>
bool ckmax(T &a, const U &b) { return a < b ? a = b, true : false; }
template<typename T>
bool between(const T &x, const T &a, const T &b) { return !(x < a) && !(b < x); }
template<typename T>
T clmp(const T &x, const T &a, const T &b) { return std::min(std::max(x, a), b); }
// clang-format on

template<typename T>
T floor_div(T a, T b) {
  assert(b != 0);
  T q = a / b, r = a % b;
  return q - (r != 0 && ((r < 0) != (b < 0)));
}

template<typename T>
T ceil_div(T a, T b) {
  assert(b != 0);
  T q = a / b, r = a % b;
  return q + (r != 0 && ((r < 0) == (b < 0)));
}

template<typename T>
void sort_unique(std::vector<T> &v) {
  std::sort(v.begin(), v.end());
  v.erase(std::unique(v.begin(), v.end()), v.end());
}

template<typename C, typename T>
void erase_one(C &c, const T &x) {
  auto it = c.find(x);
  assert(it != c.end());
  c.erase(it);
}

template<typename T>
using min_priority_queue = std::priority_queue<T, std::vector<T>, std::greater<T>>;

struct RNG {
  std::mt19937_64 gen;

  RNG() : RNG(std::chrono::steady_clock::now().time_since_epoch().count()) {}
  explicit RNG(uint64_t seed) : gen(seed) {}

  template<typename T>
  T uniform_int(T lo, T hi) {
    static_assert(std::is_integral<T>::value, "uniform_int() requires an integral type");
    return std::uniform_int_distribution<T>(lo, hi)(gen);
  }

  double uniform_real(double lo = 0.0, double hi = 1.0) {
    return std::uniform_real_distribution<double>(lo, hi)(gen);
  }

  template<typename It>
  void shuffle(It lo, It hi) {
    std::shuffle(lo, hi, gen);
  }
};

template<typename Fun>
class y_combinator_result {
  Fun fun;

 public:
  template<typename T>
  explicit y_combinator_result(T &&fun_) : fun(std::forward<T>(fun_)) {}
  template<class... Args>
  decltype(auto) operator()(Args &&...args) {
    return fun(std::ref(*this), std::forward<Args>(args)...);
  }
};

template<typename Fun>
decltype(auto) y_combinator(Fun &&fun) {
  return y_combinator_result<std::decay_t<Fun>>(std::forward<Fun>(fun));
}

/*** Example Usage ***/

#include <set>
using namespace std;

int main() {
  int tot = 0;
  Rep(i, 4) tot += i;     // 0 + 1 + 2 + 3 = 6
  For(i, 2, 5) tot += i;  // 6 + (2 + 3 + 4 + 5) = 20
  Rev(i, 3) tot += i;     // 20 + (2 + 1 + 0) = 23
  Dwn(i, 8, 6) tot += i;  // 23 + (8 + 7 + 6) = 44
  assert(tot == 44);

  int x = 10;
  assert(ckmin(x, 7) && x == 7);
  assert(!ckmin(x, 9) && x == 7);
  assert(ckmax(x, 11) && x == 11);

  vector<int> v = {1, 2, 3};
  assert(sz(v) == 3);
  assert(between(2, 1, 3));
  assert(between(1, 1, 3) && between(3, 1, 3));
  assert(!between(0, 1, 3));
  assert(clmp(-5, 1, 3) == 1);
  assert(clmp(10, 1, 3) == 3);
  assert(floor_div(7, -3) == -3);
  assert(floor_div(-7, 3) == -3);
  assert(ceil_div(7, -3) == -2);
  assert(ceil_div(-7, 3) == -2);
  v = {3, 1, 3, 2, 1};
  sort_unique(v);
  assert((v == vector<int>{1, 2, 3}));
  multiset<int> ms{1, 2, 2, 3};
  erase_one(ms, 2);
  assert(ms.count(2) == 1);
  min_priority_queue<int> pq;
  pq.push(3);
  pq.push(1);
  assert(pq.top() == 1);

  RNG rng(123);
  int r = rng.uniform_int(1, 6);
  assert(1 <= r && r <= 6);
  double d = rng.uniform_real();
  assert(0.0 <= d && d < 1.0);
  rng.shuffle(v.begin(), v.end());
  assert(sz(v) == 3);

  // Recursive lambda with y_combinator.
  auto fib =
      y_combinator([](auto fib, int n) -> int { return n < 2 ? n : fib(n - 1) + fib(n - 2); });
  assert(fib(10) == 55);

  // Recursive lambda without y_combinator: pass the lambda to itself as the first argument.
  vector<vector<int>> adj{{1, 2}, {0}, {0}};
  int seen = 0;
  auto dfs = [&](auto &&dfs, int u, int p) -> void {
    seen++;
    Each(v, adj[u]) if (v != p) dfs(dfs, v, u);
  };
  dfs(dfs, 0, -1);
  assert(seen == 3);

  // Recursive lambdas are automatically supported in C++23.
#if __cplusplus >= 202302L
  auto fact = [&](this auto fact, int n) -> int { return n ? n * fact(n - 1) : 1; };
  assert(fact(5) == 120);
#endif
  return 0;
}
