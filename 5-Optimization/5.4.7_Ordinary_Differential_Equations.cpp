/*

Given an initial value $y(t_0) = y_0$ and a derivative $y'(t) = f(t, y)$, an initial value problem
asks for $y$ at some later time. Since only the slope is known, the solution is traced by stepping
forward: estimate the slope over a short interval `h`, move along it, and repeat. The methods below
differ only in how many slope samples each step averages, which is what determines how quickly the
error vanishes as `h` shrinks.

The forward Euler method takes the slope at the current point and follows it for the whole step.
Each step therefore misses the curvature of the solution, leaving an error per step proportional to
$h^2$ that accumulates to O(h) over a fixed interval. The midpoint method spends one extra
evaluation to sample the slope halfway across the step and uses that instead, which cancels the
leading error term and leaves O(h^2). Classical Runge-Kutta combines four samples, one at the start,
two at the middle, and one at the end, weighted so that every error term through $h^4$ cancels,
leaving O(h^4). Halving $h$ therefore buys $16$ times the accuracy for twice the work, which is why
this method is the usual default.

The state is a vector, so a system of equations is integrated exactly like a single one. A
higher-order equation becomes such a system by naming its derivatives: $y'' = g(t, y, y')$ turns
into the pair $y_0' = y_1$ and $y_1' = g(t, y_0, y_1)$.

- `euler(f, t0, y0, t1, steps)` returns the state at time `t1`, advancing from state `y0` at time
  `t0` in `steps` equal steps of the forward Euler method, where `steps` must be positive. The
  derivative `f(t, y)` returns a vector of the same length as `y`.
- `midpoint(f, t0, y0, t1, steps)` and `runge_kutta(f, t0, y0, t1, steps)` do the same using the
  midpoint method and classical fourth-order Runge-Kutta.

A `t1` below `t0` integrates backwards, since the step is then negative. None of these adapt their
step, so doubling `steps` should shrink the change in the answer by roughly $2^p$ for a method of
order $p$; when it does not, the problem is stiff and needs an implicit method.

Time Complexity:
- O(s) calls to `f()` per call to `euler()`, O(2s) per call to `midpoint()`, and O(4s) per call to
  `runge_kutta()`, where $s$ is `steps`.

Space Complexity:
- O(n) auxiliary and O(n) for the returned state, where $n$ is the number of equations.

*/

#include <cassert>
#include <cstddef>
#include <vector>

using State = std::vector<double>;

template<typename Fn>
State euler(Fn f, double t0, const State &y0, double t1, int steps) {
  assert(steps > 0);
  double h = (t1 - t0) / steps;
  State y = y0;
  for (int i = 0; i < steps; i++) {
    State k = f(t0 + i * h, y);
    assert(k.size() == y.size());
    for (size_t j = 0; j < y.size(); j++) {
      y[j] += h * k[j];
    }
  }
  return y;
}

template<typename Fn>
State midpoint(Fn f, double t0, const State &y0, double t1, int steps) {
  assert(steps > 0);
  double h = (t1 - t0) / steps;
  State y = y0;
  for (int i = 0; i < steps; i++) {
    double t = t0 + i * h;
    State k1 = f(t, y), half = y;
    assert(k1.size() == y.size());
    for (size_t j = 0; j < y.size(); j++) {
      half[j] += h / 2 * k1[j];
    }
    State k2 = f(t + h / 2, half);
    assert(k2.size() == y.size());
    for (size_t j = 0; j < y.size(); j++) {
      y[j] += h * k2[j];
    }
  }
  return y;
}

template<typename Fn>
State runge_kutta(Fn f, double t0, const State &y0, double t1, int steps) {
  assert(steps > 0);
  double h = (t1 - t0) / steps;
  State y = y0, probe = y0;
  for (int i = 0; i < steps; i++) {
    double t = t0 + i * h;
    State k1 = f(t, y);
    assert(k1.size() == y.size());
    for (size_t j = 0; j < y.size(); j++) {
      probe[j] = y[j] + h / 2 * k1[j];
    }
    State k2 = f(t + h / 2, probe);
    assert(k2.size() == y.size());
    for (size_t j = 0; j < y.size(); j++) {
      probe[j] = y[j] + h / 2 * k2[j];
    }
    State k3 = f(t + h / 2, probe);
    assert(k3.size() == y.size());
    for (size_t j = 0; j < y.size(); j++) {
      probe[j] = y[j] + h * k3[j];
    }
    State k4 = f(t + h, probe);
    assert(k4.size() == y.size());
    for (size_t j = 0; j < y.size(); j++) {
      y[j] += h / 6 * (k1[j] + 2 * k2[j] + 2 * k3[j] + k4[j]);
    }
  }
  return y;
}

/*** Example Usage ***/

#include <cmath>
using namespace std;

const double PI = acos(-1.0), E = exp(1.0);

int main() {
  // y' = y, so y(1) = e starting from y(0) = 1.
  auto exponential = [](double, const State &y) { return State{y[0]}; };
  assert(fabs(euler(exponential, 0, State{1}, 1, 1000)[0] - E) < 2e-3);
  assert(fabs(midpoint(exponential, 0, State{1}, 1, 1000)[0] - E) < 1e-6);
  assert(fabs(runge_kutta(exponential, 0, State{1}, 1, 1000)[0] - E) < 1e-12);

  // Quartering the step size should cut the fourth-order error by about 256.
  double coarse = fabs(runge_kutta(exponential, 0, State{1}, 1, 10)[0] - E);
  double fine = fabs(runge_kutta(exponential, 0, State{1}, 1, 40)[0] - E);
  assert(coarse / fine > 100);

  // The harmonic oscillator y'' = -y as the system y0' = y1, y1' = -y0, whose orbit is a circle.
  auto oscillator = [](double, const State &y) { return State{y[1], -y[0]}; };
  State full_turn = runge_kutta(oscillator, 0, State{1, 0}, 2 * PI, 2000);
  assert(fabs(full_turn[0] - 1) < 1e-9 && fabs(full_turn[1]) < 1e-9);

  // Integrating backwards returns to the starting state.
  State there = runge_kutta(oscillator, 0, State{1, 0}, 3.0, 500);
  State back = runge_kutta(oscillator, 3.0, there, 0, 500);
  assert(fabs(back[0] - 1) < 1e-9 && fabs(back[1]) < 1e-9);
  return 0;
}
