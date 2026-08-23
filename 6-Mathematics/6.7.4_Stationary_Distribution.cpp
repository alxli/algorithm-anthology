/*

A Markov chain with no absorbing states never settles on a single state, but the fraction of time it
spends in each state does settle. That limit is the stationary distribution: the row vector $\pi$
with $\pi P = \pi$ and $\sum_i \pi_i = 1$, which is the one distribution left unchanged by a step of
the chain. It exists and is unique whenever the chain is irreducible, meaning every state can reach
every other, and it is the limit of the state distribution from any starting point once the chain is
also aperiodic.

The defining equations are one short of determining $\pi$. Each column of $\pi P = \pi$ says that
the probability flowing into a state equals the probability flowing out, and those $n$ equations sum
to the trivial identity $1 = 1$, leaving the system rank $n - 1$ and any scalar multiple of a
solution also a solution. Substituting the normalization $\sum_i \pi_i = 1$ for one of them pins
down the scale and makes the system uniquely solvable by ordinary elimination.

- `stationary_distribution(p, eps = 1e-10)` returns the stationary distribution of the $n$ by $n$
  row-stochastic transition matrix `p`, where `p[i][j]` is the probability of stepping from state
  `i` to state `j`. It returns `std::nullopt` when the system is singular to within `eps`, which
  happens when the chain has more than one closed recurrent class and therefore no unique stationary
  distribution. A reducible chain may still have a unique distribution when all states eventually
  enter the same closed class.

Two quantities follow at once: the expected number of steps to return to state $i$ is $1/\pi_i$, and
on an undirected graph walked by a uniformly random incident edge, $\pi_v$ is proportional to the
degree of $v$, which is why such a walk is uniform in the limit on a regular graph. Where the
absorbing chains of section 6.7.3 ask what happens before the process stops, this asks what happens
when it never does.

Time Complexity:
- O(n^3) per call, where $n$ is the number of states.

Space Complexity:
- O(n^2) auxiliary and O(n) for the returned distribution.

*/

#include <cassert>
#include <cmath>
#include <optional>
#include <utility>
#include <vector>

std::optional<std::vector<double>> stationary_distribution(
    const std::vector<std::vector<double>> &p, double eps = 1e-10
) {
  int n = static_cast<int>(p.size());
  assert(n == 0 || static_cast<int>(p[0].size()) == n);
  if (n == 0) {
    return std::vector<double>{};
  }
  // Build the transpose of P - I, then replace its last row with the normalization equation.
  std::vector<std::vector<double>> a(n, std::vector<double>(n + 1));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      a[i][j] = p[j][i] - (i == j ? 1.0 : 0.0);
    }
  }
  for (int j = 0; j < n; j++) {
    a[n - 1][j] = 1;
  }
  a[n - 1][n] = 1;
  for (int col = 0; col < n; col++) {
    int pivot = col;
    for (int i = col + 1; i < n; i++) {
      if (fabs(a[i][col]) > fabs(a[pivot][col])) {
        pivot = i;
      }
    }
    if (fabs(a[pivot][col]) <= eps) {
      return std::nullopt;  // Multiple closed classes make the stationary distribution nonunique.
    }
    std::swap(a[col], a[pivot]);
    double scale = a[col][col];  // Saved, since the loop below overwrites it on its first step.
    for (int j = col; j <= n; j++) {
      a[col][j] /= scale;
    }
    for (int i = 0; i < n; i++) {
      if (i != col && a[i][col] != 0) {
        double factor = a[i][col];
        for (int j = col; j <= n; j++) {
          a[i][j] -= factor * a[col][j];
        }
      }
    }
  }
  std::vector<double> res(n);
  for (int i = 0; i < n; i++) {
    res[i] = a[i][n];
  }
  return res;
}

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
#include <cmath>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  // A two-state chain that leaves state 0 with probability 1/4 and state 1 with probability 1/2,
  // so it rests in state 0 twice as often.
  auto two = stationary_distribution({{0.75, 0.25}, {0.5, 0.5}});
  assert(two.has_value());
  assert(EQ((*two)[0], 2.0 / 3) && EQ((*two)[1], 1.0 / 3));

  // A random walk on a path of three vertices, where the middle vertex has twice the degree and so
  // twice the stationary probability. Its expected return time is the reciprocal, 2.
  auto path = stationary_distribution({{0, 1, 0}, {0.5, 0, 0.5}, {0, 1, 0}});
  assert(path.has_value());
  assert(EQ((*path)[0], 0.25) && EQ((*path)[1], 0.5) && EQ((*path)[2], 0.25));
  assert(EQ(1 / (*path)[1], 2.0));

  // A doubly stochastic chain is uniform in the limit no matter the transition probabilities.
  auto cycle = stationary_distribution({{0, 0.3, 0.7}, {0.7, 0, 0.3}, {0.3, 0.7, 0}});
  assert(cycle.has_value());
  assert(all_of(cycle->begin(), cycle->end(), [](double p) { return EQ(p, 1.0 / 3); }));

  // Two states that never reach each other admit no unique distribution.
  assert(!stationary_distribution({{1, 0}, {0, 1}}).has_value());

  // Reducibility alone is not an obstruction: state 0 is transient and state 1 is the sole class.
  auto reducible = stationary_distribution({{0, 1}, {0, 1}});
  assert(reducible.has_value() && EQ((*reducible)[0], 0) && EQ((*reducible)[1], 1));
  return 0;
}
