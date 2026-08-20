/*

The following routines fit a model to observed data and use it to describe or predict unobserved
data. Each takes its input as a `Points` matrix whose rows are samples and whose columns are
features, so a row is one observation of the same measurements taken in the same order. Supervised
routines additionally take a `labels` vector, parallel to the rows, holding each sample's class as
an integer in $[0, `classes`)$.

Features measured on different scales distort every distance here, so call `standardize()` before
`kmeans()`, `knn_classify()`, or `logistic_regression()` and apply the returned pair to later
queries. Naive Bayes is exempt, since it fits a separate variance per feature. None of these choose
`k` or `l2` for you: hold out part of the data and keep the value predicting it best.

*/

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

using Points = std::vector<std::vector<double>>;

double sqdist(const std::vector<double> &a, const std::vector<double> &b) {
  assert(a.size() == b.size());
  double res = 0;
  for (size_t i = 0; i < a.size(); i++) {
    res += (a[i] - b[i]) * (a[i] - b[i]);
  }
  return res;
}

/*

Standardizing subtracts each column's mean and divides by its standard deviation, which is what
makes one Euclidean distance meaningful across features recorded in different units. The population
standard deviation is used, dividing by $n$ rather than $n - 1$, since these columns are the data
being fit and not a sample standing in for a larger population. A column that never varies would
give a zero divisor, so it reports $1$ instead and standardizes to all zeros, contributing nothing
to any distance, which is exactly what a constant feature deserves.

- `column_mean(points)` and `column_stddev(points, mean)` return the per-column mean and population
  standard deviation, the latter reporting $1$ for a column that never varies.
- `standardize(points)` rescales every column in place to zero mean and unit variance and returns
  the (`mean`, `stddev`) it used, while `standardize(x, mean, stddev)` applies a returned pair to
  one later sample.

Time Complexity:
- O(n*d) per call over $n$ rows of $d$ columns, and O(d) for one sample.

Space Complexity:
- O(d) auxiliary.

*/

std::vector<double> column_mean(const Points &points) {
  assert(!points.empty());
  int n = static_cast<int>(points.size()), d = static_cast<int>(points[0].size());
  std::vector<double> res(d);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < d; j++) {
      res[j] += points[i][j];
    }
  }
  for (int j = 0; j < d; j++) {
    res[j] /= n;
  }
  return res;
}

std::vector<double> column_stddev(const Points &points, const std::vector<double> &mean) {
  assert(!points.empty() && mean.size() == points[0].size());
  int n = static_cast<int>(points.size()), d = static_cast<int>(points[0].size());
  std::vector<double> res(d);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < d; j++) {
      res[j] += (points[i][j] - mean[j]) * (points[i][j] - mean[j]);
    }
  }
  for (int j = 0; j < d; j++) {
    res[j] = std::sqrt(res[j] / n);
    if (res[j] < 1e-12) {
      res[j] = 1;
    }
  }
  return res;
}

void standardize(
    std::vector<double> &x, const std::vector<double> &mean, const std::vector<double> &stddev
) {
  assert(x.size() == mean.size() && x.size() == stddev.size());
  assert(std::all_of(stddev.begin(), stddev.end(), [](double value) { return value > 0; }));
  for (size_t j = 0; j < x.size(); j++) {
    x[j] = (x[j] - mean[j]) / stddev[j];
  }
}

std::pair<std::vector<double>, std::vector<double>> standardize(Points &points) {
  std::vector<double> mean = column_mean(points), stddev = column_stddev(points, mean);
  for (std::vector<double> &row : points) {
    standardize(row, mean, stddev);
  }
  return std::make_pair(mean, stddev);
}

/*

$k$-means partitions the rows into $k$ clusters so that each row lies nearer to its own cluster's
mean than to any other. Lloyd's algorithm alternates the two halves of that condition: assign every
row to the nearest centroid, then move every centroid to the mean of the rows assigned to it. Each
half can only lower the total squared distance, so the process converges, but only to a local
optimum that the starting centroids decide.

Choosing those starting centroids is called seeding, and $k$-means++ is the standard rule: take the
first uniformly at random, then draw each remaining one with probability proportional to its squared
distance from the nearest already chosen. That spreads them out instead of letting several land in
one dense region, bounding the expected cost at $O(\log k)$ times optimal, which uniform choice
cannot promise. The loop stops once an assignment pass changes nothing, and a cluster that loses all
its rows keeps its previous centroid.

- `kmeans(points, k, rng, iterations = 100)` returns (`centroids`, `assignment`), partitioning the
  rows into `k` clusters, where `assignment[i]` is the cluster of row `i` and `centroids[c]` is the
  mean of cluster `c`. The number of rows must be at least `k`; both `k` and `iterations` must be
  positive.

Time Complexity:
- O(i*n*k*d) per call for `iterations` $i$, $n$ rows, and $d$ columns.

Space Complexity:
- O(n + k*d) auxiliary.

*/

std::pair<Points, std::vector<int>> kmeans(
    const Points &points, int k, std::mt19937 &rng, int iterations = 100
) {
  int n = static_cast<int>(points.size());
  assert(k > 0 && k <= n && iterations > 0);
  Points centroids;
  centroids.push_back(points[std::uniform_int_distribution<int>(0, n - 1)(rng)]);
  std::vector<double> best(n);
  for (int i = 0; i < n; i++) {
    best[i] = sqdist(points[i], centroids[0]);
  }
  while (static_cast<int>(centroids.size()) < k) {
    std::discrete_distribution<int> pick(best.begin(), best.end());
    centroids.push_back(points[pick(rng)]);
    for (int i = 0; i < n; i++) {
      best[i] = std::min(best[i], sqdist(points[i], centroids.back()));
    }
  }
  std::vector<int> assignment(n, -1);
  for (int it = 0; it < iterations; it++) {
    bool changed = false;
    for (int i = 0; i < n; i++) {
      int nearest = 0;
      double nearest_dist = sqdist(points[i], centroids[0]);
      for (int c = 1; c < k; c++) {
        double dist = sqdist(points[i], centroids[c]);
        if (dist < nearest_dist) {
          nearest = c;
          nearest_dist = dist;
        }
      }
      if (assignment[i] != nearest) {
        assignment[i] = nearest;
        changed = true;
      }
    }
    if (!changed) {
      break;
    }
    Points sums(k, std::vector<double>(points[0].size()));
    std::vector<int> counts(k);
    for (int i = 0; i < n; i++) {
      for (size_t j = 0; j < points[i].size(); j++) {
        sums[assignment[i]][j] += points[i][j];
      }
      counts[assignment[i]]++;
    }
    for (int c = 0; c < k; c++) {
      if (counts[c] > 0) {
        for (size_t j = 0; j < sums[c].size(); j++) {
          centroids[c][j] = sums[c][j] / counts[c];
        }
      }
    }
  }
  return std::make_pair(centroids, assignment);
}

/*

$k$-nearest neighbors classifies a query by letting the $k$ nearest training rows vote, which fits
no model at all: the training data is the model, and all the work happens at query time. Small $k$
follows the data closely and is swayed by noise, while large $k$ smooths across genuine boundaries,
so $k$ trades variance against bias. Averaging the neighbors' values rather than counting their
votes turns the same routine into regression.

Selecting the $k$ nearest with `std::nth_element` costs linear time rather than the O(n log n) of a
full sort, since the neighbors' relative order is irrelevant to the vote. Scanning every row makes
each query linear in the data, which the k-d tree of section 2.6.5 improves to logarithmic on
low-dimensional data; above roughly ten features the two converge and the scan below is preferable.

- `knn_classify(points, labels, query, k)` returns the majority label among the `k` rows nearest to
  `query`, breaking a tie toward the smaller label. `k` must lie in $[1, n]$ for `n` rows.

Time Complexity:
- O(n*d) per call, for $n$ rows of $d$ columns.

Space Complexity:
- O(n) auxiliary.

*/

int knn_classify(
    const Points &points, const std::vector<int> &labels, const std::vector<double> &query, int k
) {
  int n = static_cast<int>(points.size());
  assert(n > 0 && static_cast<int>(labels.size()) == n && query.size() == points[0].size());
  assert(k >= 1 && k <= n);
  assert(std::all_of(labels.begin(), labels.end(), [](int label) { return label >= 0; }));
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::vector<double> dist(n);
  for (int i = 0; i < n; i++) {
    dist[i] = sqdist(points[i], query);
  }
  std::nth_element(order.begin(), order.begin() + (k - 1), order.end(), [&](int a, int b) {
    return dist[a] < dist[b];
  });
  std::vector<int> votes(*std::max_element(labels.begin(), labels.end()) + 1);
  for (int i = 0; i < k; i++) {
    votes[labels[order[i]]]++;
  }
  return static_cast<int>(std::max_element(votes.begin(), votes.end()) - votes.begin());
}

/*

Naive Bayes picks the class maximizing $P(c) \prod_j P(x_j \mid c)$, which is Bayes' rule with the
denominator dropped because it does not depend on the class. The independence it assumes among
features given the class is almost never true, and yet the classifier is accurate anyway: the
products it computes are poorly calibrated as probabilities, but the class that maximizes them is
usually still the right one, since the errors tend to move every class the same way.

The Gaussian variant below models each feature within each class by a normal distribution, so
fitting is one pass accumulating a mean and variance per (class, feature) pair. Scores accumulate as
logarithms, both because a product of hundreds of densities underflows and because the logarithm of
a normal density is a plain quadratic. A feature that is constant within a class would give a zero
variance and an infinite score, so variances are floored at a small fraction of the overall spread.

- `NaiveBayes(points, labels, classes)` fits a Gaussian naive Bayes classifier, `predict(x)` returns
  its most probable class for a sample `x`, and `log_likelihood(x, c)` returns the unnormalized log
  probability that `x` belongs to class `c`. A class carried by no row is given zero prior, so it
  scores $-\infty$ and is never predicted.

Time Complexity:
- O(n*d) per construction.
- O(c*d) per call to `predict()` and O(d) per call to `log_likelihood()`, for $n$ rows of $d$
  columns and $c$ classes.

Space Complexity:
- O(c*d) for storage.

*/

class NaiveBayes {
  std::vector<double> log_prior;
  Points mean, var;

 public:
  NaiveBayes(const Points &points, const std::vector<int> &labels, int classes) {
    assert(!points.empty() && labels.size() == points.size() && classes > 0);
    assert(std::all_of(labels.begin(), labels.end(), [&](int label) {
      return 0 <= label && label < classes;
    }));
    int n = static_cast<int>(points.size()), d = static_cast<int>(points[0].size());
    std::vector<int> counts(classes);
    mean.assign(classes, std::vector<double>(d));
    var.assign(classes, std::vector<double>(d));
    log_prior.assign(classes, 0);
    for (int i = 0; i < n; i++) {
      counts[labels[i]]++;
      for (int j = 0; j < d; j++) {
        mean[labels[i]][j] += points[i][j];
      }
    }
    for (int c = 0; c < classes; c++) {
      if (counts[c] == 0) {  // A class with no rows has zero prior, so it is never predicted.
        log_prior[c] = -std::numeric_limits<double>::infinity();
        continue;
      }
      log_prior[c] = std::log(static_cast<double>(counts[c]) / n);
      for (int j = 0; j < d; j++) {
        mean[c][j] /= counts[c];
      }
    }
    double spread = 0;
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < d; j++) {
        double diff = points[i][j] - mean[labels[i]][j];
        var[labels[i]][j] += diff * diff;
        spread += diff * diff;
      }
    }
    double floor_var = 1e-9 * spread / n + 1e-12;
    for (int c = 0; c < classes; c++) {
      for (int j = 0; j < d; j++) {
        var[c][j] = counts[c] == 0 ? floor_var : std::max(var[c][j] / counts[c], floor_var);
      }
    }
  }

  double log_likelihood(const std::vector<double> &x, int c) const {
    assert(c >= 0 && c < static_cast<int>(log_prior.size()) && x.size() == mean[c].size());
    static const double LOG_2PI = 1.8378770664093453;
    double res = log_prior[c];
    for (size_t j = 0; j < x.size(); j++) {
      double diff = x[j] - mean[c][j];
      res -= 0.5 * (LOG_2PI + std::log(var[c][j]) + diff * diff / var[c][j]);
    }
    return res;
  }

  int predict(const std::vector<double> &x) const {
    int best = 0;
    double best_score = log_likelihood(x, 0);
    for (size_t c = 1; c < log_prior.size(); c++) {
      double score = log_likelihood(x, static_cast<int>(c));
      if (score > best_score) {
        best = static_cast<int>(c);
        best_score = score;
      }
    }
    return best;
  }
};

/*

Logistic regression models the probability of class $1$ as $\sigma(w \cdot x)$ for the logistic
function $\sigma(z) = 1/(1 + e^{-z})$, which maps any real score into $(0, 1)$. Fitting minimizes
the average negative log-likelihood, a convex function whose gradient is
$X^{\top}(\sigma(Xw) - y)/n$: each row pushes the weights by its own features, scaled by how wrong
the current prediction is on it. Convexity makes every local minimum global, so plain gradient
descent suffices.

Unlike the least squares of section 6.5.5 there is no closed form, and on perfectly separable data
the weights diverge as every predicted probability is driven to $0$ or $1$. The `l2` penalty adds
$\lambda\|w\|^2$ to keep them finite, excluding the intercept so that shifting all labels does not
distort it. Section 5.2.4 supplies the adaptive steps worth substituting when convergence is slow.

- `logistic_predict(w, x)` returns the probability that sample `x` belongs to class $1$ under a
  weight vector `w` whose index $0$ is the intercept.
- `logistic_regression(points, labels, ...)` returns such a weight vector, fit by gradient descent
  over labels that are $0$ or $1$. Optional parameters default to `rate = 0.1`, `iterations = 1000`,
  and `l2 = 0`.

Time Complexity:
- O(i*n*d) per call for `iterations` $i$, $n$ rows, and $d$ columns.

Space Complexity:
- O(d) auxiliary.

*/

double sigmoid(double z) {
  return z >= 0 ? 1 / (1 + std::exp(-z)) : std::exp(z) / (1 + std::exp(z));
}

double logistic_predict(const std::vector<double> &w, const std::vector<double> &x) {
  assert(w.size() == x.size() + 1);
  double z = w[0];
  for (size_t j = 0; j < x.size(); j++) {
    z += w[j + 1] * x[j];
  }
  return sigmoid(z);
}

std::vector<double> logistic_regression(
    const Points &points, const std::vector<int> &labels, double rate = 0.1, int iterations = 1000,
    double l2 = 0
) {
  assert(!points.empty() && labels.size() == points.size());
  assert(rate > 0 && iterations > 0 && l2 >= 0);
  assert(std::all_of(labels.begin(), labels.end(), [](int label) {
    return label == 0 || label == 1;
  }));
  int n = static_cast<int>(points.size()), d = static_cast<int>(points[0].size());
  std::vector<double> w(d + 1), grad(d + 1);
  for (int it = 0; it < iterations; it++) {
    std::fill(grad.begin(), grad.end(), 0.0);
    for (int i = 0; i < n; i++) {
      double error = logistic_predict(w, points[i]) - labels[i];
      grad[0] += error;
      for (int j = 0; j < d; j++) {
        grad[j + 1] += error * points[i][j];
      }
    }
    w[0] -= rate * grad[0] / n;
    for (int j = 1; j <= d; j++) {
      w[j] -= rate * (grad[j] / n + l2 * w[j]);
    }
  }
  return w;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Two well-separated blobs: the first three rows lie near (0, 0) and the last three near
  // (10, 10).
  Points points{{0, 0}, {1, 0}, {0, 1}, {10, 10}, {11, 10}, {10, 11}};
  vector<int> labels{0, 0, 0, 1, 1, 1};

  // Standardizing puts both columns on one scale; the same pair transforms later queries.
  Points scaled(points);
  pair<vector<double>, vector<double>> shift = standardize(scaled);
  vector<double> query{9, 9};
  standardize(query, shift.first, shift.second);
  assert(knn_classify(scaled, labels, query, 3) == 1);

  mt19937 rng(1234567);
  pair<Points, vector<int>> clustered = kmeans(points, 2, rng);
  assert(clustered.first.size() == 2);
  assert(clustered.second[0] == clustered.second[1] && clustered.second[1] == clustered.second[2]);
  assert(clustered.second[3] == clustered.second[4] && clustered.second[4] == clustered.second[5]);
  assert(clustered.second[0] != clustered.second[3]);

  assert(knn_classify(points, labels, {0.5, 0.5}, 3) == 0);
  assert(knn_classify(points, labels, {9, 9}, 3) == 1);

  NaiveBayes nb(points, labels, 2);
  assert(nb.predict({0.5, 0.5}) == 0);
  assert(nb.predict({9, 9}) == 1);
  // Declaring a third class that no row carries leaves it with zero prior, never predicted.
  NaiveBayes sparse(points, labels, 3);
  assert(isinf(sparse.log_likelihood({0.5, 0.5}, 2)));
  assert(sparse.predict({0.5, 0.5}) == 0 && sparse.predict({9, 9}) == 1);

  vector<double> w = logistic_regression(points, labels, 0.5, 2000);
  assert(logistic_predict(w, {0.5, 0.5}) < 0.5);
  assert(logistic_predict(w, {9, 9}) > 0.5);
  return 0;
}
