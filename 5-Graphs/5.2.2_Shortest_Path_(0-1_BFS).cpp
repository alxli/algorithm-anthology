/*

Given a starting node in a weighted graph whose edge weights are only 0 or 1, compute the shortest
distance to every reachable node. `zero_one_bfs()` applies to a global, pre-populated adjacency list
`adj` where each edge is stored as `(neighbor, weight)`.

This is a specialized version of Dijkstra's algorithm. Because every relaxation changes the distance
by either $0$ or $1$, a deque maintains nodes in nondecreasing distance order: push weight-0
relaxations to the front and weight-1 relaxations to the back.

Time Complexity:
- O(max(n, m)) per call to `zero_one_bfs()`, where $n$ is the number of nodes and $m$ is the number
  of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary deque space for `zero_one_bfs()`.

*/

#include <algorithm>
#include <climits>
#include <deque>
#include <utility>
#include <vector>

const int INF = INT_MAX / 2;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<int> dist, pred;

void zero_one_bfs(int start) {
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
  adj.resize(nodes);
  add_edge(0, 1, 1);
  add_edge(0, 2, 0);
  add_edge(2, 1, 0);
  add_edge(1, 3, 1);
  add_edge(2, 3, 1);
  zero_one_bfs(0);
  assert(dist[1] == 0);
  assert(dist[3] == 1);
  assert(pred[1] == 2);
  return 0;
}
