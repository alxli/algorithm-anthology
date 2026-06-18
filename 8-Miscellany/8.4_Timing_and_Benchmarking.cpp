/*

Small timing helpers for local profiling and codebook examples. Use `std::chrono::steady_clock`
instead of wall-clock time so elapsed durations are monotonic.

- `Timer()` starts a timer immediately.
- `timer.reset()` restarts the timer.
- `timer.elapsed()` returns the elapsed time in milliseconds as a double.
- `benchmark(name, iterations, f)` runs `f()` `iterations` times and returns the average elapsed
  milliseconds per call with the given name.

Benchmarks should be treated as local measurements, not proofs of asymptotic performance. Warm-up,
CPU frequency scaling, compiler optimization, and input choice can dominate tiny timings.

*/

#include <chrono>
#include <string>
#include <utility>
#include <vector>

class Timer {
  using Clock = std::chrono::steady_clock;
  Clock::time_point start_time;

 public:
  Timer() { reset(); }

  void reset() { start_time = Clock::now(); }

  double elapsed() const {
    std::chrono::duration<double, std::milli> diff = Clock::now() - start_time;
    return diff.count();
  }
};

template<typename Fun>
std::pair<std::string, double> benchmark(const std::string &name, int iterations, Fun f) {
  Timer timer;
  for (int i = 0; i < iterations; ++i) {
    f();
  }
  return {name, timer.elapsed() / iterations};
}

/*** Example Usage ***/

#include <cassert>
#include <numeric>
#include <vector>
using namespace std;

int main() {
  Timer timer;
  std::vector<int> v(1000);
  std::iota(v.begin(), v.end(), 0);
  assert(timer.elapsed() >= 0.0);

  volatile long long sink = 0;
  auto res = benchmark("sum", 5, [&] {
    long long cur = 0;
    for (int x : v) {
      cur += x;
    }
    sink += cur;
  });
  assert(res.first == "sum");
  assert(res.second >= 0.0);
  assert(sink > 0);
  return 0;
}
