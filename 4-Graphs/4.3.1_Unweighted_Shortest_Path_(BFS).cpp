/*

Given one or more starting nodes in an unweighted, directed graph, visit every reachable node and
determine its minimum distance from any start. Breadth-first search explores nodes in order of
increasing distance using a queue, so the first time a node is reached is via a shortest path.
Initializing the queue with several distance-$0$ sources gives multi-source BFS without changing the
traversal. Optionally, reconstruct a shortest path using the predecessor vector `pred`.

- `bfs(starts)` populates `dist` and `pred` from every node in `starts`, assigning each node its
  minimum distance from any start. The global, pre-populated adjacency list `adj` uses its indices
  as nodes and must contain only valid node indices. Pass a singleton vector for single-source BFS.
- `get_path(dest)` returns the path from a nearest starting node to `dest`, or an empty vector if
  `dest` is unreachable, using the state left by the most recent call to `bfs()`.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
a nearest start to `v`, or $-1$ if `v` is a starting node or unreachable. Follow `pred` backward
from the destination to its starting node, then reverse that sequence to recover the path.

Time Complexity:
- O(max(n, m, s)) per call, where $n$ is the number of nodes, $m$ is the number of edges, and $s$ is
  the number of supplied starting nodes. For single-source BFS, $s = 1$.
- O(p) per call to `get_path()`, where $p$ is the number of nodes in the returned path.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary.
- O(p) for the path returned by `get_path()`.

*/

#include <algorithm>
#include <climits>
#include <queue>
#include <vector>

const int INF = INT_MAX / 2;
std::vector<std::vector<int>> adj;
std::vector<int> dist, pred;

void bfs(const std::vector<int> &starts) {
  int n = static_cast<int>(adj.size());
  dist.assign(n, INF);
  pred.assign(n, -1);
  std::queue<int> q;
  for (int start : starts) {
    if (dist[start] == 0) {
      continue;
    }
    dist[start] = 0;
    q.push(start);
  }
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int v : adj[u]) {
      if (dist[v] != INF) {
        continue;
      }
      dist[v] = dist[u] + 1;
      pred[v] = u;
      q.push(v);
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
  // 0 --> 1 --> 2
  //       |     |
  //       v     |
  //       3 <---+
  adj.assign(4, {});
  adj[0].push_back(1);
  adj[1].push_back(2);
  adj[1].push_back(3);
  adj[2].push_back(3);
  bfs(vector<int>{0});
  assert((dist == vector<int>{0, 1, 2, 2}) && (pred == vector<int>{-1, 0, 1, 1}));
  assert((get_path(3) == vector<int>{0, 1, 3}));

  bfs(vector<int>{0, 2});
  assert((dist == vector<int>{0, 1, 0, 1}) && (pred == vector<int>{-1, 0, -1, 2}));
  assert((get_path(3) == vector<int>{2, 3}));
  return 0;
}
