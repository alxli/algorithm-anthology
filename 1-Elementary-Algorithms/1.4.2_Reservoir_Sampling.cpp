/*

Selects uniformly random samples from a stream whose length may be unknown in advance. Reservoir
sampling keeps only the chosen sample or samples in memory while processing each element once.

Each class maintains its reservoir incrementally; call `add(x)` once per stream element in any
order, then `get()` to retrieve the result.

- `ReservoirSampleOne<T>()` constructs a single-element sampler.
- `ReservoirSampleK<T>(k)` constructs a k-element sampler.
- `add(x)` incorporates one more stream element.
- `get()` returns the current sample. For `ReservoirSampleOne`, `get()` requires at least one
  `add()` call; for `ReservoirSampleK`, returns the full reservoir (may be fewer than `k` elements
  if the stream was shorter).
- `count()` returns the number of stream elements seen so far.

Time Complexity:
- O(1) per call to `add(x)`.
- O(n) to process a range of `n` elements.

Space Complexity:
- O(1) auxiliary for `ReservoirSampleOne`.
- O(k) auxiliary heap space for `ReservoirSampleK`.

*/

#include <cassert>
#include <random>
#include <vector>

int rand_int(int lo, int hi) {
  static std::mt19937 rng(std::random_device{}());
  return std::uniform_int_distribution<int>(lo, hi)(rng);
}

template<class T>
class ReservoirSampleOne {
  T sample{};
  int seen;

 public:
  ReservoirSampleOne() : seen(0) {}

  void add(const T &x) {
    if (rand_int(0, seen++) == 0) {
      sample = x;
    }
  }

  const T &get() const {
    assert(seen > 0);
    return sample;
  }

  int count() const { return seen; }
};

template<class T>
class ReservoirSampleK {
  int k, seen;
  std::vector<T> reservoir;

 public:
  explicit ReservoirSampleK(int k) : k(k), seen(0) {}

  void add(const T &x) {
    ++seen;
    if (static_cast<int>(reservoir.size()) < k) {
      reservoir.push_back(x);
    } else {
      int j = rand_int(0, seen - 1);
      if (j < k) {
        reservoir[j] = x;
      }
    }
  }

  const std::vector<T> &get() const { return reservoir; }
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
  int one = s1.get();
  assert(one == 10 || one == 20 || one == 30 || one == 40 || one == 50);

  ReservoirSampleK<int> sk(3);
  for (int x : a) {
    sk.add(x);
  }
  assert(static_cast<int>(sk.get().size()) == 3);
  return 0;
}
