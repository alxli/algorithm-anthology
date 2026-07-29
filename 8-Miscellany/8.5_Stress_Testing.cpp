/*

A simple stress-testing harness compares a trusted brute force implementation against an optimized
implementation on many generated tests. This is most useful for finding small counterexamples while
developing an algorithm.

- `stress(trials, generate, solve, brute)` runs `generate()` for each trial, compares `solve(test)`
  with `brute(test)`, and returns the first failing test if one is found.
- `stress(...)` returns `std::nullopt` if all generated tests pass.

The generated test type must be copyable so each solution receives an independent input, and both
solutions must return comparable results.

*/

#include <optional>

template<typename Generate, typename Solve, typename Brute>
auto stress(int trials, Generate generate, Solve solve, Brute brute)
    -> std::optional<decltype(generate())> {
  for (int tc = 0; tc < trials; ++tc) {
    auto test = generate();
    auto solve_test = test, brute_test = test;
    if (solve(solve_test) != brute(brute_test)) {
      return test;
    }
  }
  return std::nullopt;
}

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>
using namespace std;

int main() {
  std::mt19937 rng(1234567);  // Fixed seed for reproducibility.

  auto generate = [&] {
    int n = std::uniform_int_distribution<int>(1, 8)(rng);
    std::vector<int> a(n);
    for (int &x : a) {
      x = std::uniform_int_distribution<int>(-5, 5)(rng);
    }
    return a;
  };
  auto solve = [](std::vector<int> a) {
    std::sort(a.begin(), a.end());
    return a;
  };
  auto brute = [](std::vector<int> a) {
    do {
      if (std::is_sorted(a.begin(), a.end())) {
        return a;
      }
    } while (std::next_permutation(a.begin(), a.end()));
    std::sort(a.begin(), a.end());
    return a;
  };

  assert(!stress(100, generate, solve, brute));

  // Mutating solvers still receive independent copies of the generated test.
  auto mutate = [](std::vector<int> &a) {
    a[0]++;
    return a[0];
  };
  auto unchanged = [](const std::vector<int> &a) { return a[0]; };
  assert(stress(1, [] { return std::vector<int>{1}; }, mutate, unchanged));
  return 0;
}
