/*

Given a unimodal function $f$, find the position of its global minimum or maximum: a value $x$ such
that the function strictly decreases up to $x$ and strictly increases after it (for a minimum), or
the reverse (for a maximum). All three searches below exploit the same idea: evaluate two interior
points, and the comparison reveals one side of the interval that cannot contain the optimum, so it
can be discarded. They differ only in how the probes are placed and whether the domain is continuous
or discrete. Each maximum routine is the minimum routine applied to the negated function, since the
location of a maximum of $f$ is the location of a minimum of $-f$.

Ternary search splits the interval into thirds. It is the simplest to reason about, but it
recomputes both probe points each step, spending two function evaluations per iteration while the
interval shrinks to two-thirds.

Golden-section search places the probes at the golden ratio so that one probe is reused as an
interior point of the next, smaller interval. It therefore performs only one new evaluation per
iteration while shrinking the interval by a factor of $1/\varphi$ (about 0.618), and reaches a given
accuracy in roughly 2.4 times fewer evaluations than ternary search. Prefer it when the function is
expensive to evaluate.

The discrete version operates on an integer domain, narrowing by thirds until at most three
candidates remain and then scanning them. It returns the index of the optimum rather than a real
coordinate. As with ternary search on reals, the function must be strictly unimodal.

- `ternary_search_min(lo, hi, f, eps = 1e-12)` and `ternary_search_max(lo, hi, f, eps = 1e-12)`
  return a `double` `x` in $[`lo`, `hi`]$ optimizing the continuous unimodal function `f`, to within
  the optional absolute error `eps`.
- `golden_section_min(lo, hi, f, eps = 1e-12)` and
  `golden_section_max(lo, hi, f, eps = 1e-12)` do the same with one function evaluation per
  iteration.
- `discrete_ternary_min(lo, hi, f)` and `discrete_ternary_max(lo, hi, f)` return the integer index
  in the inclusive range $[`lo`, `hi`]$ optimizing the unimodal function `f`, breaking ties toward
  the smaller index.

Time Complexity:
- O(log(n / `eps`)) calls to `f()` for the continuous searches, where $n$ is the distance between
  `lo` and `hi`. Golden-section makes about half as many per iteration as ternary search.
- O(log n) calls to `f()` for the discrete searches.

Space Complexity:
- O(1) auxiliary for all operations.

*/

template<typename Fn>
double ternary_search_min(double lo, double hi, Fn f, double eps = 1e-12) {
  while (hi - lo > eps) {
    double m1 = lo + (hi - lo) / 3, m2 = hi - (hi - lo) / 3;
    if (f(m1) < f(m2)) {
      hi = m2;
    } else {
      lo = m1;
    }
  }
  return (lo + hi) / 2;
}

template<typename Fn>
double ternary_search_max(double lo, double hi, Fn f, double eps = 1e-12) {
  return ternary_search_min(lo, hi, [&](double x) { return -f(x); }, eps);
}

template<typename Fn>
double golden_section_min(double lo, double hi, Fn f, double eps = 1e-12) {
  const double r = 0.6180339887498949;  // 1 / phi = (sqrt(5) - 1) / 2.
  double m1 = hi - r * (hi - lo), m2 = lo + r * (hi - lo);
  double f1 = f(m1), f2 = f(m2);
  while (hi - lo > eps) {
    if (f1 < f2) {
      hi = m2;
      m2 = m1;
      f2 = f1;
      m1 = hi - r * (hi - lo);
      f1 = f(m1);
    } else {
      lo = m1;
      m1 = m2;
      f1 = f2;
      m2 = lo + r * (hi - lo);
      f2 = f(m2);
    }
  }
  return (lo + hi) / 2;
}

template<typename Fn>
double golden_section_max(double lo, double hi, Fn f, double eps = 1e-12) {
  return golden_section_min(lo, hi, [&](double x) { return -f(x); }, eps);
}

template<typename Int, typename Fn>
Int discrete_ternary_min(Int lo, Int hi, Fn f) {
  while (hi - lo > 2) {
    Int m1 = lo + (hi - lo) / 3, m2 = hi - (hi - lo) / 3;
    if (f(m1) < f(m2)) {
      hi = m2 - 1;
    } else {
      lo = m1 + 1;
    }
  }
  Int best = lo;
  auto best_value = f(best);
  for (Int k = lo + 1; k <= hi; k++) {
    auto value = f(k);
    if (value < best_value) {
      best = k;
      best_value = value;
    }
  }
  return best;
}

template<typename Int, typename Fn>
Int discrete_ternary_max(Int lo, Int hi, Fn f) {
  return discrete_ternary_min(lo, hi, [&](Int x) { return -f(x); });
}

/*** Example Usage ***/

#include <cassert>
#include <cmath>
#include <vector>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-6;
}

int main() {
  // Parabola opening up with vertex at x = -2.
  auto up = [](double x) { return 3 * x * x + 12 * x - 12; };
  // Parabola opening down with vertex at x = 2/19.
  auto down = [](double x) { return -5.7 * x * x + 1.2 * x + 88; };

  assert(EQ(ternary_search_min(-1000, 1000, up), -2));
  assert(EQ(golden_section_min(-1000, 1000, up), -2));
  assert(EQ(ternary_search_max(-1000, 1000, down), 2.0 / 19));
  assert(EQ(golden_section_max(-1000, 1000, down), 2.0 / 19));

  // Discrete unimodal arrays: a valley and a peak.
  vector<int> valley{5, 3, 1, 2, 4, 6};
  vector<int> peak{1, 3, 7, 6, 2};
  auto v = [&](int i) { return valley[i]; };
  auto p = [&](int i) { return peak[i]; };
  assert(discrete_ternary_min(0, static_cast<int>(valley.size()) - 1, v) == 2);  // Value 1.
  assert(discrete_ternary_max(0, static_cast<int>(peak.size()) - 1, p) == 2);    // Value 7.
  return 0;
}
