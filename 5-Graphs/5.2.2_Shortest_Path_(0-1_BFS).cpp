/*

Given a starting node in a weighted graph whose edge weights are only 0 or 1,
compute the shortest distance to every reachable node. `zero_one_bfs()` applies
to a global, pre-populated adjacency list `adj[]` where each edge is stored as
`(neighbor, weight)`.

This is a specialized version of Dijkstra's algorithm. Because every relaxation
changes the distance by either $0$ or $1$, a deque maintains nodes in
nondecreasing distance order: push weight-0 relaxations to the front and
weight-1 relaxations to the back.

Time Complexity:
- O(max(n, m)) per call to `zero_one_bfs()`, where $n$ is the number of nodes and
  $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$
  is the number of edges.
- O(n) auxiliary deque space for `zero_one_bfs()`.

*/

#include <algorithm>
#include <deque>
#include <utility>
#include <vector>

const int MAXN = 100, INF = 0x3f3f3f3f;
std::vector<std::pair<int, int> > adj[MAXN];
int dist[MAXN], pred[MAXN];

void zero_one_bfs(int nodes, int start) {
  std::fill(dist, dist + nodes, INF);
  std::fill(pred, pred + nodes, -1);
  std::deque<int> dq;
  dist[start] = 0;
  dq.push_front(start);
  while (!dq.empty()) {
    int u = dq.front();
    dq.pop_front();
    for (int i = 0; i < (int)adj[u].size(); i++) {
      int v = adj[u][i].first;
      int w = adj[u][i].second;
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
  adj[u].push_back(make_pair(v, w));
}

int main() {
  add_edge(0, 1, 1);
  add_edge(0, 2, 0);
  add_edge(2, 1, 0);
  add_edge(1, 3, 1);
  add_edge(2, 3, 1);
  zero_one_bfs(4, 0);
  assert(dist[1] == 0);
  assert(dist[3] == 1);
  assert(pred[1] == 2);
  return 0;
}
