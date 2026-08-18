/*

Selects uniformly random samples from a stream whose length may be unknown in advance. This is
useful when values arrive online or when storing the full dataset is unnecessary.

Reservoir sampling keeps only the chosen sample in memory while processing each element once. The
first $k$ arrivals fill the reservoir, and the $i$-th element thereafter is accepted with
probability $k/i$, evicting a uniformly chosen occupant. One draw performs both choices: an integer
uniform on $[0, i)$ is below $k$ with probability $k/i$, and is then uniform on the $k$ occupants.

The invariant is that after $n \geq k$ arrivals the reservoir is a uniformly random $k$-subset, so
each of the $\binom{n}{k}$ possibilities is equally likely. This is certain at $n = k$. Assuming it
after $n - 1$, fix a $k$-subset $A$ of the first $n$. If $n \notin A$, the reservoir must already
hold $A$ and reject the arrival, with probability $\binom{n-1}{k}^{-1}(n-k)/n$. If $n \in A$, it
must hold $A$ with $n$ replaced by one of the $n - k$ outsiders, each then evicted with probability
$(k/n)(1/k) = 1/n$, giving $\binom{n-1}{k}^{-1}(n-k)/n$ again. Both equal $\binom{n}{k}^{-1}$, since
$\binom{n}{k} = \binom{n-1}{k} \cdot n/(n-k)$. Taking marginals, every element seen is in the sample
with probability $k/n$, and `ReservoirSampleOne` is the case $k = 1$.

Each class maintains its reservoir incrementally; call `add(x)` once per stream element in any
order, then call `sample()` to retrieve the result.

- `ReservoirSampleOne<T>()` constructs a single-element sampler.
- `ReservoirSampleK<T>(k)` constructs a `k`-element sampler, allocating the reservoir up front so
  that no later `add()` reallocates.
- `add(x)` incorporates one more stream element.
- `sample()` returns the current sample. For `ReservoirSampleOne`, `sample()` requires at least one
  `add()` call; for `ReservoirSampleK`, it returns the full reservoir, which may contain fewer than
  `k` elements if the stream was shorter.
- `count()` returns the number of stream elements seen so far.

Time Complexity:
- O(1) per call to `ReservoirSampleOne::add()` and `ReservoirSampleK::add()`.
- O(k) per call to the `ReservoirSampleK` constructor.
- O(n) to process a range of $n$ elements.

Space Complexity:
- O(1) storage for `ReservoirSampleOne`.
- O(k) storage for `ReservoirSampleK`, allocated on construction.

*/

#include <cassert>
#include <chrono>
#include <optional>
#include <random>
#include <vector>

int rand_int(int lo, int hi) {
  static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  return std::uniform_int_distribution<int>(lo, hi)(rng);
}

template<typename T>
class ReservoirSampleOne {
  std::optional<T> value;
  int seen = 0;

 public:
  void add(const T &x) {
    seen++;
    if (rand_int(1, seen) == 1) {
      value = x;
    }
  }

  const T &sample() const {
    assert(value.has_value());
    return *value;
  }

  int count() const { return seen; }
};

template<typename T>
class ReservoirSampleK {
  int k, seen = 0;
  std::vector<T> reservoir;

 public:
  explicit ReservoirSampleK(int k) : k(k) {
    assert(k >= 0);
    reservoir.reserve(k);
  }

  void add(const T &x) {
    ++seen;
    if (k == 0) {
      return;
    }
    if (static_cast<int>(reservoir.size()) < k) {
      reservoir.push_back(x);
    } else {
      int j = rand_int(0, seen - 1);
      if (j < k) {
        reservoir[j] = x;
      }
    }
  }

  const std::vector<T> &sample() const { return reservoir; }
  int count() const { return seen; }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{10, 20, 30, 40, 50};

  // Streaming interface: feed elements one at a time.
  ReservoirSampleOne<int> s1;
  for (int x : a) {
    s1.add(x);
  }
  assert(s1.count() == 5);
  int one = s1.sample();
  assert(one == 10 || one == 20 || one == 30 || one == 40 || one == 50);

  ReservoirSampleK<int> sk(3);
  for (int x : a) {
    sk.add(x);
  }
  assert(sk.count() == 5);
  assert(sk.sample().size() == 3);

  ReservoirSampleK<int> large(10);
  for (int x : a) {
    large.add(x);
  }
  assert(large.count() == 5);
  assert(large.sample().size() == 5);
  return 0;
}
