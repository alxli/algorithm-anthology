/*

A functional graph is a directed graph in which every node has exactly one outgoing edge, given by a
successor function `f`, where `f[i]` is the node reached in one step from node `i`. Following the
edges from any node eventually enters a cycle, so each weakly connected component consists of one
directed cycle with trees of nodes feeding into it. This structure underlies problems about
iterating a function: where a sequence `i`, `f[i]`, `f[f[i]]`, ... ends up, and how far it is from
repeating.

Building the structure peels away nodes of in-degree zero to expose the cycles (the survivors), then
labels each cycle and walks the remaining tail nodes to record their distance to the cycle. A binary
lifting table stores `f` applied $2^k$ times, so any number of steps can be taken in logarithmic
time. Together these answer, for any node, which cycle it eventually reaches, how many steps until
it gets there, and exactly where it lands after a huge number of steps.

- `FunctionalGraph(f)` builds the structure for the successor function `f` over nodes numbered
  $[0, `n`)$, where `n` is `f.size()`.
- `on_cycle(i)` returns whether node `i` lies on a cycle.
- `dist_to_cycle(i)` returns the number of steps from `i` to the first cycle node it reaches ($0$ if
  `i` is itself on a cycle).
- `cycle_id(i)` returns an identifier for the cycle that `i` eventually reaches; two nodes share an
  identifier exactly when they funnel into the same cycle.
- `cycle_length(id)` returns the length of the cycle with the given identifier.
- `num_cycles()` returns the number of distinct cycles.
- `successor(i, steps)` returns the node reached from `i` after applying `f` exactly `steps` times,
  for any nonnegative `steps` (which may be astronomically large).

Time Complexity:
- O(n log n) per call to the constructor, where $n$ is the number of nodes.
- O(log n) per call to `successor()`.
- O(1) per call to all other member functions.

Space Complexity:
- O(n log n) for storage of the binary lifting table.

*/

#include <cstdint>
#include <vector>

class FunctionalGraph {
  int n, LOG;
  std::vector<std::vector<int>> up;  // up[k][i] = f applied 2^k times to i.
  std::vector<char> on_cyc;
  std::vector<int> to_cyc;  // Steps from i to the first cycle node.
  std::vector<int> comp;    // Identifier of the cycle that i reaches.
  std::vector<int> clen;    // clen[c] = length of cycle c.

  int jump(int i, int64_t steps) const {
    for (int k = 0; steps > 0; k++, steps >>= 1) {
      if (steps & 1) {
        i = up[k][i];
      }
    }
    return i;
  }

 public:
  FunctionalGraph(const std::vector<int> &f) : n(f.size()) {
    LOG = 1;
    while ((1 << LOG) < n) {
      LOG++;
    }
    up.assign(LOG + 1, std::vector<int>(n));
    up[0] = f;
    for (int k = 1; k <= LOG; k++) {
      for (int i = 0; i < n; i++) {
        up[k][i] = up[k - 1][up[k - 1][i]];
      }
    }
    // Peel nodes of in-degree zero; the survivors are exactly the cycle nodes.
    std::vector<int> indeg(n, 0);
    for (int i = 0; i < n; i++) {
      indeg[f[i]]++;
    }
    std::vector<char> removed(n, false);
    std::vector<int> stack;
    for (int i = 0; i < n; i++) {
      if (indeg[i] == 0) {
        stack.push_back(i);
      }
    }
    while (!stack.empty()) {
      int u = stack.back();
      stack.pop_back();
      removed[u] = true;
      if (--indeg[f[u]] == 0) {
        stack.push_back(f[u]);
      }
    }
    on_cyc.assign(n, false);
    for (int i = 0; i < n; i++) {
      on_cyc[i] = !removed[i];
    }
    // Label each cycle and record its length.
    comp.assign(n, -1);
    to_cyc.assign(n, 0);
    for (int i = 0; i < n; i++) {
      if (on_cyc[i] && comp[i] == -1) {
        int len = 0;
        for (int u = i; comp[u] == -1; u = f[u]) {
          comp[u] = static_cast<int>(clen.size());
          len++;
        }
        clen.push_back(len);
      }
    }
    // Walk each tail to the cycle, assigning its component and distance.
    for (int i = 0; i < n; i++) {
      if (comp[i] != -1) {
        continue;
      }
      std::vector<int> path;
      int u = i;
      while (comp[u] == -1) {
        path.push_back(u);
        u = f[u];
      }
      for (int j = static_cast<int>(path.size()) - 1, step = 1; j >= 0; j--, step++) {
        comp[path[j]] = comp[u];
        to_cyc[path[j]] = to_cyc[u] + step;
      }
    }
  }

  bool on_cycle(int i) const { return on_cyc[i]; }
  int dist_to_cycle(int i) const { return to_cyc[i]; }
  int cycle_id(int i) const { return comp[i]; }
  int cycle_length(int id) const { return clen[id]; }
  int num_cycles() const { return static_cast<int>(clen.size()); }

  int successor(int i, int64_t steps) const {
    if (steps <= to_cyc[i]) {
      return jump(i, steps);
    }
    steps -= to_cyc[i];
    i = jump(i, to_cyc[i]);  // Advance to the cycle entry node.
    steps %= clen[comp[i]];
    return jump(i, steps);
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Cycle 0 -> 1 -> 2 -> 0, with a tail 5 -> 4 -> 3 -> 1 and 6 -> 2.
  vector<int> f{1, 2, 0, 1, 3, 4, 2};
  FunctionalGraph g(f);

  assert(g.on_cycle(0) && g.on_cycle(1) && g.on_cycle(2));
  assert(!g.on_cycle(3) && !g.on_cycle(5));
  assert(g.num_cycles() == 1);
  assert(g.cycle_length(0) == 3);

  assert(g.dist_to_cycle(2) == 0);
  assert(g.dist_to_cycle(5) == 3);
  assert(g.dist_to_cycle(6) == 1);
  assert(g.cycle_id(5) == g.cycle_id(0));

  assert(g.successor(5, 1) == 4);
  assert(g.successor(5, 3) == 1);    // 5 -> 4 -> 3 -> 1.
  assert(g.successor(5, 4) == 2);    // One more step around into the cycle.
  assert(g.successor(0, 100) == 1);  // 100 mod 3 == 1 step around the cycle.
  return 0;
}
