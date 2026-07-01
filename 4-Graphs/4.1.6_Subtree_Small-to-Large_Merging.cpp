/*

Given a rooted tree, compute a mutable summary for every subtree by merging child summaries into
one kept summary. Small-to-large merging always keeps the largest child summary and absorbs smaller
summaries into it. Each stored item can move only into summaries at least twice as large as before,
so the total number of moved items is logarithmic per item.

This pattern is often called DSU on tree, although it is unrelated to the disjoint-set union data
structure used by Kruskal's algorithm. It is useful for subtree queries whose answer can be
maintained by inserting all items from child summaries into one mutable summary, such as distinct
colors, frequency modes, or other multiset-style statistics.

- `subtree_small_to_large_merge(adj, root, init, merge, answer)` visits the tree rooted at `root`.
  The callback `init(u)` returns the initial summary for node `u`; `merge(big, small)` absorbs one
  child summary into another; and `answer(u, summary)` records the final subtree summary for `u`.

The input must be a tree. Summary objects must support `.size()`, and `merge(big, small)` should
iterate over `small` and insert into `big`.

Time Complexity:
- O(n log n) expected moved items per call, where $n$ is the number of nodes.

Space Complexity:
- O(n) auxiliary stack space and O(n) auxiliary heap space.

*/

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <vector>

template<typename Summary, typename Init, typename Merge, typename Answer>
int dfs_subtree_small_to_large(
    int u, int p, const std::vector<std::vector<int>> &adj, std::vector<Summary> &summary,
    Init &init, Merge &merge, Answer &answer
) {
  int keep = u;
  summary[u] = init(u);
  for (int v : adj[u]) {
    if (v == p) {
      continue;
    }
    int child = dfs_subtree_small_to_large(v, u, adj, summary, init, merge, answer);
    if (summary[keep].size() < summary[child].size()) {
      std::swap(keep, child);
    }
    merge(summary[keep], summary[child]);
  }
  answer(u, summary[keep]);
  return keep;
}

template<typename Init, typename Merge, typename Answer>
void subtree_small_to_large_merge(
    const std::vector<std::vector<int>> &adj, int root, Init init, Merge merge, Answer answer
) {
  int n = static_cast<int>(adj.size());
  assert(0 <= root && root < n);
  using Summary = decltype(init(0));
  std::vector<Summary> summary(n);
  dfs_subtree_small_to_large(root, -1, adj, summary, init, merge, answer);
}

/*** Example Usage ***/

int main() {
  //          0:A
  //       /     |
  //     1:B    2:A
  //    /  |     |
  // 3:C  4:B   5:C
  std::vector<std::vector<int>> adj(6);
  auto add_edge = [&](int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  };
  add_edge(0, 1);
  add_edge(0, 2);
  add_edge(1, 3);
  add_edge(1, 4);
  add_edge(2, 5);

  std::vector<int> color{0, 1, 0, 2, 1, 2};
  std::vector<int> ans(6);
  auto init = [&](int u) {
    std::unordered_map<int, int> freq;
    freq[color[u]] = 1;
    return freq;
  };
  auto merge = [](std::unordered_map<int, int> &big, const std::unordered_map<int, int> &small) {
    for (const auto &[c, cnt] : small) {
      big[c] += cnt;
    }
  };
  auto answer = [&](int u, const std::unordered_map<int, int> &freq) {
    ans[u] = static_cast<int>(freq.size());
  };
  subtree_small_to_large_merge(adj, 0, init, merge, answer);
  assert((ans == std::vector<int>{3, 2, 2, 1, 1, 1}));
  return 0;
}
