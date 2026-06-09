/*

Given a starting node in a weighted graph whose edge weights are only 0 or 1, compute the shortest
distance to every reachable node.

- `bfs_zero_one(start)` populates `dist` and `pred` for a global, pre-populated adjacency list `adj`
  which must consist of nodes numbered from 0 to `adj.size() - 1`. Each edge is stored as
  `(neighbor, weight)`, where `weight` is either 0 or 1.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

This is a specialized version of Dijkstra's algorithm. Because every relaxation changes the distance
by either $0$ or $1$, a deque maintains nodes in nondecreasing distance order: push weight-0
relaxations to the front and weight-1 relaxations to the back.

Time Complexity:
- O(max(n, m)) per call to `bfs_zero_one()`, where $n$ is the number of nodes and $m$ is the number
  of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary deque space for `bfs_zero_one()`.

*/

#include <algorithm>
#include <climits>
#include <deque>
#include <utility>
#include <vector>

const int INF = INT_MAX / 2;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<int> dist, pred;

void bfs_zero_one(int start) {
  int nodes = adj.size();
  dist.assign(nodes, INF);
  pred.assign(nodes, -1);
  std::deque<int> dq;
  dist[start] = 0;
  dq.push_front(start);
  while (!dq.empty()) {
    int u = dq.front();
    dq.pop_front();
    for (auto &[v, w] : adj[u]) {
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

/*** Example Usage ***/

#include <cassert>
using namespace std;

void add_edge(int u, int v, int w) {
  adj[u].push_back({v, w});
}

int main() {
  int nodes = 4;
  adj.assign(nodes, {});
  add_edge(0, 1, 1);
  add_edge(0, 2, 0);
  add_edge(2, 1, 0);
  add_edge(1, 3, 1);
  add_edge(2, 3, 1);
  bfs_zero_one(0);
  assert(dist[1] == 0);
  assert(dist[3] == 1);
  assert(pred[1] == 2);
  return 0;
}
