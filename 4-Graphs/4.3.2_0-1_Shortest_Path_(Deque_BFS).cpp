/*

Given a starting node in a weighted graph whose edge weights are only $0$ or $1$, compute the
shortest distance to every reachable node. Optionally, output the shortest path to a specific
destination node using the shortest-path tree from the predecessor array `pred`.

0-1 BFS is a specialized version of Dijkstra's algorithm. Because every relaxation changes the
distance by either $0$ or $1$, a deque maintains nodes in nondecreasing distance order: push
weight-0 relaxations to the front and weight-1 relaxations to the back.

- `bfs_zero_one(start)` populates `dist` and `pred` for a global, pre-populated adjacency list `adj`
  which must consist of nodes numbered [$0$, `n`), where `n` is `adj.size()`. Each edge is stored as
  (`neighbor`, `weight`), where `weight` is either $0$ or $1$.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(max(n, m)) per call, where $n$ is the number of nodes and $m$ is the number of edges.

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
  int n = static_cast<int>(adj.size());
  dist.assign(n, INF);
  pred.assign(n, -1);
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

/*** Example Usage and Output:

The shortest distance from 0 to 3 is 1.
Take the path: 0->2->3.

***/

#include <iostream>
using namespace std;

void add_edge(int u, int v, int w) {
  adj[u].emplace_back(v, w);
}

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
  int start = 0, dest = 3;
  adj.assign(4, {});
  add_edge(0, 1, 1);
  add_edge(0, 2, 0);
  add_edge(2, 1, 0);
  add_edge(1, 3, 1);
  add_edge(2, 3, 1);
  bfs_zero_one(start);
  cout << "The shortest distance from " << start << " to " << dest << " is " << dist[dest] << ".\n";
  print_path(dest);
  return 0;
}
