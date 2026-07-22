/*

Approximately minimizes an energy function $E(s)$ over an arbitrary state space using simulated
annealing. This randomized heuristic is useful when the states may be discrete, the energy has many
local minima, and gradients or stronger structural properties are unavailable.

Starting from an initial state $s$, each iteration generates a random neighboring state $s'$. An
improving move is always accepted, while a worsening move is accepted with probability
$\exp(-(E(s') - E(s))/T)$ at temperature $T$. High temperatures encourage exploration across energy
barriers; as the temperature decreases, the search increasingly favors improvements. The best state
is tracked separately because the current state may later move to a worse one.

- `anneal_min(initial, energy, rand_neighbor, rng, ...)` returns (`best_energy`, `best_state`) given
  an initial state `initial`, a callable `energy(state)` that returns the state's energy, and a
  callable `rand_neighbor(state, temp, rng)` that returns a randomly chosen nearby state. Optional
  parameters default to `temp_start = 1000`, `temp_end = 1e-6`, and `cooling_rate = 0.995`.
  `temp_end` must be positive and less than `temp_start`, while `cooling_rate` must be strictly
  between $0$ and $1$.

The geometric schedule repeatedly multiplies the temperature by `cooling_rate`, performing
$k = \lceil \log(T_{end}/T_{start}) / \log(r) \rceil$ iterations. Temperature has the same scale as
energy: multiplying every energy by a constant requires multiplying both temperature bounds by the
same constant to preserve acceptance probabilities. For continuous states, temperature can also
control the neighbor step size. Slower cooling, independent restarts, and a time-based stopping
condition often improve results, but simulated annealing never proves optimality.

This generic interface copies each proposed state and recomputes its energy. For large permutation
states, adapt the loop to mutate and revert moves in place and update the energy from the affected
terms instead.

Time Complexity:
- O(k(C_E + C_N)), where $k$ is the number of temperature levels and $C_E$ and $C_N$ are the costs
  of evaluating the energy and generating a neighboring state.

Space Complexity:
- O(S) for the current, next, and best states, where $S$ is the size of one state.

*/

#include <cassert>
#include <cmath>
#include <random>
#include <utility>

template<typename State, typename Energy, typename Neighbor>
std::pair<double, State> anneal_min(
    State initial, Energy energy, Neighbor rand_neighbor, std::mt19937 &rng,
    const double temp_start = 1000, const double temp_end = 1e-6, const double cooling_rate = 0.995
) {
  assert(0 < temp_end && temp_end < temp_start);
  assert(0 < cooling_rate && cooling_rate < 1);
  std::uniform_real_distribution<double> unit(0.0, 1.0);
  State current = initial, best = initial;
  double current_energy = energy(current), best_energy = current_energy;
  for (double temp = temp_start; temp > temp_end; temp *= cooling_rate) {
    State next = rand_neighbor(current, temp, rng);
    double next_energy = energy(next);
    if (next_energy <= current_energy ||
        unit(rng) < std::exp((current_energy - next_energy) / temp)) {
      current = next;
      current_energy = next_energy;
      if (current_energy < best_energy) {
        best = current;
        best_energy = current_energy;
      }
    }
  }
  return {best_energy, best};
}

/*** Example Usage ***/

#include <algorithm>
#include <vector>
using namespace std;

int main() {
  mt19937 rng(1234567);  // Fixed seed for reproducibility.

  // Escape the local minimum near x = 1 and find the lower minimum near x = -1.
  auto energy_1d = [](double x) { return (x * x - 1) * (x * x - 1) + 0.2 * x; };
  auto neighbor_1d = [](double x, double temp, mt19937 &gen) {
    uniform_real_distribution<double> move(-1.0, 1.0);
    return x + move(gen) * sqrt(temp);
  };
  auto [best_energy, best_x] = anneal_min(1.0, energy_1d, neighbor_1d, rng, 2.0, 1e-8);
  assert(best_energy < -0.2 && best_x < -1);

  // Approximate TSP with a permutation state and random 2-opt reversals.
  vector<pair<double, double>> points{{0, 0}, {1, 0}, {2, 0}, {2, 1},
                                      {2, 2}, {1, 2}, {0, 2}, {0, 1}};
  auto tour_length = [&](const vector<int> &tour) {
    double length = 0;
    for (int i = 0; i < static_cast<int>(tour.size()); i++) {
      auto [x1, y1] = points[tour[i]];
      auto [x2, y2] = points[tour[(i + 1) % tour.size()]];
      length += hypot(x1 - x2, y1 - y2);
    }
    return length;
  };
  auto reverse_segment = [](const vector<int> &tour, double, mt19937 &gen) {
    vector<int> next = tour;
    uniform_int_distribution<int> pick(0, static_cast<int>(tour.size()) - 1);
    int lo = pick(gen), hi = pick(gen);
    if (lo > hi) {
      swap(lo, hi);
    }
    reverse(next.begin() + lo, next.begin() + hi + 1);
    return next;
  };
  vector<int> initial_tour{0, 4, 2, 6, 1, 5, 3, 7};
  auto [best_length, best_tour] =
      anneal_min(initial_tour, tour_length, reverse_segment, rng, 5.0, 1e-6, 0.995);
  assert(fabs(best_length - 8) < 1e-9);
  assert(fabs(tour_length(best_tour) - best_length) < 1e-9);
  return 0;
}
