/*

Given a starting node in an unweighted, directed graph, visit every connected node and determine the
minimum distance to each such node. Breadth-first search explores nodes in order of increasing
distance using a queue, so the first time a node is reached is via a shortest path. Optionally,
output the shortest path to a specific destination node using the shortest-path tree from the
predecessor vector `pred`.

- `bfs(start)` populates `dist` and `pred` for a global, pre-populated adjacency list `adj` which
  must consist of nodes numbered from 0 to `adj.size() - 1`.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(max(n, m)) per call to `bfs()`, where $n$ is the number of nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary heap space for `bfs()`.

*/

#include <algorithm>
#include <climits>
#include <queue>
#include <vector>

const int INF = INT_MAX / 2;
std::vector<std::vector<int>> adj;
std::vector<int> dist, pred;

void bfs(int start) {
  int nodes = adj.size();
  dist.assign(nodes, INF);
  pred.assign(nodes, -1);
  std::queue<int> q;
  dist[start] = 0;
  q.push(start);
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

/*** Example Usage and Output:

The shortest distance from 0 to 3 is 2.
Take the path: 0->1->3.

***/

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
  int nodes = 4, start = 0, dest = 3;
  adj.assign(nodes, {});
  adj[0].push_back(1);
  adj[1].push_back(2);
  adj[1].push_back(3);
  adj[2].push_back(3);
  bfs(start);
  cout << "The shortest distance from " << start << " to " << dest << " is " << dist[dest] << "."
       << endl;
  print_path(dest);
  return 0;
}
