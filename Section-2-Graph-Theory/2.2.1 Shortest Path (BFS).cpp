/*

Given a starting node in an unweighted, directed graph, traverse to every
connected node and determine the minimum distance to each such node. Optionally,
output the shortest path to a specific destination node using the shortest-path
tree from the predecessor array pred[]. bfs() applies to a global, pre-populated
adjacency list adj[] which must only consist of nodes numbered with integers
between 0 (inclusive) and the total number of nodes (exclusive), as passed in
the function argument.

Time Complexity: O(n) on the number of nodes.
Space Complexity: O(n) auxiliary on the number of nodes.

*/

#include <queue>
#include <utility>
#include <vector>

const int MAXN = 100, INF = 0x3f3f3f3f;
std::vector<int> adj[MAXN];
int dist[MAXN], pred[MAXN];

void bfs(int nodes, int start) {
  std::vector<bool> vis(nodes, false);
  for (int i = 0; i < nodes; i++) {
    dist[i] = INF;
    pred[i] = -1;
  }
  int u, v, d;
  std::queue<std::pair<int, int> > q;
  q.push(std::make_pair(start, 0));
  while (!q.empty()) {
    u = q.front().first;
    d = q.front().second;
    q.pop();
    vis[u] = true;
    for (int j = 0; j < (int)adj[u].size(); j++) {
      v = adj[u][j];
      if (vis[v])
        continue;
      dist[v] = d + 1;
      pred[v] = u;
      q.push(std::make_pair(v, d + 1));
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
  int start = 0, dest = 3;
  adj[0].push_back(1);
  adj[0].push_back(3);
  adj[1].push_back(2);
  adj[1].push_back(3);
  adj[2].push_back(3);
  adj[0].push_back(3);
  bfs(4, start);
  cout << "The shortest distance from " << start;
  cout << " to " << dest << " is " << dist[dest] << "." << endl;
  print_path(dest);
  return 0;
}
