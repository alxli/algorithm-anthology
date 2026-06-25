/*

Given a tree DP that can answer one rooted query with a recursive `dfs(u, p)`, compute that same
answer for every possible root. Rerooting DP first stores parent pointers, depths, preorder,
subtree sizes, and entry/exit times. A bottom-up pass computes the usual child-to-parent DFS values,
then a top-down pass uses prefix/suffix aggregation to compute the missing parent-to-child values.
The prefix/suffix aggregation requires `combine` to be associative for a running time of O(n), which
is usually preferred, but a segment-tree/exclusive-combine variant can support a left-fold style
interface in O(n log n).

- `RerootingTree(n)` constructs a tree with `n` nodes numbered $[0, `n`)$.
- `add_edge(u, v)` adds an undirected edge between nodes `u` and `v`.
- `build_rooted_tree(root)` populates `parent`, `depth`, `subtree_size`, `order`, `tin`, and `tout`
  for the tree rooted at `root` (default 0). This must be called only after the added edges define a
  connected tree of size `n`.
- `is_ancestor(u, v)` returns whether `u` is an ancestor of `v` in the last rooted tree.
- `rerooting_dp(identity, combine, finalize, lift)` returns the value that `dfs(root, p)` would
  produce for every possible `root` with sentinel parent $p = -1$. This must be called only after
  the added edges define a connected tree of size `n`.

The aggregation scheme for `rerooting_dp()` is defined by the following pieces:
- `Data` is the return type of a normal fixed-root DFS call `dfs(u, p)`. It summarizes the
  component containing `u` after removing edge `u`-`p`, with all values measured from node `u`.
- `identity` is the neutral value for an empty component, which for all aggregate values `a` must
  satisfy `combine(a, identity) = combine(identity, a) = a`.
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
- O(1) per call to `add_edge()`.
- O(n) per call to `build_rooted_tree()`, where $n$ is the number of nodes.
- O(1) per call to `is_ancestor()`.
- O(n) per call to `rerooting_dp()`, assuming O(1) callback functions.

Space Complexity:
- O(n) for the tree, metadata, and DP arrays.
- O(n) auxiliary stack space for the recursive search.

*/

#include <cstdint>
#include <vector>

class RerootingTree {
  int timer;

  void dfs_rooted(int u, int p) {
    parent[u] = p;
    tin[u] = timer++;
    order.push_back(u);
    subtree_size[u] = 1;
    for (int v : adj[u]) {
      if (v == p) {
        continue;
      }
      depth[v] = depth[u] + 1;
      dfs_rooted(v, u);
      subtree_size[u] += subtree_size[v];
    }
    tout[u] = timer;
  }

 public:
  std::vector<std::vector<int>> adj;
  std::vector<int> parent, depth, subtree_size, order, tin, tout;

  explicit RerootingTree(int n) : timer(0), adj(n) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  }

  void build_rooted_tree(int root = 0) {
    int n = static_cast<int>(adj.size());
    parent.assign(n, -1);
    depth.assign(n, 0);
    subtree_size.assign(n, 0);
    order.clear();
    tin.assign(n, 0);
    tout.assign(n, 0);
    timer = 0;
    dfs_rooted(root, -1);
  }

  bool is_ancestor(int u, int v) const { return tin[u] <= tin[v] && tout[v] <= tout[u]; }

  template<typename Data, typename Combine, typename Finalize, typename Lift>
  std::vector<Data> rerooting_dp(
      const Data &identity, Combine combine, Finalize finalize, Lift lift
  ) {
    int n = static_cast<int>(adj.size());
    std::vector<Data> down(n, identity), outside(n, identity), answer(n, identity);
    build_rooted_tree();
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
      std::vector<int> children;
      std::vector<Data> vals;
      for (int v : adj[u]) {
        if (parent[v] == u) {
          children.push_back(v);
          vals.push_back(lift(down[v], v, u));
        }
      }
      std::vector<Data> suffix(vals.size() + 1, identity);
      for (int i = static_cast<int>(vals.size()) - 1; i >= 0; i--) {
        suffix[i] = combine(vals[i], suffix[i + 1]);
      }
      Data prefix = outside[u];
      for (int i = 0; i < static_cast<int>(children.size()); i++) {
        Data without_child = finalize(combine(prefix, suffix[i + 1]), u);
        outside[children[i]] = lift(without_child, u, children[i]);
        prefix = combine(prefix, vals[i]);
      }
      answer[u] = finalize(prefix, u);
    }
    return answer;
  }
};

/*** Example Usage and Output:

Sum of distances from each root: 8 5 8 6 9

***/

#include <cassert>
#include <iostream>
using namespace std;

vector<int64_t> sum_distances_all_roots(RerootingTree &tree) {
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
  vector<DistanceData> dp = tree.rerooting_dp(DistanceData{}, combine, finalize, lift);
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
  RerootingTree tree(5);
  tree.add_edge(0, 1);
  tree.add_edge(1, 2);
  tree.add_edge(1, 3);
  tree.add_edge(3, 4);
  tree.build_rooted_tree();
  assert(tree.parent[4] == 3);
  assert(tree.depth[4] == 3);
  assert(tree.subtree_size[1] == 4);
  assert(tree.is_ancestor(1, 4));
  assert(!tree.is_ancestor(3, 2));
  vector<int64_t> distances = sum_distances_all_roots(tree);
  assert((distances == vector<int64_t>{8, 5, 8, 6, 9}));
  cout << "Sum of distances from each root:";
  for (int64_t d : distances) {
    cout << " " << d;
  }
  cout << "\n";
  return 0;
}
