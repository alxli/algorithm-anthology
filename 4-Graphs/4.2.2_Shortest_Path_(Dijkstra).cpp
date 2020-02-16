/*

Given a starting node in a weighted, directed graph with nonnegative weights
only, visit every connected node and determine the minimum distance to each such
node. Optionally, output the shortest path to a specific destination node using
the shortest-path tree from the predecessor array pred[]. dijkstra() applies to
a global, pre-populated adjacency list adj[] which must only consist of nodes
numbered with integers between 0 (inclusive) and the total number of nodes
(exclusive), as passed in the function argument.

Since std::priority_queue is by default a max-heap, we simulate a min-heap by
negating node distances before pushing them and negating them again after
popping them. Alternatively, the container can be declared with the following
template arguments (#include <functional> to access std::greater):
  priority_queue<pair<int, int>, vector<pair<int, int> >,
                 greater<pair<int, int> > > pq;

Dijkstra's algorithm may be modified to support negative edge weights by
allowing nodes to be re-visited (removing the visited array check in the inner
for-loop). This is known as the Shortest Path Faster Algorithm (SPFA), which has
a larger running time of O(n*m) on the number of nodes and edges respectively.
While it is as slow in the worst case as the Bellman-Ford algorithm, the SPFA
still tends to outperform in the average case.

Time Complexity:
- O(m log n) for dijkstra(), where m is the number of edges and n is the number
  of nodes.

Space Complexity:
- O(max(n, m)) for storage of the graph, where n is the number of nodes and m
  is the number of edges.
- O(n) auxiliary heap space for dijkstra().

*/

#include <queue>
#include <utility>
#include <vector>

const int MAXN = 100, INF = 0x3f3f3f3f;
std::vector<std::pair<int, int> > adj[MAXN];
int dist[MAXN], pred[MAXN];

void dijkstra(int nodes, int start) {
  std::vector<bool> visit(nodes, false);
  for (int i = 0; i < nodes; i++) {
    dist[i] = INF;
    pred[i] = -1;
  }
  dist[start] = 0;
  std::priority_queue<std::pair<int, int> > pq;
  pq.push(std::make_pair(0, start));
  while (!pq.empty()) {
    int u = pq.top().second;
    pq.pop();
    visit[u] = true;
    for (int j = 0; j < (int)adj[u].size(); j++) {
      int v = adj[u][j].first;
      if (visit[v]) {
        continue;
      }
      if (dist[v] > dist[u] + adj[u][j].second) {
        dist[v] = dist[u] + adj[u][j].second;
        pred[v] = u;
        pq.push(std::make_pair(-dist[v], v));
      }
    }
  }
}

/*** Example Usage and Output:

The shortest distance from 0 to 3 is 5.
Take the path: 0->1->2->3.

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
  int start = 0, dest = 3;
  adj[0].push_back(make_pair(1, 2));
  adj[0].push_back(make_pair(3, 8));
  adj[1].push_back(make_pair(2, 2));
  adj[1].push_back(make_pair(3, 4));
  adj[2].push_back(make_pair(3, 1));
  dijkstra(4, start);
  cout << "The shortest distance from " << start << " to " << dest << " is "
       << dist[dest] << "." << endl;
  print_path(dest);
  return 0;
}
