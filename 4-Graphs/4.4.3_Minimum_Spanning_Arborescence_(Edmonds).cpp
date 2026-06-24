/*

A spanning arborescence rooted at a node `root` of a weighted, directed graph is a set of edges
forming a tree in which every other node is reachable from the root along directed edges, that is,
every node except the root has exactly one incoming edge. The minimum spanning arborescence
minimizes the total edge weight, and is the directed analog of the minimum spanning tree.

Edmonds' (a.k.a. the Chu-Liu/Edmonds) algorithm computes the arborescence by first selecting, for
each non-root node, its cheapest incoming edge. If these choices form no cycle, they already
constitute the answer. Otherwise each cycle is contracted into a single supernode, and every edge
entering the cycle has its weight reduced by the cost of the edge it would replace inside the cycle.
Solving the problem on the contracted graph and expanding the cycles back yields the optimum. The
implementation below repeats this contraction in rounds, accumulating the selected weights, until no
cycle remains.

- `directed_mst(n, root, edges)` returns the total weight of the minimum spanning arborescence
  rooted at `root` over `n` nodes numbered $[0, `n`)$, or $-1$ if some node is unreachable from the
  root (no arborescence exists). Edges are given as $(`from`, `to`, `weight`)$ triples; parallel edges
  and self-loops are allowed, with self-loops simply ignored.

Time Complexity:
- O(n * m) per call, where $n$ is the number of nodes and $m$ is the number of edges.

Space Complexity:
- O(n + m) auxiliary heap space.

*/

#include <cstdint>
#include <vector>

struct Edge {
  int u, v;
  int64_t weight;
};

int64_t directed_mst(int n, int root, std::vector<Edge> edges) {
  const int64_t INF = INT64_MAX / 4;
  int64_t result = 0;
  while (true) {
    std::vector<int64_t> min_in(n, INF);
    std::vector<int> pre(n, -1);
    for (const Edge &e : edges) {
      if (e.u != e.v && e.weight < min_in[e.v]) {
        min_in[e.v] = e.weight;
        pre[e.v] = e.u;
      }
    }
    for (int v = 0; v < n; v++) {
      if (v != root && min_in[v] == INF) {
        return -1;  // Node v has no incoming edge, so it is unreachable.
      }
    }
    // Identify cycles formed by the chosen incoming edges, labeling each with an ID in `comp`.
    int cycles = 0;
    std::vector<int> comp(n, -1), seen(n, -1);
    min_in[root] = 0;
    for (int v = 0; v < n; v++) {
      result += min_in[v];
      int u = v;
      while (seen[u] != v && comp[u] == -1 && u != root) {
        seen[u] = v;
        u = pre[u];
      }
      if (u != root && comp[u] == -1) {
        for (int w = pre[u]; w != u; w = pre[w]) {
          comp[w] = cycles;
        }
        comp[u] = cycles++;
      }
    }
    if (cycles == 0) {
      break;  // No cycle remains; the selected edges form the arborescence.
    }
    // Contract each cycle into a supernode and reduce the weights of edges entering it.
    for (int v = 0; v < n; v++) {
      if (comp[v] == -1) {
        comp[v] = cycles++;
      }
    }
    for (Edge &e : edges) {
      int u = comp[e.u], v = comp[e.v];
      if (u != v) {
        e.weight -= min_in[e.v];
      }
      e.u = u;
      e.v = v;
    }
    n = cycles;
    root = comp[root];
  }
  return result;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Root 0. Cheapest incoming edges 1<-0 (1), 2<-1 (2), 3<-2 (3) form a tree of weight 6.
  vector<Edge> edges{
      {0, 1, 1}, {0, 2, 5}, {1, 2, 2}, {2, 3, 3}, {3, 1, 4},
  };
  assert(directed_mst(4, 0, edges) == 6);

  // Node 3 is unreachable from root 0.
  vector<Edge> disconnected{{0, 1, 1}, {1, 2, 1}};
  assert(directed_mst(4, 0, disconnected) == -1);
  return 0;
}
