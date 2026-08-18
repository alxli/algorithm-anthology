/*

A random process that moves between states accumulates an expected cost that satisfies one linear
equation per state. If leaving state $u$ costs $c_u$ and sends the process to state $v$ with
probability $p_{uv}$, then linearity of expectation gives $E[u] = c_u + \sum_v p_{uv} E[v]$, where
the expected cost of a state is the immediate cost plus the average of the expected costs of its
successors. Counting steps is the special case $c_u = 1$ for every non-absorbing state, and an
absorbing state is one with no outgoing transitions, so that $E = c$ there.

When the transition graph is acyclic, the equations can be evaluated instead of solved: process the
states in reverse topological order and every successor on the right-hand side is already known.
This is ordinary backward induction, and it is the reason expected-value problems are usually solved
from the goal backwards rather than from the start forwards.

A self-loop is the one cycle this method still handles. If the process stays in $u$ with probability
$q$, then $E[u] = c_u + q E[u] + \sum_{v \neq u} p_{uv} E[v]$, and isolating $E[u]$ gives
$E[u] = (c_u + \sum_{v \neq u} p_{uv} E[v]) / (1 - q)$. Equivalently, the process retries until it
leaves, so the expected number of attempts is the geometric mean $1/(1 - q)$. Self-loops appear
whenever a move can be rejected and repeated, such as a die roll that would overshoot the goal or a
draw that repeats a coupon already held.

- `expected_cost(trans, cost)` returns a vector `e` such that `e[u]` is the expected total cost
  accumulated from state `u` until the process reaches a state with no outgoing transitions. States
  are the indices of `trans`, each entry `trans[u]` lists the outgoing transitions of `u` as pairs
  of (`next_state`, `probability`), and `cost[u]` is the cost charged on leaving `u`. Transition
  probabilities out of a state must sum to at most $1$, with any missing probability treated as
  ending the process.

Aside from self-loops the transition graph must be acyclic, since a genuine cycle leaves the
equations mutually dependent and they must then be solved as a linear system; two states that reach
each other call for the absorbing Markov chains of section 6.7.3 instead.

Time Complexity:
- O(n + m) per call, where $n$ is the number of states and $m$ is the total number of transitions.

Space Complexity:
- O(n + m) auxiliary.
- O(n) for the returned vector.

*/

#include <cassert>
#include <utility>
#include <vector>

std::vector<double> expected_cost(
    const std::vector<std::vector<std::pair<int, double>>> &trans, const std::vector<double> &cost
) {
  int n = static_cast<int>(trans.size());
  assert(static_cast<int>(cost.size()) == n);
  std::vector<int> indeg(n);
  for (int u = 0; u < n; u++) {
    for (auto [v, p] : trans[u]) {
      if (v != u) {  // Self-loops are resolved algebraically, so they do not order the states.
        indeg[v]++;
      }
    }
  }
  std::vector<int> order, ready;
  order.reserve(n);
  for (int u = 0; u < n; u++) {
    if (indeg[u] == 0) {
      ready.push_back(u);
    }
  }
  while (!ready.empty()) {
    int u = ready.back();
    ready.pop_back();
    order.push_back(u);
    for (auto [v, p] : trans[u]) {
      if (v != u && --indeg[v] == 0) {
        ready.push_back(v);
      }
    }
  }
  assert(static_cast<int>(order.size()) == n);  // Cycles other than self-loops are unsupported.
  std::vector<double> res(n);
  for (int i = n - 1; i >= 0; i--) {
    int u = order[i];
    double stay = 0, total = cost[u];
    for (auto [v, p] : trans[u]) {
      if (v == u) {
        stay += p;
      } else {
        total += p * res[v];
      }
    }
    assert(stay < 1);  // A state that never leaves itself has infinite expected cost.
    res[u] = total / (1 - stay);
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
  // Roll a fair die to advance along squares [0, n], where a roll that would pass the last square
  // is rerolled. State n is absorbing, and every roll costs one step.
  const int n = 2;
  vector<vector<pair<int, double>>> board(n + 1);
  for (int i = 0; i < n; i++) {
    for (int face = 1; face <= 6; face++) {
      board[i].emplace_back(i + face <= n ? i + face : i, 1.0 / 6);
    }
  }
  vector<double> steps(n + 1, 1);
  steps[n] = 0;
  auto rolls = expected_cost(board, steps);
  assert(EQ(rolls[n], 0));
  assert(EQ(rolls[1], 6.0));  // Only one of six faces advances from square 1.
  assert(EQ(rolls[0], 6.0));
  // The same equation before the self-loop is eliminated: four of six faces reroll square 0.
  assert(EQ(rolls[0], 1 + rolls[1] / 6 + rolls[2] / 6 + 4.0 / 6 * rolls[0]));

  // The coupon collector, where state k is the number of distinct coupons already held. A draw
  // repeats a held coupon with probability k/m, which is a self-loop.
  const int m = 8;
  vector<vector<pair<int, double>>> coupons(m + 1);
  for (int k = 0; k < m; k++) {
    coupons[k].emplace_back(k, static_cast<double>(k) / m);
    coupons[k].emplace_back(k + 1, static_cast<double>(m - k) / m);
  }
  vector<double> draws(m + 1, 1);
  draws[m] = 0;
  auto collected = expected_cost(coupons, draws);
  double harmonic = 0;
  for (int i = 1; i <= m; i++) {
    harmonic += 1.0 / i;
  }
  assert(EQ(collected[0], m * harmonic));  // The classic m*H_m coupon collector bound.
  return 0;
}
