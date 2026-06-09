/*

A monotonic stack maintains its elements in sorted order as values are pushed, popping away any
entries that would violate the ordering. Scanning an array once while keeping such a stack of
indices answers, for every position, where the nearest smaller or larger neighbor lies. These
"nearest smaller/greater" relationships underlie many array problems, the most famous being the
largest rectangle in a histogram. Each index is pushed and popped at most once, so a full scan runs
in linear time.

All functions below take a vector `a` of `n` comparable values and return a vector of `n` indices.
A "less" query uses a strictly smaller neighbor and a "greater" query uses a strictly larger one;
changing the comparison from strict to non-strict (e.g. `>=` to `>`) toggles how ties are handled.

- `previous_less(a)` returns, for each `i`, the largest index `j < i` with `a[j] < a[i]`, or `-1`.
- `next_less(a)` returns, for each `i`, the smallest index `j > i` with `a[j] < a[i]`, or `n`.
- `previous_greater(a)` returns, for each `i`, the largest index `j < i` with `a[j] > a[i]`, or `-1`.
- `next_greater(a)` returns, for each `i`, the smallest index `j > i` with `a[j] > a[i]`, or `n`.
- `largest_rectangle(heights)` returns the maximum area of an axis-aligned rectangle that fits under
  the given histogram, where bar `i` has width 1 and height `heights[i]`.

Time Complexity:
- O(n) per call to all functions, where $n$ is the size of the input.

Space Complexity:
- O(n) auxiliary heap space for the stack and result.

*/

#include <stack>
#include <vector>

template<class T>
std::vector<int> previous_less(const std::vector<T> &a) {
  int n = static_cast<int>(a.size());
  std::vector<int> res(n);
  std::stack<int> s;
  for (int i = 0; i < n; i++) {
    while (!s.empty() && a[s.top()] >= a[i]) {
      s.pop();
    }
    res[i] = s.empty() ? -1 : s.top();
    s.push(i);
  }
  return res;
}

template<class T>
std::vector<int> next_less(const std::vector<T> &a) {
  int n = static_cast<int>(a.size());
  std::vector<int> res(n);
  std::stack<int> s;
  for (int i = n - 1; i >= 0; i--) {
    while (!s.empty() && a[s.top()] >= a[i]) {
      s.pop();
    }
    res[i] = s.empty() ? n : s.top();
    s.push(i);
  }
  return res;
}

template<class T>
std::vector<int> previous_greater(const std::vector<T> &a) {
  int n = static_cast<int>(a.size());
  std::vector<int> res(n);
  std::stack<int> s;
  for (int i = 0; i < n; i++) {
    while (!s.empty() && a[s.top()] <= a[i]) {
      s.pop();
    }
    res[i] = s.empty() ? -1 : s.top();
    s.push(i);
  }
  return res;
}

template<class T>
std::vector<int> next_greater(const std::vector<T> &a) {
  int n = static_cast<int>(a.size());
  std::vector<int> res(n);
  std::stack<int> s;
  for (int i = n - 1; i >= 0; i--) {
    while (!s.empty() && a[s.top()] <= a[i]) {
      s.pop();
    }
    res[i] = s.empty() ? n : s.top();
    s.push(i);
  }
  return res;
}

template<class T>
T largest_rectangle(const std::vector<T> &heights) {
  int n = static_cast<int>(heights.size());
  std::vector<int> left = previous_less(heights), right = next_less(heights);
  T best = 0;
  for (int i = 0; i < n; i++) {
    T area = heights[i] * (right[i] - left[i] - 1);
    if (area > best) {
      best = area;
    }
  }
  return best;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{2, 1, 4, 3};
  assert((previous_less(a) == vector<int>{-1, -1, 1, 1}));
  assert((next_less(a) == vector<int>{1, 4, 3, 4}));
  assert((previous_greater(a) == vector<int>{-1, 0, -1, 2}));
  assert((next_greater(a) == vector<int>{2, 2, 4, 4}));

  vector<int> hist{2, 1, 5, 6, 2, 3};
  assert(largest_rectangle(hist) == 10);
  return 0;
}
