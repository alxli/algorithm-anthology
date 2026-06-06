/*

Given a starting node in a weighted directed graph, compute shortest paths even when some edge
weights are negative. `spfa()` applies to a global, pre-populated adjacency list `adj`, where each
edge is stored as `(neighbor, weight)`.

The Shortest Path Faster Algorithm is a queue-based optimization of Bellman-Ford. It is often fast
on benign inputs, but it still has Bellman-Ford's worst-case behavior and can be forced to run in
O(n*m). Prefer Dijkstra for nonnegative weights, and use SPFA mainly when negative edges are present
and the input is not adversarial.

`spfa()` returns `false` if it detects a reachable negative cycle, and returns `true` otherwise.

Time Complexity:
- O(n*m) in the worst case for `spfa()`, where $n$ is the number of nodes and $m$ is the number of
  edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary queue space for `spfa()`.

*/

#include <algorithm>
#include <climits>
#include <queue>
#include <utility>
#include <vector>

const long long INF = LLONG_MAX / 4;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<long long> dist;
std::vector<int> pred, relax_count;
std::vector<bool> in_queue;

bool spfa(int start) {
  int nodes = adj.size();
  dist.assign(nodes, INF);
  pred.assign(nodes, -1);
  relax_count.assign(nodes, 0);
  in_queue.assign(nodes, false);
  std::queue<int> q;
  dist[start] = 0;
  q.push(start);
  in_queue[start] = true;
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    in_queue[u] = false;
    for (auto &[v, w] : adj[u]) {
      if (dist[u] != INF && dist[v] > dist[u] + w) {
        dist[v] = dist[u] + w;
        pred[v] = u;
        if (!in_queue[v]) {
          q.push(v);
          in_queue[v] = true;
          if (++relax_count[v] >= nodes) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

void add_edge(int u, int v, int w) {
  adj[u].push_back({v, w});
}

int main() {
  int nodes = 4;
  adj.resize(nodes);
  add_edge(0, 1, 4);
  add_edge(0, 2, 5);
  add_edge(1, 2, -2);
  add_edge(2, 3, 3);
  assert(spfa(0));
  assert(dist[3] == 5);
  assert(pred[2] == 1);
  return 0;
}
