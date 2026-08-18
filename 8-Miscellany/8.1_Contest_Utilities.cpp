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
- `ckmin(a, b)` assigns `a = b` only when `b` < `a`, and returns whether the assignment happened.
- `ckmax(a, b)` assigns `a = b` only when `a` < `b`, and returns whether the assignment happened.
- `floor_div(a, b)` and `ceil_div(a, b)` divide signed integers with mathematical rounding toward
  negative or positive infinity. Requires nonzero `b` and the resulting quotient to be representable
  by the result type; in particular, the minimum value cannot be divided by $-1$.
- `make_vec(n1, ..., nk, v)` returns a `k`-dimensional nested vector with the given sizes and every
  element initialized to `v`, whose type is deduced from `v`. The last argument is always the fill
  value, so `make_vec(3, 5)` is a vector of three fives rather than a $3$ by $5$ grid.
- `indices(n)` returns the vector $[0, 1, \ldots, `n` - 1]$.
- `lb(a, x, comp = std::less<>)` and `ub(a, x, comp = std::less<>)` return lower and upper bound
  indices in random-access sequence `a`, which must be sorted according to `comp`.
- `sort_unique(v, comp = std::less<>)` sorts a vector and removes duplicates, which are the elements
  that `comp` deems equivalent.
- `argsort(a, comp = std::less<>)` returns the indices of `a` ordered by their values according to
  `comp`; the order of indices whose values compare equal is unspecified.
- `find_ptr(m, k)` returns a pointer to the value that key `k` maps to in the associative container
  `m`, or `nullptr` if the key is absent. The pointer is const only when `m` is, so
  `if (auto *p = find_ptr(m, k))` both tests for the key and updates it in a single lookup.
- `get_or(m, k, def = {})` returns the value that key `k` maps to in the associative container `m`,
  or `def` if the key is absent. Unlike `operator[]`, it never inserts and works on a const `m`.
- `erase_one(c, x)` erases one copy of `x` from associative container `c`, asserting that it is
  present.
- `min_heap<T>` is a min-heap alias.
- `y_combinator(f)` wraps a recursive lambda so that it can call itself as its first argument.

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

// clang-format off
#define Rep(i, N)    for (int i = 0, _##i = (N); i < _##i; ++i)     //   0 to N-1
#define For(i, L, H) for (int i = (L), _##i = (H); i <= _##i; ++i)  //   L to H
#define Rev(i, N)    for (int i = (N); --i >= 0;)                   // N-1 to 0
#define Dwn(i, H, L) for (int i = (H), _##i = (L); i >= _##i; --i)  //   H to L
#define Each(x, C)   for (auto &x : (C))
#define All(C)       std::begin(C), std::end(C)
#define Rall(C)      std::rbegin(C), std::rend(C)

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

template<typename T>
std::vector<T> make_vec(int n, const T &v) {
  return std::vector<T>(n, v);
}

template<typename... Args>
auto make_vec(int n, Args... args) {
  auto inner = make_vec(args...);
  return std::vector<decltype(inner)>(n, inner);
}

std::vector<int> indices(int n) {
  std::vector<int> v(n);
  std::iota(v.begin(), v.end(), 0);
  return v;
}

template<typename Seq, typename T, typename Compare = std::less<>>
int lb(const Seq &a, const T &x, Compare comp = Compare{}) {
  return static_cast<int>(std::lower_bound(a.begin(), a.end(), x, comp) - a.begin());
}

template<typename Seq, typename T, typename Compare = std::less<>>
int ub(const Seq &a, const T &x, Compare comp = Compare{}) {
  return static_cast<int>(std::upper_bound(a.begin(), a.end(), x, comp) - a.begin());
}

template<typename T, typename Compare = std::less<>>
void sort_unique(std::vector<T> &v, Compare comp = Compare{}) {
  std::sort(v.begin(), v.end(), comp);
  auto same = [&](const T &a, const T &b) { return !comp(a, b) && !comp(b, a); };
  v.erase(std::unique(v.begin(), v.end(), same), v.end());
}

template<typename Seq, typename Compare = std::less<>>
std::vector<int> argsort(const Seq &a, Compare comp = Compare{}) {
  std::vector<int> p = indices(static_cast<int>(a.size()));
  std::sort(p.begin(), p.end(), [&](int i, int j) { return comp(a[i], a[j]); });
  return p;
}

template<typename M>
auto find_ptr(M &m, const typename M::key_type &k) {
  auto it = m.find(k);
  return it == m.end() ? nullptr : &it->second;
}

template<typename M>
typename M::mapped_type get_or(
    const M &m, const typename M::key_type &k, const typename M::mapped_type &def = {}
) {
  auto it = m.find(k);
  return it == m.end() ? def : it->second;
}

template<typename C, typename T>
void erase_one(C &c, const T &x) {
  auto it = c.find(x);
  assert(it != c.end());
  c.erase(it);
}

template<typename T>
using min_heap = std::priority_queue<T, std::vector<T>, std::greater<T>>;

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

#include <map>
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

  auto grid = make_vec(2, 3, -1);
  grid[0][0] = 7;
  assert(sz(grid) == 2 && sz(grid[1]) == 3 && grid[1][0] == -1);
  v = {3, 1, 3, 2, 1};
  sort_unique(v);
  assert((v == vector<int>{1, 2, 3}));
  v = {3, 1, 3, 2, 1};
  sort_unique(v, greater<>{});
  assert((v == vector<int>{3, 2, 1}));
  assert((indices(4) == vector<int>{0, 1, 2, 3}));
  assert((argsort(vector<int>{30, 10, 20}) == vector<int>{1, 2, 0}));
  assert((argsort(vector<int>{30, 10, 20}, greater<>{}) == vector<int>{0, 2, 1}));

  map<int, int> cnt{{2, 5}};
  if (int *p = find_ptr(cnt, 2)) {
    *p += 10;  // A single lookup both tests for the key and updates it.
  }
  const map<int, int> &view = cnt;  // A const container yields a pointer to const.
  assert(*find_ptr(view, 2) == 15 && find_ptr(view, 7) == nullptr);
  assert(get_or(cnt, 2) == 15 && get_or(cnt, 7, -1) == -1 && sz(cnt) == 1);
  min_heap<int> pq;
  pq.push(3);
  pq.push(1);
  assert(pq.top() == 1);

  multiset<int> ms{1, 2, 2, 3};
  erase_one(ms, 2);
  assert(ms.count(2) == 1);

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
