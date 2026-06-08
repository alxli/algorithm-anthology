/*

Given an array and a fixed window length `k`, report the maximum (or minimum) of every contiguous
window of that length as the window slides from left to right. A monotonic deque of indices answers
all windows in linear total time: it holds the indices of candidates that could still become a
window maximum, kept in decreasing order of value. Each new element pops every smaller element off
the back, since those can never again be the maximum while the larger newer element is in the window,
and indices that fall outside the window are dropped from the front. The front of the deque is always
the maximum of the current window. This is the windowed counterpart of the monotonic stack: each
index is pushed and popped at most once.

Both functions take a vector `a` of `n` comparable values and a window length `k` with
$1 \leq k \leq n$, and return a vector of `n - k + 1` results, one per window in left-to-right order.

- `sliding_window_maximum(a, k)` returns the maximum of each window of length `k`.
- `sliding_window_minimum(a, k)` returns the minimum of each window of length `k`.

Time Complexity:
- O(n) per call to both functions, where $n$ is the size of the input.

Space Complexity:
- O(n) auxiliary heap space for the result, plus O(k) for the deque.

*/

#include <deque>
#include <vector>

template<class T>
std::vector<T> sliding_window_maximum(const std::vector<T> &a, int k) {
  int n = a.size();
  std::deque<int> window;
  std::vector<T> res;
  for (int i = 0; i < n; i++) {
    while (!window.empty() && a[window.back()] <= a[i]) {
      window.pop_back();
    }
    window.push_back(i);
    if (window.front() <= i - k) {
      window.pop_front();
    }
    if (i >= k - 1) {
      res.push_back(a[window.front()]);
    }
  }
  return res;
}

template<class T>
std::vector<T> sliding_window_minimum(const std::vector<T> &a, int k) {
  int n = a.size();
  std::deque<int> window;
  std::vector<T> res;
  for (int i = 0; i < n; i++) {
    while (!window.empty() && a[window.back()] >= a[i]) {
      window.pop_back();
    }
    window.push_back(i);
    if (window.front() <= i - k) {
      window.pop_front();
    }
    if (i >= k - 1) {
      res.push_back(a[window.front()]);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{1, 3, -1, -3, 5, 3, 6, 7};
  assert((sliding_window_maximum(a, 3) == vector<int>{3, 3, 5, 5, 6, 7}));
  assert((sliding_window_minimum(a, 3) == vector<int>{-1, -3, -3, -3, 3, 3}));
  return 0;
}
