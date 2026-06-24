/*

Given a starting node in a weighted directed graph, compute shortest paths even when some edge
weights are negative. The Shortest Path Faster Algorithm (SPFA) is a queue-based optimization of
Bellman-Ford: instead of relaxing every edge in every round, it keeps a queue of nodes whose
distances have improved and relaxes only their outgoing edges. It is often fast on benign inputs,
but it still has Bellman-Ford's worst-case behavior and can be forced to run in O(n*m). Prefer
Dijkstra for nonnegative weights, and use SPFA mainly when negative edges are present and the input
is not adversarial.

- `spfa(start)` populates `dist` and `pred` for a global, pre-populated adjacency list `adj` which
  must consist of nodes numbered $[0, `n`)$, where `n` is `adj.size()`. Each edge is stored as
  $(`neighbor`, `weight`)$. The function returns `false` if it detects a reachable negative cycle, and
  returns `true` otherwise.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(n*m) in the worst case for `spfa()`, where $n$ is the number of nodes and $m$ is the number of
  edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary queue space for `spfa()`.

*/

#include <algorithm>
#include <cstdint>
#include <queue>
#include <utility>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<int64_t> dist;
std::vector<int> pred;

bool spfa(int start) {
  int n = static_cast<int>(adj.size());
  dist.assign(n, INF);
  pred.assign(n, -1);
  std::vector<int> relax_count(n, 0);
  std::vector<char> in_queue(n, false);
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
          if (++relax_count[v] >= n) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

/*** Example Usage and Output:

The shortest distance from 0 to 3 is 5.
Take the path: 0->1->2->3.

***/

#include <cassert>
#include <iostream>
using namespace std;

void print_path(int dest) {
  vector<int> path;
  for (int j = dest; pred[j] != -1; j = pred[j]) {
    path.push_back(pred[j]);
  }
  cout << "Take the path: ";
  while (!path.empty()) {
    cout << path.back() << "->";
    path.pop_back();
  }
  cout << dest << "." << endl;
}

int main() {
  //        w=4
  //     0 ----> 1
  //     |      /
  // w=5 |    / w=-2
  //     |  /
  //     v v   w=3
  //     2 ---------> 3
  adj.assign(4, {});
  adj[0].emplace_back(1, 4);
  adj[0].emplace_back(2, 5);
  adj[1].emplace_back(2, -2);
  adj[2].emplace_back(3, 3);
  int start = 0, dest = 3;
  spfa(start);
  assert(dist[dest] == 5);
  assert(pred[dest] == 2 && pred[2] == 1);
  cout << "The shortest distance from " << start << " to " << dest << " is " << dist[dest] << ".\n";
  print_path(dest);
  return 0;
}
