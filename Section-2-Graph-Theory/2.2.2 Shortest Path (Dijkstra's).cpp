/*

Given an weighted, directed graph with nonnegative weights only,
traverse to every connected node and determine the shortest distance to
each. Optionally, output the shortest path to a specific destination
node using the predecessor array precomputed during the search.

This implementation negates node distances before adding them to the
priority_queue, since the standard library container is a max-heap by
default. This method is recommended over defining special comparators.
An alternative would be declaring the priority_queue with the following
template arguments (#include <functional> to access std::greater):
  priority_queue< pair<int, int>, vector<pair<int, int> >,
                  greater<pair<int, int> > > pq;

Dijkstra's algorithm may be easily modified to support negative edge
weights by allowing nodes to be re-visited (i.e. removing the first
if-statement in the inner for-loop). This is known as the Shortest Path
Faster Algorithm (SPFA). However, the worst case time complexity will
degenerate to that of the Bellman-Ford algorithm - O(nm) on the number
of nodes and edges respectively. In this case, the SPFA may still
outperform the Bellman-Ford algorithm in the average case.

Time Complexity: dijkstra() is O(m log n) where m is the number of
edges and n is the number of nodes. print_path() is O(n) on the number
of nodes in the shortest path to be printed.

Space Complexity: O(n) auxiliary on the number of nodes.

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAXN = 100, INF = 0x3f3f3f3f;
int dist[MAXN], pred[MAXN];
vector<pair<int, int> > adj[MAXN];

void dijkstra(int start) {
  vector<bool> vis(MAXN, false);
  for (int i = 0; i < MAXN; i++) {
    dist[i] = INF;
    pred[i] = -1;
  }
  int u, v;
  dist[start] = 0;
  priority_queue<pair<int, int> > pq;
  pq.push(make_pair(0, start));
  while (!pq.empty()) {
    u = pq.top().second;
    pq.pop();
    vis[u] = true;
    for (int j = 0; j < (int)adj[u].size(); j++) {
      if (vis[v = adj[u][j].first])
        continue;
      if (dist[v] > dist[u] + adj[u][j].second) {
        dist[v] = dist[u] + adj[u][j].second;
        pred[v] = u;
        pq.push(make_pair(-dist[v], v));
      }
    }
  }
}

void print_path(int dest) {
  int i = 0, j = dest, path[MAXN];
  while (pred[j] != -1)
    j = path[++i] = pred[j];
  cout << "Take the path: ";
  while (i > 0)
    cout << path[i--] << "->";
  cout << dest << ".\n";
}

/*** Example Usage

Sample Output:
The shortest distance from 0 to 3 is 5.
Take the path: 0->1->2->3.

***/

int main() {
  int start = 0, dest = 3;
  adj[0].push_back(make_pair(1, 2));
  adj[0].push_back(make_pair(3, 8));
  adj[1].push_back(make_pair(2, 2));
  adj[1].push_back(make_pair(3, 4));
  adj[2].push_back(make_pair(3, 1));
  dijkstra(start);
  cout << "The shortest distance from " << start;
  cout << " to " << dest << " is " << dist[dest] << ".\n";
  print_path(dest);
  return 0;
}
