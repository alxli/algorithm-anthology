/*

Given a differentiable objective $f$ over $\mathbb{R}^n$ and a starting guess, approximately
minimize the function by repeatedly stepping against the gradient. The gradient points in the
direction of steepest increase, so moving the opposite way lowers $f$ for a small enough step; the
whole difficulty is choosing how far to move, since too large a step overshoots and diverges while
too small a step never arrives.

Backtracking answers this by trying a full step and halving it until the Armijo condition
$f(x - t\nabla f) \leq f(x) - c\,t\,\|\nabla f\|^2$ holds, which accepts a step only when it
delivers a fixed fraction of the decrease that the gradient predicts. It needs no tuning and is the
default to reach for. Adam instead keeps exponentially decaying averages of the gradient and of its
squared entries, then divides one by the square root of the other, so each coordinate gets a step
scaled to its own recent gradient magnitude. That makes it far less sensitive to badly scaled
variables and to noisy gradients, at the cost of a learning rate to choose.

- `numeric_gradient(f, x, h = 1e-6)` returns the gradient of `f` at `x` estimated by central
  differences, using a step scaled by the magnitude of each coordinate.
- `gradient_descent_min(f, grad, x0, ...)` returns (`value`, `point`) for an approximate minimum of
  `f` reached from the initial guess `x0`, taking backtracking steps against `grad(x)`. Optional
  parameters default to `eps = 1e-9`, `iterations = 10000`, and `step_max = 1`, where `eps` is the
  gradient norm below which the search stops and `step_max` is the first step length tried.
- `adam_min(f, grad, x0, ...)` returns (`value`, `point`) for an approximate minimum of `f` reached
  from `x0` using Adam. Optional parameters default to `rate = 0.01`, `iterations = 10000`,
  `eps = 1e-9`, `beta1 = 0.9`, and `beta2 = 0.999`, where `rate` is the learning rate and the two
  decay factors, both required to lie in $[0, 1)$, control the two running averages.

The gradient callable must return a vector with the same length as its input point.

Both routines find a local minimum, and only a convex $f$ makes that the global one, as with the
logistic regression of section 6.7.5. Prefer the golden-section search of section 5.2.1 in one
dimension, or the hill climbing of section 5.2.2 and annealing of section 5.2.3 when no gradient is
available, since a numeric gradient costs $2n$ evaluations per step and half the usable precision.

Time Complexity:
- O(n) calls to `f()` per call to `numeric_gradient()`, where $n$ is the number of variables.
- O(k(C_g + l*C_f)) per call to `gradient_descent_min()`, where $k$ is the number of iterations
  performed, $l$ is the number of trial steps per line search, and $C_f$ and $C_g$ are the costs of
  evaluating `f` and `grad`.
- O(k(C_g + n)) per call to `adam_min()`, where $k$ is the number of iterations performed.

Space Complexity:
- O(n) auxiliary for all operations.

*/

#include <cassert>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

template<typename Fn>
std::vector<double> numeric_gradient(Fn f, const std::vector<double> &x, double h = 1e-6) {
  std::vector<double> res(x.size()), p(x);
  for (size_t i = 0; i < x.size(); i++) {
    double step = h * (1 + std::fabs(x[i]));
    p[i] = x[i] + step;
    double hi = f(p);
    p[i] = x[i] - step;
    double lo = f(p);
    p[i] = x[i];
    res[i] = (hi - lo) / (2 * step);
  }
  return res;
}

template<typename Fn, typename Grad>
std::pair<double, std::vector<double>> gradient_descent_min(
    Fn f, Grad grad, std::vector<double> x, double eps = 1e-9, int iterations = 10000,
    double step_max = 1
) {
  static const double ARMIJO = 1e-4, STEP_MIN = 1e-18;
  double res = f(x);
  std::vector<double> next(x.size());
  for (int it = 0; it < iterations; it++) {
    std::vector<double> g = grad(x);
    assert(g.size() == x.size());
    double norm2 = 0;
    for (double v : g) {
      norm2 += v * v;
    }
    if (std::sqrt(norm2) < eps) {
      break;
    }
    bool moved = false;
    for (double step = step_max; step > STEP_MIN; step /= 2) {
      for (size_t i = 0; i < x.size(); i++) {
        next[i] = x[i] - step * g[i];
      }
      double value = f(next);
      if (value <= res - ARMIJO * step * norm2) {
        x = next;
        res = value;
        moved = true;
        break;
      }
    }
    if (!moved) {
      break;
    }
  }
  return std::make_pair(res, x);
}

template<typename Fn, typename Grad>
std::pair<double, std::vector<double>> adam_min(
    Fn f, Grad grad, std::vector<double> x, double rate = 0.01, int iterations = 10000,
    double eps = 1e-9, double beta1 = 0.9, double beta2 = 0.999
) {
  static const double EPS = 1e-8;
  assert(0 <= beta1 && beta1 < 1 && 0 <= beta2 && beta2 < 1);
  std::vector<double> m(x.size()), v(x.size());
  for (int t = 1; t <= iterations; t++) {
    std::vector<double> g = grad(x);
    assert(g.size() == x.size());
    double norm2 = 0;
    for (double d : g) {
      norm2 += d * d;
    }
    if (std::sqrt(norm2) < eps) {
      break;
    }
    double c1 = 1 - std::pow(beta1, t), c2 = 1 - std::pow(beta2, t);
    for (size_t i = 0; i < x.size(); i++) {
      m[i] = beta1 * m[i] + (1 - beta1) * g[i];
      v[i] = beta2 * v[i] + (1 - beta2) * g[i] * g[i];
      x[i] -= rate * (m[i] / c1) / (std::sqrt(v[i] / c2) + EPS);
    }
  }
  return std::make_pair(f(x), x);
}

/*** Example Usage ***/

using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-6;
}

// Ill-conditioned paraboloid with global minimum at f(2, 3) = 0.
double f(const vector<double> &x) {
  return 100 * (x[0] - 2) * (x[0] - 2) + (x[1] - 3) * (x[1] - 3);
}

vector<double> grad(const vector<double> &x) {
  return {200 * (x[0] - 2), 2 * (x[1] - 3)};
}

int main() {
  vector<double> x0{0, 0};
  vector<double> g = numeric_gradient(f, x0);
  assert(EQ(g[0], -400) && EQ(g[1], -6));

  pair<double, vector<double>> res = gradient_descent_min(f, grad, x0);
  assert(EQ(res.first, 0) && EQ(res.second[0], 2) && EQ(res.second[1], 3));

  res = adam_min(f, grad, x0, 0.05, 100000);
  assert(EQ(res.first, 0) && EQ(res.second[0], 2) && EQ(res.second[1], 3));

  // A gradient is not required: central differences suffice, at 2n evaluations per step.
  res = gradient_descent_min(f, [](const vector<double> &x) { return numeric_gradient(f, x); }, x0);
  assert(EQ(res.first, 0));
  return 0;
}
