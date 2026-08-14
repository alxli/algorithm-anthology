/*

Small timing helpers for local profiling and codebook examples. Use `std::chrono::steady_clock`
instead of wall-clock time so elapsed durations are monotonic.

- `Timer()` starts a timer immediately.
- `timer.reset()` restarts the timer.
- `timer.elapsed()` returns the elapsed time in milliseconds as a double.
- `benchmark(iterations, f)` runs `f()` `iterations` times and returns the average elapsed
  milliseconds per call. `iterations` must be positive.

Benchmarks should be treated as local measurements, not proofs of asymptotic performance. Warm-up,
CPU frequency scaling, compiler optimization, and input choice can dominate tiny timings.

*/

#include <cassert>
#include <chrono>

class Timer {
  using Clock = std::chrono::steady_clock;
  Clock::time_point start_time;

 public:
  Timer() { reset(); }

  void reset() { start_time = Clock::now(); }

  double elapsed() const {
    return std::chrono::duration<double, std::milli>(Clock::now() - start_time).count();
  }
};

template<typename Fn>
double benchmark(int iterations, Fn f) {
  assert(iterations > 0);
  Timer timer;
  for (int i = 0; i < iterations; ++i) {
    f();
  }
  return timer.elapsed() / iterations;
}

/*** Example Usage ***/

#include <cassert>
#include <numeric>
#include <vector>
using namespace std;

int main() {
  Timer timer;
  vector<int> v(1000);
  iota(v.begin(), v.end(), 0);
  assert(timer.elapsed() >= 0.0);

  volatile long long sink = 0;
  double avg_time = benchmark(5, [&] { sink += accumulate(v.begin(), v.end(), 0LL); });
  assert(avg_time >= 0.0);
  assert(sink > 0);
  return 0;
}
