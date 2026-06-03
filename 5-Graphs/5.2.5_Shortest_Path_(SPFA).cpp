/*

Given a starting node in a weighted directed graph, compute shortest paths even when some edge
weights are negative. `spfa()` applies to a global, pre-populated adjacency list `adj[]`, where each
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
#include <queue>
#include <utility>
#include <vector>

const int MAXN = 100, INF = 0x3f3f3f3f;
std::vector<std::pair<int, int> > adj[MAXN];
int dist[MAXN], pred[MAXN], relax_count[MAXN];
bool in_queue[MAXN];

bool spfa(int nodes, int start) {
  std::fill(dist, dist + nodes, INF);
  std::fill(pred, pred + nodes, -1);
  std::fill(relax_count, relax_count + nodes, 0);
  std::fill(in_queue, in_queue + nodes, false);
  std::queue<int> q;
  dist[start] = 0;
  q.push(start);
  in_queue[start] = true;
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    in_queue[u] = false;
    for (int i = 0; i < (int)adj[u].size(); i++) {
      int v = adj[u][i].first;
      int w = adj[u][i].second;
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
  adj[u].push_back(make_pair(v, w));
}

int main() {
  add_edge(0, 1, 4);
  add_edge(0, 2, 5);
  add_edge(1, 2, -2);
  add_edge(2, 3, 3);
  assert(spfa(4, 0));
  assert(dist[3] == 5);
  assert(pred[2] == 1);
  return 0;
}
