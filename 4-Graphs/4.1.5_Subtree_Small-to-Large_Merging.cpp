/*

Given a rooted tree, compute a mutable summary for every subtree by merging child summaries into one
kept summary. Small-to-large merging always keeps the largest child summary and absorbs smaller
summaries into it. Each stored item can move only into summaries at least twice as large as before,
so the total number of moved items is logarithmic per item.

This pattern is often called DSU on tree, although it is unrelated to the disjoint-set union data
structure used by Kruskal's algorithm. It is useful for subtree queries whose answer can be
maintained by inserting all items from child summaries into one mutable summary, such as distinct
colors, frequency modes, or other multiset-style statistics.

- `subtree_small_to_large_merge(adj, root, init, merge, answer)` visits the tree rooted at `root`,
  given by the bidirectional adjacency list `adj`, which must form a valid tree. The callback
  `init(u)` returns the initial summary for node `u`; `merge(big, small)` absorbs one child summary
  into another; and `answer(u, summary)` records the final subtree summary for `u`.

Summary objects must support `.size()`, and `merge(big, small)` should iterate over `small` and
insert into `big`.

Time Complexity:
- O(n log n) expected per call when `merge()` does expected O(1) work per moved item, where $n$ is
  the number of nodes.

Space Complexity:
- O(n) auxiliary stack space and O(n log n) auxiliary heap space in the worst case, since absorbed
  summaries remain stored.

*/

#include <cassert>
#include <unordered_map>
#include <vector>

template<typename Init, typename Merge, typename Answer>
void subtree_small_to_large_merge(
    const std::vector<std::vector<int>> &adj, int root, Init init, Merge merge, Answer answer
) {
  int n = static_cast<int>(adj.size());
  assert(0 <= root && root < n);
  std::vector<decltype(init(0))> summary(n);
  auto dfs = [&](auto &&dfs, int u, int p) -> int {
    int keep = u;
    summary[u] = init(u);
    for (int v : adj[u]) {
      if (v == p) {
        continue;
      }
      int child = dfs(dfs, v, u);
      if (summary[keep].size() < summary[child].size()) {
        std::swap(keep, child);
      }
      merge(summary[keep], summary[child]);
    }
    answer(u, summary[keep]);
    return keep;
  };
  dfs(dfs, root, -1);
}

/*** Example Usage ***/

using namespace std;

int main() {
  //          0:A
  //       /     |
  //     1:B    2:A
  //    /  |     |
  // 3:C  4:B   5:C
  vector<vector<int>> adj(6);
  auto add_edge = [&](int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  };
  add_edge(0, 1);
  add_edge(0, 2);
  add_edge(1, 3);
  add_edge(1, 4);
  add_edge(2, 5);
  vector<int> color{0, 1, 0, 2, 1, 2};
  vector<int> ans(6);
  using Summary = unordered_map<int, int>;
  auto init = [&](int u) {
    Summary freq;
    freq[color[u]] = 1;
    return freq;
  };
  auto merge = [](Summary &big, const Summary &small) {
    for (const auto &[c, cnt] : small) {
      big[c] += cnt;
    }
  };
  auto answer = [&](int u, const Summary &freq) { ans[u] = static_cast<int>(freq.size()); };
  subtree_small_to_large_merge(adj, 0, init, merge, answer);
  assert((ans == vector<int>{3, 2, 2, 1, 1, 1}));
  return 0;
}
