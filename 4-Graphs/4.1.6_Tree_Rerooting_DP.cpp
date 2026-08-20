/*

Given a DP on a rooted tree, compute its result for every choice of root. For each direction
$u \to v$ of an edge $u$-$v$, removing the edge leaves a component containing $u$; rerooting DP
summarizes that component as a contribution from $u$ to $v$. After rooting the tree arbitrarily, a
bottom-up pass computes contributions toward the initial root, and a top-down pass computes
contributions in the opposite direction. Prefix/suffix aggregation combines all contributions except
the one from a chosen neighbor in O(1) time. This requires `combine` to be associative and gives an
O(n) algorithm; a segment-tree/exclusive-combine variant can instead support a left-fold style
interface in O(n log n).

- `rerooting_dp(identity, combine, finalize, lift)` returns the finalized aggregate for every node
  when chosen as the root. The global, bidirectionally pre-populated adjacency list `adj` must form
  a nonempty tree whose indices represent its nodes.

The aggregation scheme for `rerooting_dp()` is defined by the following pieces:
- `Summary` is the type used for component aggregates and neighbor contributions. A contribution
  from $u$ to $v$ summarizes the component containing $u$ after removing edge $u$-$v$, transformed
  so it can be combined at $v$.
- `identity` is the neutral value for an empty component, which for all aggregate values `a` must
  satisfy `combine(a, identity)` = `combine(identity, a)` = `a`.
- `combine(a, b)` merges two independent neighbor contributions expressed at the same node.
- `finalize(acc, u)` incorporates node `u` after its available neighbor contributions have been
  merged.
- `lift(acc, u, v)` transfers a finalized aggregate at node `u` across edge `u`-`v`, producing a
  contribution expressed at adjacent node `v`.

This covers problems such as subtree sizes, sum/max of distances from every root, and rerooted
polynomial/hash-like aggregates, as long as sibling contributions can be combined associatively. The
usage example shows how to compute the sum of distances from every root: `Summary` stores `sum`, the
total distance from the current boundary node to all nodes in the summarized component, and `nodes`,
the number of nodes in that component. The node count is needed because `lift(acc, u, v)` crosses
one edge, so every represented node becomes 1 farther from `v` and `sum` increases by `nodes`.

Time Complexity:
- O(n) per call to `rerooting_dp()`, assuming O(1) callback functions.

Space Complexity:
- O(n) for the tree and auxiliary arrays.
- O(n) auxiliary stack space for the recursive search.

*/

#include <vector>

std::vector<std::vector<int>> adj;

template<typename Summary, typename Combine, typename Finalize, typename Lift>
std::vector<Summary> rerooting_dp(
    const Summary &identity, Combine combine, Finalize finalize, Lift lift
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
  std::vector<Summary> down(n, identity), outside(n, identity), answer(n, identity);
  for (int i = n - 1; i >= 0; i--) {
    int u = order[i];
    Summary acc = identity;
    for (int v : adj[u]) {
      if (parent[v] == u) {
        acc = combine(acc, lift(down[v], v, u));
      }
    }
    down[u] = finalize(acc, u);
  }
  for (int u : order) {
    std::vector<Summary> vals;
    for (int v : adj[u]) {
      if (parent[v] == u) {
        vals.push_back(lift(down[v], v, u));
      } else {
        vals.push_back(outside[u]);
      }
    }
    std::vector<Summary> suffix(vals.size() + 1, identity);
    for (int i = static_cast<int>(vals.size()) - 1; i >= 0; i--) {
      suffix[i] = combine(vals[i], suffix[i + 1]);
    }
    Summary prefix = identity;
    for (int i = 0; i < static_cast<int>(adj[u].size()); i++) {
      int v = adj[u][i];
      if (parent[v] == u) {
        Summary without_child = finalize(combine(prefix, suffix[i + 1]), u);
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
#include <cstdint>
#include <string>
using namespace std;

vector<int64_t> sum_distances_all_roots() {
  struct DistanceSummary {
    int64_t sum = 0;
    int nodes = 0;
  };
  auto combine = [](DistanceSummary a, DistanceSummary b) {
    return DistanceSummary{a.sum + b.sum, a.nodes + b.nodes};
  };
  auto finalize = [](DistanceSummary a, int) {
    a.nodes++;
    return a;
  };
  auto lift = [](DistanceSummary a, int, int) {
    a.sum += a.nodes;
    return a;
  };
  vector<DistanceSummary> dp = rerooting_dp(DistanceSummary{}, combine, finalize, lift);
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
  auto combine = [](const string &a, const string &b) { return a + b; };
  auto finalize = [](const string &a, int u) { return a + to_string(u); };
  auto lift = [](const string &a, int, int) { return a; };
  assert(rerooting_dp(string{}, combine, finalize, lift)[1] == "201");
  return 0;
}
