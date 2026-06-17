/*

Maintains the number of connected components of an undirected graph as edges are added and removed
over time, answering queries offline once the entire sequence of operations is known. Each edge is
alive over a contiguous interval of time, from when it is added until it is removed. These intervals
are placed into a segment tree over the time axis, so that for $T$ operations, each edge appears in
O(log T) nodes whose ranges it spans. A depth-first traversal of this segment tree unites the edges
stored at each node on the way down and undoes them on the way back up, so at each leaf the disjoint
set forest reflects exactly the edges alive at that moment.

Undoing unions requires a disjoint set forest with rollback (see the disjoint sets section): it
joins by size or rank without path compression, recording each change on a stack so it can be
reversed. Path compression is incompatible with rollback, so finding a representative costs
O(log n) rather than near-constant time.

- `OfflineDynamicConnectivity(n)` creates a structure of `n` nodes numbered [0, `n`), initially with
  no edges.
- `add_edge(u, v)` and `remove_edge(u, v)` record adding or removing the undirected edge `u`-`v` at
  the current time, then advance the time by one step. Each edge may be present at most once at a
  time: `add_edge` must not name an edge that is already present, and `remove_edge` must name one
  that is. Both preconditions are checked with `assert` during `solve()`.
- `count_components()` records a query at the current time and advances the time by one step.
- `solve()` processes all recorded operations and returns a vector holding, for each
  `count_components()` query in the order issued, the number of connected components at that time.

Time Complexity:
- O((T + m log T) log n) for `solve()`, where $T$ is the total number of operations (`add_edge`,
  `remove_edge`, and `count_components` combined), $m$ is the number of `add_edge` operations, and
  $n$ is the number of vertices.
- O(1) amortized per call to `add_edge()`, `remove_edge()`, and `count_components()`.

Space Complexity:
- O(n + m log T) auxiliary heap space.

*/

#include <algorithm>
#include <cassert>
#include <map>
#include <numeric>
#include <utility>
#include <vector>

class OfflineDynamicConnectivity {
  static const int ADD = 0, REMOVE = 1, QUERY = 2;

  struct Op {
    int type, u, v;
  };

  int n, comps;
  std::vector<Op> ops;
  std::vector<int> par, rnk;
  std::vector<std::pair<int, int>> undo;  // (child root, whether its parent's rank increased)
  std::vector<std::vector<std::pair<int, int>>> seg;
  std::vector<int> answers;

  int find(int x) const {
    while (par[x] != x) {
      x = par[x];
    }
    return x;
  }

  void unite(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b) {
      undo.emplace_back(-1, 0);
      return;
    }
    if (rnk[a] < rnk[b]) {
      std::swap(a, b);
    }
    undo.emplace_back(b, rnk[a] == rnk[b] ? 1 : 0);
    par[b] = a;
    if (rnk[a] == rnk[b]) {
      rnk[a]++;
    }
    comps--;
  }

  void rollback() {
    auto [b, raised] = undo.back();
    undo.pop_back();
    if (b == -1) {
      return;
    }
    rnk[par[b]] -= raised;
    par[b] = b;
    comps++;
  }

  void add_interval(int node, int lo, int hi, int l, int r, const std::pair<int, int> &e) {
    if (r < lo || hi < l) {
      return;
    }
    if (l <= lo && hi <= r) {
      seg[node].push_back(e);
      return;
    }
    int mid = lo + (hi - lo) / 2;
    add_interval(2 * node, lo, mid, l, r, e);
    add_interval(2 * node + 1, mid + 1, hi, l, r, e);
  }

  void dfs(int node, int lo, int hi) {
    int applied = 0;
    for (const auto &[eu, ev] : seg[node]) {
      unite(eu, ev);
      applied++;
    }
    if (lo == hi) {
      if (ops[lo].type == QUERY) {
        answers.push_back(comps);
      }
    } else {
      int mid = lo + (hi - lo) / 2;
      dfs(2 * node, lo, mid);
      dfs(2 * node + 1, mid + 1, hi);
    }
    while (applied-- > 0) {
      rollback();
    }
  }

 public:
  OfflineDynamicConnectivity(int n) : n(n) {}

  void add_edge(int u, int v) { ops.push_back({ADD, std::min(u, v), std::max(u, v)}); }
  void remove_edge(int u, int v) { ops.push_back({REMOVE, std::min(u, v), std::max(u, v)}); }
  void count_components() { ops.push_back({QUERY, 0, 0}); }

  std::vector<int> solve() {
    int t = static_cast<int>(ops.size());
    answers.clear();
    if (t == 0) {
      return answers;
    }
    seg.assign(4 * t, {});
    std::map<std::pair<int, int>, int> active;
    for (int i = 0; i < t; i++) {
      std::pair<int, int> key(ops[i].u, ops[i].v);
      if (ops[i].type == ADD) {
        assert(active.find(key) == active.end());  // The edge must not already be present.
        active[key] = i;
      } else if (ops[i].type == REMOVE) {
        auto it = active.find(key);
        assert(it != active.end());  // The edge must currently be present.
        add_interval(1, 0, t - 1, it->second, i - 1, {ops[i].u, ops[i].v});
        active.erase(it);
      }
    }
    for (const auto &[key, start] : active) {
      add_interval(1, 0, t - 1, start, t - 1, {key.first, key.second});
    }
    par.resize(n);
    std::iota(par.begin(), par.end(), 0);
    rnk.assign(n, 0);
    comps = n;
    undo.clear();
    dfs(1, 0, t - 1);
    return answers;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  OfflineDynamicConnectivity dc(4);
  dc.count_components();  // 4 isolated vertices.
  dc.add_edge(0, 1);
  dc.add_edge(2, 3);
  dc.count_components();  // {0,1} {2,3} -> 2 components.
  dc.add_edge(1, 2);
  dc.count_components();  // All four connected -> 1 component.
  dc.remove_edge(1, 2);
  dc.count_components();  // Back to 2 components.

  assert((dc.solve() == vector<int>{4, 2, 1, 2}));
  return 0;
}
