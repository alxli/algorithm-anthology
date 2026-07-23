/*

Given a set of integers, find the minimum excluded nonnegative value (MEX), that is, the smallest
integer $x \geq 0$ that does not appear in the set. MEX often appears in array problems, game theory
with Grundy numbers, and dynamic programming states. Since the MEX of $n$ values never exceeds $n$,
the one-shot version tracks seen values in $[0, n]$ before scanning to find the first missing.

- `mex(lo, hi)` returns the MEX of the values in $[`lo`, `hi`)$.

The MEX of a dynamic multiset can be computed using counts of present values and an ordered set of
currently missing candidates.

- `DynamicMex()` maintains the MEX of a multiset of nonnegative integers.
- `add(x)` inserts one copy of value `x` if `x` $\geq 0$.
- `remove(x)` removes one copy of value `x` if present.
- `get()` returns the current MEX.

Time Complexity:
- O(n) per call to `mex(lo, hi)`, where $n$ is the number of values.
- O(log n) expected per call to `add(x)` and `remove(x)` for `DynamicMex`, and O(n) in the
  collision-heavy worst case for the hash table.
- O(1) per call to `get()` for `DynamicMex`.

Space Complexity:
- O(n) auxiliary for `mex(lo, hi)`.
- O(n) storage for `DynamicMex`.

*/

#include <iterator>
#include <set>
#include <unordered_map>
#include <vector>

template<typename It>
int mex(It lo, It hi) {
  int n = std::distance(lo, hi);
  std::vector<char> seen(n + 1, false);
  for (It it = lo; it != hi; ++it) {
    if (0 <= *it && *it <= n) {
      seen[*it] = true;
    }
  }
  for (int x = 0; x <= n; x++) {
    if (!seen[x]) {
      return x;
    }
  }
  return n + 1;
}

class DynamicMex {
  std::unordered_map<int, int> count;
  std::set<int> missing;  // Missing candidates in [0, size].
  int size = 0;

 public:
  DynamicMex() { missing.insert(0); }

  void add(int x) {
    if (x < 0) {
      return;
    }
    if (!count.count(size + 1)) {
      missing.insert(size + 1);
    }
    if (count[x]++ == 0) {
      missing.erase(x);
    }
    size++;
  }

  void remove(int x) {
    auto it = count.find(x);
    if (it == count.end()) {
      return;
    }
    bool removed_last = --it->second == 0;
    if (removed_last) {
      count.erase(it);
    }
    size--;
    missing.erase(size + 1);
    if (removed_last && x <= size) {
      missing.insert(x);
    }
  }

  int get() const { return *missing.begin(); }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{0, 1, 4, 2, 1};
  assert(mex(a.begin(), a.end()) == 3);
  vector<int> b{-1, 0, 1, 2};
  assert(mex(b.begin(), b.end()) == 3);  // Negative values are ignored.

  DynamicMex m;
  m.add(0);
  m.add(1);
  m.add(1);
  m.add(3);
  assert(m.get() == 2);
  m.add(2);
  assert(m.get() == 4);
  m.remove(1);
  assert(m.get() == 4);  // One copy of 1 remains.
  m.remove(1);
  assert(m.get() == 1);
  m.remove(100);
  assert(m.get() == 1);
  return 0;
}
