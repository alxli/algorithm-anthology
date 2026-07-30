/*

Given one or more starting nodes in a weighted graph whose edge weights are only $0$ or $1$, compute
the shortest distance from any start to every reachable node. Optionally, reconstruct the shortest
path to a specific destination node using the shortest-path tree from the predecessor array `pred`.

0-1 BFS is a specialized version of Dijkstra's algorithm. Because every relaxation changes the
distance by either $0$ or $1$, a deque maintains nodes in nondecreasing distance order: push
weight-0 relaxations to the front and weight-1 relaxations to the back.

- `bfs_zero_one(starts)` populates `dist` and `pred` for shortest paths from the nodes in `starts`,
  assigning each node its minimum distance from any start. The global, pre-populated adjacency list
  `adj` uses its indices as nodes. Each edge is stored as (`neighbor`, `weight`),
  where `weight` is either $0$ or $1$. Pass a singleton vector for a single source.
- `get_path(dest)` returns the path from a nearest starting node to `dest`, or an empty vector if
  `dest` is unreachable, using the state left by the most recent call to `bfs_zero_one()`.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
a nearest start to `v`, or $-1$ if `v` is a starting node or unreachable. Follow `pred` backward
from the destination to its starting node, then reverse that sequence to recover the path.

Time Complexity:
- O(max(n, m, s)) per call, where $n$ is the number of nodes, $m$ is the number of edges, and $s$ is
  the number of supplied starting nodes. For a single source, $s = 1$.
- O(p) per call to `get_path()`, where $p$ is the number of nodes in the returned path.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary deque space.
- O(p) for the path returned by `get_path()`.

*/

#include <algorithm>
#include <climits>
#include <deque>
#include <utility>
#include <vector>

const int INF = INT_MAX / 2;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<int> dist, pred;

void bfs_zero_one(const std::vector<int> &starts) {
  int n = static_cast<int>(adj.size());
  dist.assign(n, INF);
  pred.assign(n, -1);
  std::deque<int> dq;
  for (int start : starts) {
    if (dist[start] == 0) {
      continue;
    }
    dist[start] = 0;
    dq.push_back(start);
  }
  while (!dq.empty()) {
    int u = dq.front();
    dq.pop_front();
    for (auto [v, w] : adj[u]) {
      if (dist[v] > dist[u] + w) {
        dist[v] = dist[u] + w;
        pred[v] = u;
        if (w == 0) {
          dq.push_front(v);
        } else {
          dq.push_back(v);
        }
      }
    }
  }
}

std::vector<int> get_path(int dest) {
  if (dist[dest] == INF) {
    return {};
  }
  std::vector<int> path;
  for (int v = dest; v != -1; v = pred[v]) {
    path.push_back(v);
  }
  std::reverse(path.begin(), path.end());
  return path;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  //         w=1
  //     0 -------> 1
  //     |        ^ |
  // w=0 |    w=0   | w=1
  //     v  /       v
  //     2 -------> 3
  //         w=1
  adj.assign(4, {});
  adj[0].emplace_back(1, 1);
  adj[0].emplace_back(2, 0);
  adj[2].emplace_back(1, 0);
  adj[1].emplace_back(3, 1);
  adj[2].emplace_back(3, 1);
  int start = 0, dest = 3;
  bfs_zero_one(vector<int>{start});
  assert(dist[dest] == 1);
  assert((get_path(dest) == vector<int>{0, 2, 3}));

  bfs_zero_one(vector<int>{1, 0});
  assert(dist[3] == 1);
  assert((get_path(3) == vector<int>{1, 3}));
  return 0;
}
