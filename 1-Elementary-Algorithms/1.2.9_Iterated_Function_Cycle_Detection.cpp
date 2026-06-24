/*

Given a function $f$ mapping a finite set to itself and a starting value $x_0$, return the
position and length of the cycle reached by repeatedly applying $f$. Formally, the sequence
  $x_0, x_1 = f(x_0), x_2 = f(x_1), \ldots, x_n = f(x_{n - 1}), \ldots$
must eventually repeat some value. If $x_i = x_j$ for $i < j$, then the sequence from $x_i$ to
$x_{j - 1}$ repeats forever. This is useful for detecting cycles in functional graphs, pseudo-random
generators, Pollard rho style algorithms, degenerate linked lists, and arrays where each index
points to the next index. For example, if `nums` has size $n + 1$ and all values are in $[1, n]$,
then $f(i)$ = `nums[i]` forms a functional graph; the duplicate value is the entry point of the
cycle reached from 0.

Floyd's cycle-finding algorithm, a.k.a. the "tortoise and the hare algorithm", is a space-efficient
default choice: it moves two pointers through the sequence at different speeds, finds a meeting
point inside the cycle, then locates the cycle start. Brent's algorithm is a compact variant that
usually calls $f$ fewer times by only resetting the tortoise at powers of two; prefer it when
evaluating $f$ is expensive and constant factors matter. In the detection phase, Brent advances
one pointer with one call to $f$ per step, while Floyd advances the tortoise once and the hare
twice, using three calls to $f$ per step.

For Floyd's algorithm, suppose the tail has length $m$ and the cycle has length $n$. At a meeting,
the hare has traveled twice as far as the tortoise, so the tortoise's distance is a multiple of $n$.
Consequently, the distance from the meeting point around the cycle to its entry is congruent to $m$
modulo $n$. Resetting one pointer to $x_0$ and advancing both one step at a time makes them meet at
the cycle entry after $m$ steps. Brent first determines $n$, then advances one pointer $n$ steps;
moving both together from that fixed separation likewise makes them meet at the entry.

- `find_cycle_floyd(f, x0)` returns the reached cycle as a pair $(`start`, `length`)$, where `start`
  is the smallest index $i$ such that $x_i$ is in the cycle, and `length` is the number of distinct
  values in the cycle.
- `find_cycle_brent(f, x0)` does the same with fewer calls to $f$ in many cases.

Time Complexity:
- O(m + n) per call, where $m$ is the first index in the cycle and $n$ is the cycle length.

Space Complexity:
- O(1) auxiliary.

*/

#include <utility>

template<typename Fn, typename T>
std::pair<int, int> find_cycle_floyd(Fn f, T x0) {
  // Find a meeting point inside the cycle.
  T tortoise = f(x0), hare = f(f(x0));
  while (tortoise != hare) {
    tortoise = f(tortoise);
    hare = f(f(hare));
  }
  // Find the cycle entry.
  int start = 0;
  tortoise = x0;
  while (tortoise != hare) {
    tortoise = f(tortoise);
    hare = f(hare);
    start++;
  }
  // Measure the cycle length.
  int length = 1;
  hare = f(tortoise);
  while (tortoise != hare) {
    hare = f(hare);
    length++;
  }
  return {start, length};
}

template<typename Fn, typename T>
std::pair<int, int> find_cycle_brent(Fn f, T x0) {
  // Find the cycle length by doubling the search block.
  int power = 1, length = 1;
  T tortoise = x0, hare = f(x0);
  while (tortoise != hare) {
    if (power == length) {
      tortoise = hare;
      power *= 2;
      length = 0;
    }
    hare = f(hare);
    length++;
  }
  // Separate the pointers by one cycle length.
  hare = x0;
  for (int i = 0; i < length; i++) {
    hare = f(hare);
  }
  // Find the cycle entry.
  int start = 0;
  tortoise = x0;
  while (tortoise != hare) {
    tortoise = f(tortoise);
    hare = f(hare);
    start++;
  }
  return {start, length};
}

/*** Example Usage ***/

#include <cassert>
#include <set>
#include <vector>
using namespace std;

int f(int x) {
  return (123 * x * x + 4567890) % 1337;
}

void verify(int x0, int start, int length) {
  set<int> s;
  int x = x0;
  for (int i = 0; i < start; i++) {
    assert(!s.count(x));
    s.insert(x);
    x = f(x);
  }
  int startx = x;
  s.clear();
  for (int i = 0; i < length; i++) {
    assert(!s.count(x));
    s.insert(x);
    x = f(x);
  }
  assert(startx == x);
}

// Given n + 1 integers within [1, n], where one value in [1, n] is present 2 or more times (but
// other values in [1, n] are present at most once), find the duplicate without modifying the array.
// Interpret each value as a pointer to the next index; the duplicate is the cycle entry.
int find_duplicate_integer(const vector<int> &nums) {
  auto [start, _] = find_cycle_floyd([&](int i) { return nums[i]; }, 0);
  int res = 0;
  for (int i = 0; i < start; i++) {
    res = nums[res];
  }
  return res;
}

int main() {
  int x0 = 0;
  auto [floyd_start, floyd_len] = find_cycle_floyd(f, x0);
  assert(floyd_start == 4 && floyd_len == 2);
  verify(x0, floyd_start, floyd_len);

  auto [brent_start, brent_len] = find_cycle_brent(f, x0);
  assert(brent_start == floyd_start && brent_len == floyd_len);

  assert(find_duplicate_integer({1, 3, 4, 2, 2}) == 2);
  assert(find_duplicate_integer({3, 1, 3, 4, 2}) == 3);
  return 0;
}
