/*

Given a tree DP that can answer one rooted query with a recursive `dfs(u, p)`, compute that same
answer for every possible root. Rerooting DP first stores parent pointers and preorder from an
arbitrary root. A bottom-up pass computes the usual child-to-parent DFS values, then a top-down pass
uses prefix/suffix aggregation to compute the missing parent-to-child values. The prefix/suffix
aggregation requires `combine` to be associative for a running time of O(n), which is usually
preferred, but a segment-tree/exclusive-combine variant can support a left-fold style interface in
O(n log n).

- `rerooting_dp(identity, combine, finalize, lift)` returns the value that `dfs(root, p)` would
  produce for every possible `root` with sentinel parent $p = -1$. The global, bidirectionally
  pre-populated adjacency list `adj` must form a nonempty tree whose indices represent its nodes.

The aggregation scheme for `rerooting_dp()` is defined by the following pieces:
- `Data` is the return type of a normal fixed-root DFS call `dfs(u, p)`. It summarizes the component
  containing `u` after removing edge `u`-`p`, with all values measured from node `u`.
- `identity` is the neutral value for an empty component, which for all aggregate values `a` must
  satisfy `combine(a, identity)` = `combine(identity, a)` = `a`.
- `combine(a, b)` merges two independent neighbor contributions into the same node `u`, like two
  different child DFS returns.
- `finalize(acc, u)` finalizes node `u` after all neighbor contributions have been merged. This is
  the last line of a normal `dfs(u, p)` before returning.
- `lift(acc, u, p)` converts a finished `dfs(u, p)` value into the contribution that node `u` gives
  to adjacent node `p`. Here `p` is the neighbor treated as the parent/excluded side of the directed
  DFS state, not necessarily `u`'s parent in the initial root-$0$ traversal.

This covers problems such as subtree sizes, sum/max of distances from every root, and rerooted
polynomial/hash-like aggregates, as long as sibling contributions can be combined associatively. The
usage example shows how to compute the sum of distances from every root: `Data` stores `sum`, the
total distance from `u` to all nodes in `dfs(u, p)`, and `nodes`, the node count in that component.
The node count is needed because `lift(acc, u, p)` crosses one edge, so every represented node
becomes 1 farther from `p` and `sum` increases by `nodes`.

Time Complexity:
- O(n) per call to `rerooting_dp()`, assuming O(1) callback functions.

Space Complexity:
- O(n) for the tree and auxiliary arrays.
- O(n) auxiliary stack space for the recursive search.

*/

#include <cstdint>
#include <vector>

std::vector<std::vector<int>> adj;

template<typename Data, typename Combine, typename Finalize, typename Lift>
std::vector<Data> rerooting_dp(
    const Data &identity, Combine combine, Finalize finalize, Lift lift
) {
  int n = static_cast<int>(adj.size());
  std::vector<int> parent(n, -1), order;
  auto dfs = [&](auto &&dfs, int u, int p) -> void {
    parent[u] = p;
    order.push_back(u);
    for (int v : adj[u]) {
      if (v != p) {
        dfs(dfs, v, u);
      }
    }
  };
  dfs(dfs, 0, -1);
  std::vector<Data> down(n, identity), outside(n, identity), answer(n, identity);
  for (int i = n - 1; i >= 0; i--) {
    int u = order[i];
    Data acc = identity;
    for (int v : adj[u]) {
      if (parent[v] == u) {
        acc = combine(acc, lift(down[v], v, u));
      }
    }
    down[u] = finalize(acc, u);
  }
  for (int u : order) {
    std::vector<Data> vals;
    for (int v : adj[u]) {
      if (parent[v] == u) {
        vals.push_back(lift(down[v], v, u));
      } else {
        vals.push_back(outside[u]);
      }
    }
    std::vector<Data> suffix(vals.size() + 1, identity);
    for (int i = static_cast<int>(vals.size()) - 1; i >= 0; i--) {
      suffix[i] = combine(vals[i], suffix[i + 1]);
    }
    Data prefix = identity;
    for (int i = 0; i < static_cast<int>(adj[u].size()); i++) {
      int v = adj[u][i];
      if (parent[v] == u) {
        Data without_child = finalize(combine(prefix, suffix[i + 1]), u);
        outside[v] = lift(without_child, u, v);
      }
      prefix = combine(prefix, vals[i]);
    }
    answer[u] = finalize(prefix, u);
  }
  return answer;
}

/*** Example Usage ***/

#include <cassert>
#include <string>
using namespace std;

vector<int64_t> sum_distances_all_roots() {
  struct DistanceData {
    int64_t sum = 0;
    int nodes = 0;
  };
  auto combine = [](DistanceData a, DistanceData b) {
    return DistanceData{a.sum + b.sum, a.nodes + b.nodes};
  };
  auto finalize = [](DistanceData a, int) {
    a.nodes++;
    return a;
  };
  auto lift = [](DistanceData a, int, int) {
    a.sum += a.nodes;
    return a;
  };
  vector<DistanceData> dp = rerooting_dp(DistanceData{}, combine, finalize, lift);
  vector<int64_t> answer(dp.size());
  for (int i = 0; i < static_cast<int>(dp.size()); i++) {
    answer[i] = dp[i].sum;
  }
  return answer;
}

int main() {
  // 0---1---2
  //     |
  //     3---4
  adj = {{1}, {0, 2, 3}, {1}, {1, 4}, {3}};
  assert((sum_distances_all_roots() == vector<int64_t>{8, 5, 8, 6, 9}));

  // Associative contributions retain adjacency-list order even when combine is not commutative.
  adj = {{1}, {2, 0}, {1}};  // adj[1] is [2, 0], while 0 is its initial parent.
  auto combine = [](string a, const string &b) { return a + b; };
  auto finalize = [](string a, int u) { return a + to_string(u); };
  auto lift = [](const string &a, int, int) { return a; };
  assert(rerooting_dp(string{}, combine, finalize, lift)[1] == "201");
  return 0;
}
