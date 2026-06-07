/*

Given a function `f(x, y)` to minimize over two real variables, return a good approximate minimum
found by simulated annealing. This is a randomized heuristic for difficult search spaces where
gradients, convexity, and unimodality are not available.

At each temperature, the algorithm proposes a random nearby point. Better moves are always accepted,
while worse moves are accepted with probability `exp((current - candidate) / temperature)`. This
lets the search sometimes escape local minima early on, then become greedier as the temperature
decreases.

Simulated annealing is not a proof of optimality. Its quality depends heavily on the objective
function, the starting point, the initial temperature, the cooling rate, and the number of
independent restarts.

- `anneal_min(f, x0, y0, &best_x, &best_y)` returns a small value found by the randomized search
  starting from `(x0, y0)`.
- `TEMPERATURE_START` controls the initial move scale and willingness to accept worse states.
- `TEMPERATURE_END` controls when the search stops.
- `COOLING_RATE` controls how quickly the temperature decreases.

Time Complexity:
- O(r log n) calls to `f()`, where $r$ is the number of restarts and $n$ is roughly
  `TEMPERATURE_START / TEMPERATURE_END`.

Space Complexity:
- O(1) auxiliary.

*/

#include <cmath>
#include <random>

const double TEMPERATURE_START = 10.0;
const double TEMPERATURE_END = 1e-7;
const double COOLING_RATE = 0.997;
const int NUM_RESTARTS = 8;

double random_unit() {
  static std::mt19937 rng(1234567);
  return std::uniform_real_distribution<double>(0.0, 1.0)(rng);
}

template<class Fn>
double anneal_min(Fn f, double x0, double y0, double *best_x = nullptr, double *best_y = nullptr) {
  double sol_x = x0, sol_y = y0, best = f(x0, y0);
  for (int restart = 0; restart < NUM_RESTARTS; restart++) {
    double x = x0, y = y0, cur = f(x, y);
    double temperature = TEMPERATURE_START;
    while (temperature > TEMPERATURE_END) {
      double nx = x + (2.0 * random_unit() - 1.0) * temperature;
      double ny = y + (2.0 * random_unit() - 1.0) * temperature;
      double next = f(nx, ny);
      double probability = exp((cur - next) / temperature);
      if (next < cur || random_unit() < probability) {
        x = nx;
        y = ny;
        cur = next;
        if (cur < best) {
          best = cur;
          sol_x = x;
          sol_y = y;
        }
      }
      temperature *= COOLING_RATE;
    }
  }

  if (best_x != nullptr) {
    *best_x = sol_x;
  }
  if (best_y != nullptr) {
    *best_y = sol_y;
  }
  return best;
}

/*** Example Usage ***/

#include <cassert>

bool close(double a, double b) {
  return fabs(a - b) < 1e-2;
}

// Smooth bowl with global minimum at f(2, -3) = 0.
double f(double x, double y) {
  return (x - 2) * (x - 2) + (y + 3) * (y + 3);
}

int main() {
  double x, y;
  double value = anneal_min(f, 0, 0, &x, &y);
  assert(value < 1e-4);
  assert(close(x, 2));
  assert(close(y, -3));
  return 0;
}
