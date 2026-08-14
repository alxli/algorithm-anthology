/*

Small contest convenience helpers that are useful across many algorithms. These snippets avoid
algorithm-specific policy and are meant to be pasted near the top of a solution file.

A personal template may also collect shortened versions of utilities elsewhere in the anthology,
such as fast I/O and debugging, custom hashing, coordinate compression, bit operations, modular
arithmetic, binary search, and GNU policy-based data structures. They remain in their dedicated
sections here to avoid duplication and let each template be assembled to taste.

- `Rep(i,N)`, `For(i,L,H)`, `Rev(i,N)`, `Dwn(i,H,L)`, and `Each(x,C)` are compact loop macros.
- `All(c)` and `Rall(c)` expand to the forward or reverse iterator pair of a container or array.
- `sz(c)` returns the signed size of a container.
- `ckmin(a, b)` assigns `a = b` and returns true if `b` < `a` before the assignment.
- `ckmax(a, b)` assigns `a = b` and returns true if `a` < `b` before the assignment.
- `floor_div(a, b)` and `ceil_div(a, b)` divide signed integers with mathematical rounding toward
  negative or positive infinity. Requires nonzero `b` and the resulting quotient to be representable
  by the result type; in particular, the minimum value cannot be divided by $-1$.
- `lb(a, x, comp = std::less<>)` and `ub(a, x, comp = std::less<>)` return lower and upper bound
  indices in random-access sequence `a`, which must be sorted according to `comp`.
- `sort_unique(v)` sorts a vector and removes duplicates.
- `indices(n)` returns the vector `{0, 1, ..., n - 1}`.
- `argsort(a, comp = std::less<>)` returns the indices of `a` ordered by their values according to
  `comp`; the order of indices whose values compare equal is unspecified.
- `erase_one(c, x)` erases one existing value from an associative container, asserting that it is
  present.
- `min_heap<T>` is a min-heap alias.
- `rng` is a global 64-bit Mersenne Twister seeded from the current `std::chrono::steady_clock`
  reading; call `rng.seed(seed)` for reproducible results.
- `rand_int(lo, hi)` returns a random integer in range $[`lo`, `hi`]$.
- `rand_real(lo = 0.0, hi = 1.0)` returns a random real number in the half-open range
  $[`lo`, `hi`)$.
- `y_combinator(f)` wraps a recursive lambda so that the lambda can call itself as its first
  argument.

*/

#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <iterator>
#include <numeric>
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
#define All(C) std::begin(C), std::end(C)
#define Rall(C) std::rbegin(C), std::rend(C)

// clang-format off
template<typename C> int sz(const C &c) { return static_cast<int>(c.size()); }
template<typename T, typename U> bool ckmin(T &a, const U &b) { return b < a && (a = b, true); }
template<typename T, typename U> bool ckmax(T &a, const U &b) { return a < b && (a = b, true); }
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

template<typename Seq, typename T, typename Compare = std::less<>>
int lb(const Seq &a, const T &x, Compare comp = Compare{}) {
  return static_cast<int>(std::lower_bound(a.begin(), a.end(), x, comp) - a.begin());
}

template<typename Seq, typename T, typename Compare = std::less<>>
int ub(const Seq &a, const T &x, Compare comp = Compare{}) {
  return static_cast<int>(std::upper_bound(a.begin(), a.end(), x, comp) - a.begin());
}

template<typename T>
void sort_unique(std::vector<T> &v) {
  std::sort(v.begin(), v.end());
  v.erase(std::unique(v.begin(), v.end()), v.end());
}

std::vector<int> indices(int n) {
  std::vector<int> v(n);
  std::iota(v.begin(), v.end(), 0);
  return v;
}

template<typename Seq, typename Compare = std::less<>>
std::vector<int> argsort(const Seq &a, Compare comp = Compare{}) {
  std::vector<int> p = indices(static_cast<int>(a.size()));
  std::sort(p.begin(), p.end(), [&](int i, int j) { return comp(a[i], a[j]); });
  return p;
}

template<typename C, typename T>
void erase_one(C &c, const T &x) {
  auto it = c.find(x);
  assert(it != c.end());
  c.erase(it);
}

template<typename T>
using min_heap = std::priority_queue<T, std::vector<T>, std::greater<T>>;

std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());

template<typename Int>
Int rand_int(Int lo, Int hi) {
  static_assert(std::is_integral<Int>::value, "rand_int() requires an integral type");
  assert(lo <= hi);
  return std::uniform_int_distribution<Int>(lo, hi)(rng);
}

double rand_real(double lo = 0.0, double hi = 1.0) {
  assert(lo < hi);
  return std::uniform_real_distribution<double>(lo, hi)(rng);
}

template<typename Fn>
class y_combinator_result {
  Fn fn;

 public:
  template<typename T>
  explicit y_combinator_result(T &&f) : fn(std::forward<T>(f)) {}

  template<typename... Args>
  decltype(auto) operator()(Args &&...args) {
    return fn(std::ref(*this), std::forward<Args>(args)...);
  }
};

template<typename Fn>
decltype(auto) y_combinator(Fn &&f) {
  return y_combinator_result<std::decay_t<Fn>>(std::forward<Fn>(f));
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

  vector<int> v{1, 2, 3};
  assert(sz(v) == 3);
  sort(Rall(v));
  assert((v == vector<int>{3, 2, 1}));
  assert(lb(v, 2, greater<>{}) == 1 && ub(v, 2, greater<>{}) == 2);
  sort(All(v));
  assert(lb(v, 2) == 1 && ub(v, 2) == 2);
  assert(floor_div(7, -3) == -3);
  assert(floor_div(-7, 3) == -3);
  assert(ceil_div(7, -3) == -2);
  assert(ceil_div(-7, 3) == -2);
  v = {3, 1, 3, 2, 1};
  sort_unique(v);
  assert((v == vector<int>{1, 2, 3}));
  assert((indices(4) == vector<int>{0, 1, 2, 3}));
  assert((argsort(vector<int>{30, 10, 20}) == vector<int>{1, 2, 0}));
  assert((argsort(vector<int>{30, 10, 20}, greater<>{}) == vector<int>{0, 2, 1}));
  multiset<int> ms{1, 2, 2, 3};
  erase_one(ms, 2);
  assert(ms.count(2) == 1);
  min_heap<int> pq;
  pq.push(3);
  pq.push(1);
  assert(pq.top() == 1);

  rng.seed(1234567);  // Fixed seed for reproducibility.
  int r = rand_int(1, 6);
  assert(1 <= r && r <= 6);
  double d = rand_real();
  assert(0.0 <= d && d < 1.0);
  shuffle(All(v), rng);
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
