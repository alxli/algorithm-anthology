/*

Provides random input generators and a harness that compares an optimized solution against a trusted
brute force implementation. Small tests keep brute force practical and produce manageable
counterexamples. The generators create common contest inputs such as distinct samples, permutations,
graphs, trees, and integer compositions for stress tests, randomized tests, and benchmarks.

- `rng` is a global 64-bit Mersenne Twister. Call `rng.seed(seed)` before a stress test to make it
  reproducible. Use an explicit seed so generated failures are reproducible.
- `rand_int(lo, hi)` returns a uniformly random integer in range $[`lo`, `hi`]$.
- `rand_biased(lo, hi, bias)` returns an integer in $[`lo`, `hi`]$. Positive `bias` favors larger
  values by taking the maximum of `bias + 1` samples, negative `bias` similarly favors smaller
  values, and zero is uniform.
- `rand_real(lo = 0.0, hi = 1.0)` returns a uniformly random real number in the half-open range
  $[`lo`, `hi`)$.
- `rand_choice(values)` returns a uniformly random element of the nonempty vector `values`.
- `rand_weighted(weights)` returns a sampled 0-based index with probability proportional to its
  nonnegative weight.
- `rand_vec(n, lo, hi)` returns `n` independent uniformly random integers in $[`lo`, `hi`]$.
- `rand_str(n, alphabet)` returns a length-`n` string whose characters are sampled independently and
  uniformly from the nonempty string `alphabet`.
- `rand_perm(n, first = 0)` returns a random permutation of the $n$ consecutive integers beginning
  at `first`.
- `rand_distinct(k, lo, hi)` returns `k` distinct integers sampled uniformly from $[`lo`, `hi`]$.
- `rand_tree(n)` returns a uniformly random labeled tree on nodes $[0, `n`)$ as an edge list using a
  Prüfer code.
- `rand_graph(n, m, directed = false, connected = false)` returns a random simple graph with `n`
  nodes numbered $[0, `n`)$ and exactly `m` edges. Without connectivity it is uniform; with
  connectivity it starts from `rand_tree(n)` and is not uniform over connected graphs. A connected
  directed graph is weakly connected.
- `rand_composition(parts, sum, min_part)` uniformly splits `sum` into `parts` ordered integers,
  each at least `min_part`.

Time Complexity:
- O(1) per call to `rand_int()`, `rand_real()`, and `rand_choice()`.
- O(|b| + 1) per call to `rand_biased()` where $b$ is the bias.
- O(n) per call to `rand_weighted()`, `rand_vec()`, `rand_str()`, and `rand_perm()`, where $n$ is
  the number of weights or generated elements.
- O(k) expected per call to `rand_distinct()`.
- O(n log n) per call to `rand_tree()`.
- O(m) expected per call to `rand_graph()` without connectivity, or O(n log n + m) with it.
- O(k log k) per call to `rand_composition()`, where $k$ is `parts`.

Space Complexity:
- O(1) auxiliary for the scalar generators and `rand_choice()`.
- O(n) auxiliary for `rand_weighted()`.
- O(n) for the values returned by `rand_vec()`, `rand_str()`, `rand_perm()`, and `rand_tree()`.
- O(k) auxiliary and output space for `rand_distinct()` and `rand_composition()`.
- O(m) auxiliary and output space for `rand_graph()`.

*/

#include <algorithm>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

std::mt19937_64 rng;

template<typename Int>
Int rand_int(Int lo, Int hi) {
  static_assert(std::is_integral<Int>::value, "rand_int() requires an integral type");
  assert(lo <= hi);
  return std::uniform_int_distribution<Int>(lo, hi)(rng);
}

template<typename Int>
Int rand_biased(Int lo, Int hi, int bias) {
  Int value = rand_int(lo, hi);
  while (bias > 0) {
    value = std::max(value, rand_int(lo, hi));
    bias--;
  }
  while (bias < 0) {
    value = std::min(value, rand_int(lo, hi));
    bias++;
  }
  return value;
}

double rand_real(double lo = 0.0, double hi = 1.0) {
  assert(lo < hi);
  return std::uniform_real_distribution<double>(lo, hi)(rng);
}

template<typename T>
T rand_choice(const std::vector<T> &values) {
  assert(!values.empty() && values.size() <= INT_MAX);
  return values[rand_int(0, static_cast<int>(values.size()) - 1)];
}

template<typename W>
int rand_weighted(const std::vector<W> &weights) {
  static_assert(std::is_arithmetic<W>::value, "weights must be numeric");
  assert(!weights.empty() && weights.size() <= INT_MAX);
  std::vector<double> probabilities;
  probabilities.reserve(weights.size());
  double total = 0;
  for (const W &value : weights) {
    double weight = static_cast<double>(value);
    assert(std::isfinite(weight) && weight >= 0);
    probabilities.push_back(weight);
    total += weight;
  }
  assert(std::isfinite(total) && total > 0);
  return std::discrete_distribution<int>(probabilities.begin(), probabilities.end())(rng);
}

template<typename W>
int rand_weighted(std::initializer_list<W> weights) {
  return rand_weighted(std::vector<W>(weights));
}

template<typename Int>
std::vector<Int> rand_vec(int n, Int lo, Int hi) {
  assert(n >= 0 && lo <= hi);
  std::vector<Int> values(n);
  for (Int &x : values) {
    x = rand_int(lo, hi);
  }
  return values;
}

std::string rand_str(int n, const std::string &alphabet) {
  assert(n >= 0 && !alphabet.empty() && alphabet.size() <= INT_MAX);
  std::string s(n, ' ');
  for (char &c : s) {
    c = alphabet[rand_int(0, static_cast<int>(alphabet.size()) - 1)];
  }
  return s;
}

std::vector<int> rand_perm(int n, int first = 0) {
  assert(n >= 0 && (n == 0 || static_cast<int64_t>(first) + n - 1 <= INT_MAX));
  std::vector<int> p(n);
  std::iota(p.begin(), p.end(), first);
  std::shuffle(p.begin(), p.end(), rng);
  return p;
}

std::vector<int> rand_distinct(int k, int lo, int hi) {
  int64_t count = static_cast<int64_t>(hi) - lo + 1;
  assert(lo <= hi && 0 <= k && k <= count);
  std::unordered_set<int> used;
  used.reserve(k);
  std::vector<int> values;
  values.reserve(k);
  // Floyd's algorithm avoids materializing the entire interval.
  for (int64_t j = static_cast<int64_t>(hi) - k + 1; j <= hi; ++j) {
    int x = rand_int(lo, static_cast<int>(j));
    if (!used.insert(x).second) {
      x = static_cast<int>(j);
      used.insert(x);
    }
    values.push_back(x);
  }
  std::shuffle(values.begin(), values.end(), rng);
  return values;
}

std::vector<std::pair<int, int>> rand_tree(int n) {
  assert(n >= 1);
  if (n == 1) {
    return {};
  }
  std::vector<int> degree(n, 1), code(n - 2);
  for (int &u : code) {
    degree[u = rand_int(0, n - 1)]++;
  }
  std::priority_queue<int, std::vector<int>, std::greater<int>> leaves;
  for (int u = 0; u < n; ++u) {
    if (degree[u] == 1) {
      leaves.push(u);
    }
  }
  std::vector<std::pair<int, int>> edges;
  for (int u : code) {
    int leaf = leaves.top();
    leaves.pop();
    edges.emplace_back(leaf, u);
    if (--degree[u] == 1) {
      leaves.push(u);
    }
  }
  int u = leaves.top();
  leaves.pop();
  edges.emplace_back(u, leaves.top());
  std::shuffle(edges.begin(), edges.end(), rng);
  return edges;
}

std::vector<std::pair<int, int>> rand_graph(
    int n, int m, bool directed = false, bool connected = false
) {
  assert(n >= 0 && m >= 0 && (!connected || n >= 1));
  int64_t total =
      directed ? static_cast<int64_t>(n) * (n - 1) : static_cast<int64_t>(n) * (n - 1) / 2;
  int required = connected ? n - 1 : 0;
  assert(required <= m && m <= total);
  std::vector<std::pair<int, int>> edges;
  if (connected) {
    edges = rand_tree(n);
  }
  std::unordered_set<int64_t> required_edges;
  required_edges.reserve(required);
  for (auto &[u, v] : edges) {
    if ((directed && rand_int(0, 1)) || (!directed && u > v)) {
      std::swap(u, v);
    }
    required_edges.insert(static_cast<int64_t>(u) * n + v);
  }
  int64_t available = total - required;
  int extra = m - required;
  bool complement = extra > available / 2;
  int sample = complement ? static_cast<int>(total - m) : extra;
  // For dense graphs, sample the omitted non-tree edges instead of the included ones.
  std::unordered_set<int64_t> sampled;
  sampled.reserve(sample);
  while (static_cast<int>(sampled.size()) < sample) {
    int u = rand_int(0, n - 1), v = rand_int(0, n - 2);
    if (v >= u) {
      v++;
    }
    if (!directed && u > v) {
      std::swap(u, v);
    }
    int64_t key = static_cast<int64_t>(u) * n + v;
    if (!required_edges.count(key)) {
      sampled.insert(key);
    }
  }
  edges.reserve(m);
  if (!complement) {
    for (int64_t key : sampled) {
      edges.emplace_back(static_cast<int>(key / n), static_cast<int>(key % n));
    }
  } else {
    edges.clear();
    for (int u = 0; u < n; u++) {
      for (int v = directed ? 0 : u + 1; v < n; v++) {
        if (u != v && !sampled.count(static_cast<int64_t>(u) * n + v)) {
          edges.emplace_back(u, v);
        }
      }
    }
  }
  std::shuffle(edges.begin(), edges.end(), rng);
  return edges;
}

std::vector<int> rand_composition(int parts, int sum, int min_part) {
  int64_t remaining = static_cast<int64_t>(sum) - static_cast<int64_t>(parts) * min_part;
  assert(parts >= 1 && min_part >= 0 && remaining >= 0 && remaining + parts - 2 <= INT_MAX);
  if (parts == 1) {
    return {sum};
  }
  auto cuts = rand_distinct(parts - 1, 0, static_cast<int>(remaining) + parts - 2);
  std::sort(cuts.begin(), cuts.end());
  std::vector<int> values;
  int prev = -1;
  for (int cut : cuts) {
    values.push_back(min_part + cut - prev - 1);
    prev = cut;
  }
  values.push_back(min_part + static_cast<int>(remaining) + parts - 2 - prev);
  return values;
}

/*

A stress test compares an optimized implementation against a trusted brute force implementation on
many generated inputs. Small tests make brute force practical and produce manageable
counterexamples.

- `stress(trials, gen, solve, brute, equal = std::equal_to<>())` runs `trials` tests, calling
  `gen(trial)` with each 1-based trial number and comparing `solve(test)` with `brute(test)` using
  `equal`. It returns a `StressFailure` containing the number, first failing test, and actual and
  expected results. The generated test type must be copyable so each solution receives an
  independent input, and `equal` must accept the respective results of `solve(test)` and
  `brute(test)`. If all generated tests pass, it returns `std::nullopt`.

Time Complexity:
- O(t*(G + X + S + B + C)) per call in the worst case, where $t$ is `trials`, $X$ is the cost of
  copying a generated test, and $G$, $S$, $B$, and $C$ are the costs of `gen`, `solve`, `brute`, and
  `equal`, respectively.

Space Complexity:
- O(x + a + e) for the test copies and results, where $x$, $a$, and $e$ are their respective sizes,
  in addition to the working space used by the callbacks.

*/

template<typename Test, typename Actual, typename Expected>
struct StressFailure {
  int trial;
  Test test;
  Actual actual;
  Expected expected;
};

template<typename Generate, typename Solve, typename Brute, typename Equal = std::equal_to<>>
auto stress(int trials, Generate gen, Solve solve, Brute brute, Equal equal = {}) {
  assert(trials >= 0);
  using Test = std::decay_t<decltype(gen(1))>;
  using Actual = std::decay_t<decltype(solve(std::declval<Test &>()))>;
  using Expected = std::decay_t<decltype(brute(std::declval<Test &>()))>;
  using Failure = StressFailure<Test, Actual, Expected>;
  for (int trial = 1; trial <= trials; ++trial) {
    auto test = gen(trial);
    auto solve_test = test, brute_test = test;
    auto actual = solve(solve_test);
    auto expected = brute(brute_test);
    if (!equal(actual, expected)) {
      return std::optional<Failure>{
          Failure{trial, std::move(test), std::move(actual), std::move(expected)}
      };
    }
  }
  return std::optional<Failure>{};
}

/*** Example Usage ***/

using namespace std;

bool EQ(double a, double b) {
  return a == b || fabs(a - b) <= 1e-9;
}

int main() {
  rng.seed(1234567);  // Fixed seed for reproducibility.

  double real = rand_real(-1.0, 1.0);
  assert(-1.0 <= real && real < 1.0);
  int biased = rand_biased(1, 100000, 3);
  assert(1 <= biased && biased <= 100000);
  assert(rand_weighted({20, 20, 30}) < 3);
  auto values = rand_vec(8, -5, 5);
  assert(values.size() == 8 && all_of(values.begin(), values.end(), [](int x) {
           return -5 <= x && x <= 5;
         }));
  assert(rand_str(12, "abc").size() == 12);
  auto graph = rand_graph(5, 7);
  assert(graph.size() == 7 && all_of(graph.begin(), graph.end(), [](auto edge) {
           return edge.first < edge.second;
         }));
  assert(rand_tree(6).size() == 5);
  assert(rand_graph(6, 8, false, true).size() == 8);
  auto composition = rand_composition(4, 20, 2);
  assert(accumulate(composition.begin(), composition.end(), 0) == 20);
  assert(*min_element(composition.begin(), composition.end()) >= 2);

  auto generate = [](int trial) {
    int n = rand_int(1, 8), kind = trial <= 3 ? trial - 1 : rand_int(0, 2);
    switch (kind) {
      case 0:
        return rand_perm(n, 1);
      case 1:
        return rand_distinct(n, -5, 5);
      default:
        break;
    }
    static const vector<int> boundary_values{-5, 0, 5};
    bernoulli_distribution use_boundary(0.25);
    vector<int> a(n);
    for (int &x : a) {
      x = use_boundary(rng) ? rand_choice(boundary_values) : rand_int(-5, 5);
    }
    return a;
  };
  auto solve = [](vector<int> a) {
    sort(a.begin(), a.end());
    return a;
  };
  auto brute = [](vector<int> a) {
    do {
      if (is_sorted(a.begin(), a.end())) {
        return a;
      }
    } while (next_permutation(a.begin(), a.end()));
    sort(a.begin(), a.end());
    return a;
  };

  assert(!stress(100, generate, solve, brute));
  assert(!stress(
      1, [](int) { return 0; }, [](int) { return 1.0; }, [](int) { return 1.0 + 1e-12; }, EQ
  ));

  // Mutating solvers still receive independent copies of the generated test.
  auto mutate = [](vector<int> &a) { return ++a[0]; };
  auto unchanged = [](const vector<int> &a) { return a[0]; };
  auto failure = stress(1, [](int) { return vector<int>{1}; }, mutate, unchanged);
  assert(failure && failure->trial == 1 && failure->test == vector<int>{1});
  assert(failure->actual == 2 && failure->expected == 1);
  return 0;
}
