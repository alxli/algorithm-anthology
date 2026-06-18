/*

Basic multithreading can speed up independent local work such as stress tests, batch experiments, or
offline preprocessing. It is rarely useful in standard contest submissions, and many judges restrict
threads, so use it only when the environment allows it.

- `parallel_for(n, threads, f)` runs `f(i)` once for each `0 <= i < n`, splitting the indices among
  at most `threads` worker threads.
- `parallel_cases(cases, threads, solve)` solves independent test cases in parallel and returns the
  outputs in the original case order.

Each call to `f(i)` must be safe to run concurrently with the others. If it writes shared state, use
separate output slots or synchronization.

*/

#include <algorithm>
#include <cassert>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

template<typename Fun>
void parallel_for(int n, int threads, Fun f) {
  threads = std::max(1, std::min(threads, n));
  std::vector<std::thread> workers;
  workers.reserve(threads);
  for (int id = 0; id < threads; ++id) {
    workers.emplace_back([=, &f] {
      for (int i = id; i < n; i += threads) {
        f(i);
      }
    });
  }
  for (auto &worker : workers) {
    worker.join();
  }
}

template<typename Case, typename Solve>
std::vector<std::string> parallel_cases(const std::vector<Case> &cases, int threads, Solve solve) {
  std::vector<std::string> out(cases.size());
  parallel_for(static_cast<int>(cases.size()), threads, [&](int i) {
    std::ostringstream os;
    solve(cases[i], os);
    out[i] = os.str();
  });
  return out;
}

/*** Example Usage ***/

int main() {
  std::vector<long long> squares(1000);
  parallel_for(static_cast<int>(squares.size()), 4, [&](int i) { squares[i] = 1LL * i * i; });
  assert(squares[17] == 289);
  assert(std::accumulate(squares.begin(), squares.end(), 0LL) == 332833500);

  std::vector<int> cases = {3, 1, 4, 2};
  auto out =
      parallel_cases(cases, 2, [](int x, std::ostringstream &os) { os << x << "^2 = " << x * x; });
  assert((out == std::vector<std::string>{"3^2 = 9", "1^2 = 1", "4^2 = 16", "2^2 = 4"}));
  return 0;
}
