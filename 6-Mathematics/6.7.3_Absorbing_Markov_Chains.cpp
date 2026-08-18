/*

An absorbing Markov chain splits its states into absorbing states, which are never left once
entered, and transient states, which are left forever with probability $1$. Ordering the transient
states first splits the transition matrix into four blocks: $Q$ holds the probabilities between
transient states, $R$ holds the probabilities from transient states into absorbing ones, and the two
remaining blocks are a zero matrix and an identity matrix, since an absorbing state only ever moves
to itself.

The expected number of visits to transient state $j$ starting from transient state $i$ is the entry
$(i, j)$ of $I + Q + Q^2 + \dots$, since the entry $(i, j)$ of $Q^k$ is the probability of being at
$j$ after exactly $k$ steps. Because absorption is certain, the powers of $Q$ tend to zero and the
series converges to the fundamental matrix $N = (I - Q)^{-1}$. Every other quantity follows from
$N$: summing a row counts the expected visits to all transient states, which is the expected number
of steps before absorption, and multiplying by $R$ takes one final step into an absorbing state.

- `fundamental_matrix(q)` returns $N = (I - `q`)^{-1}$ for the transient-to-transient matrix `q`, as
  a matrix whose entry in row $i$ and column $j$ is the expected number of visits to transient state
  $j$ when starting at transient state $i$, counting the starting visit itself.
- `absorption_steps(q)` returns a vector $t$ where $t_i$ is the expected number of steps taken from
  transient state $i$ until absorption.
- `absorption_probabilities(q, r)` returns a matrix $B$ where $B_{i,k}$ is the probability that a
  chain starting at transient state $i$ is eventually absorbed at absorbing state $k$, given the
  transient-to-absorbing matrix $R$.

Every transient state must reach an absorbing one, which is exactly what makes $I - Q$ invertible; a
state that cannot is not transient but part of a closed group to be modeled as one absorbing state,
so the zero pivot is asserted rather than reported. Unlike the backward induction of section 6.7.2,
cycles are no obstacle here, since the system is solved rather than evaluated.

Time Complexity:
- O(n^3) per call to `fundamental_matrix()` and `absorption_steps()`, where $n$ is the number of
  transient states.
- O(n^3 + n^2*k) per call to `absorption_probabilities()`, where $k$ is the number of absorbing
  states.

Space Complexity:
- O(n^2) auxiliary for `fundamental_matrix()` and `absorption_steps()`, and O(n^2 + n*k) for
  `absorption_probabilities()`.
- O(n^2) for the returned matrix from `fundamental_matrix()`, O(n) for the returned vector from
  `absorption_steps()`, and O(n*k) for the returned matrix from `absorption_probabilities()`.

*/

#include <cassert>
#include <cmath>
#include <vector>

const double EPS = 1e-9;

std::vector<std::vector<double>> fundamental_matrix(const std::vector<std::vector<double>> &q) {
  int n = static_cast<int>(q.size());
  // Reduce the augmented matrix [I - Q | I] to [I | N] by Gauss-Jordan elimination.
  std::vector<std::vector<double>> a(n, std::vector<double>(2 * n));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      a[i][j] = (i == j ? 1.0 : 0.0) - q[i][j];
    }
    a[i][n + i] = 1;
  }
  for (int col = 0; col < n; col++) {
    int pivot = col;
    for (int i = col + 1; i < n; i++) {
      if (fabs(a[i][col]) > fabs(a[pivot][col])) {
        pivot = i;
      }
    }
    assert(fabs(a[pivot][col]) > EPS);  // Some transient state never reaches an absorbing state.
    std::swap(a[col], a[pivot]);
    double scale = a[col][col];
    for (int j = col; j < 2 * n; j++) {
      a[col][j] /= scale;
    }
    for (int i = 0; i < n; i++) {
      if (i != col && a[i][col] != 0) {
        double factor = a[i][col];
        for (int j = col; j < 2 * n; j++) {
          a[i][j] -= factor * a[col][j];
        }
      }
    }
  }
  std::vector<std::vector<double>> res(n, std::vector<double>(n));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      res[i][j] = a[i][n + j];
    }
  }
  return res;
}

std::vector<double> absorption_steps(const std::vector<std::vector<double>> &q) {
  auto visits = fundamental_matrix(q);
  int n = static_cast<int>(visits.size());
  std::vector<double> res(n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      res[i] += visits[i][j];
    }
  }
  return res;
}

std::vector<std::vector<double>> absorption_probabilities(
    const std::vector<std::vector<double>> &q, const std::vector<std::vector<double>> &r
) {
  assert(q.size() == r.size());
  auto visits = fundamental_matrix(q);
  int n = static_cast<int>(visits.size()), k = n == 0 ? 0 : static_cast<int>(r[0].size());
  std::vector<std::vector<double>> res(n, std::vector<double>(k));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int c = 0; c < k; c++) {
        res[i][c] += visits[i][j] * r[j][c];
      }
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cmath>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  // A gambler with i of 4 dollars bets a dollar on a fair coin until going broke or reaching 4.
  // Transient states 0, 1, and 2 stand for holding 1, 2, and 3 dollars, and the two absorbing
  // states are ruin and victory. States 0 and 1 can each reach the other, so the transition graph
  // is cyclic and backward induction alone does not apply.
  vector<vector<double>> q{{0, 0.5, 0}, {0.5, 0, 0.5}, {0, 0.5, 0}};
  vector<vector<double>> r{{0.5, 0}, {0, 0}, {0, 0.5}};

  auto visits = fundamental_matrix(q);
  assert(EQ(visits[0][0], 1.5) && EQ(visits[0][1], 1.0) && EQ(visits[0][2], 0.5));
  assert(EQ(visits[1][0], 1.0) && EQ(visits[1][1], 2.0) && EQ(visits[1][2], 1.0));
  assert(EQ(visits[2][0], 0.5) && EQ(visits[2][1], 1.0) && EQ(visits[2][2], 1.5));

  // Starting with i dollars, the game lasts i*(4 - i) rounds on average.
  auto steps = absorption_steps(q);
  assert(EQ(steps[0], 3.0) && EQ(steps[1], 4.0) && EQ(steps[2], 3.0));

  // A fair game is won with probability i/4.
  auto odds = absorption_probabilities(q, r);
  assert(EQ(odds[0][1], 0.25) && EQ(odds[1][1], 0.5) && EQ(odds[2][1], 0.75));
  for (int i = 0; i < 3; i++) {
    assert(EQ(odds[i][0] + odds[i][1], 1.0));  // Absorption is certain.
  }
  return 0;
}
