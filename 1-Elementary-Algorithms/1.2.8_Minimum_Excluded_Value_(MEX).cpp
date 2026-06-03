/*

Computes the minimum excluded nonnegative integer, commonly called the MEX. The MEX of a set is the
smallest integer $x \geq 0$ that does not appear in the set. It appears frequently in array
problems, game theory with Grundy numbers, and dynamic programming states.

- `mex(lo, hi)` returns the MEX of the values in `[lo, hi)`.
- `dynamic_mex(n)` maintains counts of values in `[0, n]`, enough to track the MEX of a multiset of
  at most `n` relevant nonnegative values.
- `add(x)` inserts one copy of value `x` if `0 <= x <= n`.
- `remove(x)` removes one copy of value `x` if present.
- `get()` returns the current MEX.

Time Complexity:
- O(n) per call to `mex(lo, hi)`, where $n$ is the number of values.
- O(log n) per call to `add(x)`, `remove(x)`, and `get()` for `dynamic_mex`.

Space Complexity:
- O(n) auxiliary heap space for both approaches.

*/

#include <set>
#include <vector>

template<class It>
int mex(It lo, It hi) {
  int n = hi - lo;
  std::vector<bool> seen(n + 1, false);
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

class dynamic_mex {
  std::vector<int> count;
  std::set<int> missing;

 public:
  explicit dynamic_mex(int n) : count(n + 1, 0) {
    for (int x = 0; x <= n; x++) {
      missing.insert(x);
    }
  }

  void add(int x) {
    if (0 <= x && x < (int)count.size() && count[x]++ == 0) {
      missing.erase(x);
    }
  }

  void remove(int x) {
    if (0 <= x && x < (int)count.size() && count[x] > 0 && --count[x] == 0) {
      missing.insert(x);
    }
  }

  int get() const { return *missing.begin(); }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  int raw[] = {0, 1, 4, 2, 1};
  assert(mex(raw, raw + 5) == 3);

  dynamic_mex m(5);
  m.add(0);
  m.add(1);
  m.add(3);
  assert(m.get() == 2);
  m.add(2);
  assert(m.get() == 4);
  m.remove(1);
  assert(m.get() == 1);
  return 0;
}
