/*

Given an unweighted, directed graph, traverse to every connected node
and determine the shortest distance to each. Optionally, output the
shortest path to a specific destination node using the predecessor
array precomputed during the search.

Time Complexity: bfs() is O(n) on the number of edges to compute the
shortest distance to each node. print_path() is O(n) on the number of
nodes in the shortest path to be printed.

Space Complexity: O(n) auxiliary on the number of nodes.

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAXN = 100, INF = 0x3f3f3f3f;
int dist[MAXN], pred[MAXN];
vector<int> adj[MAXN];

void bfs(int start) {
  vector<bool> vis(MAXN, false);
  for (int i = 0; i < MAXN; i++) {
    dist[i] = INF;
    pred[i] = -1;
  }
  int u, v, d;
  queue<pair<int, int> > q;
  q.push(make_pair(start, 0));
  while (!q.empty()) {
    u = q.front().first;
    d = q.front().second;
    q.pop();
    vis[u] = true;
    for (int j = 0; j < (int)adj[u].size(); j++) {
      if (vis[v = adj[u][j]]) continue;
      dist[v] = d + 1;
      pred[v] = u;
      q.push(make_pair(v, d + 1));
    }
  }
}

void print_path(int dest) {
  int i = 0, j = dest, path[MAXN];
  while (pred[j] != -1) j = path[++i] = pred[j];
  cout << "Take the path: ";
  while (i > 0) cout << path[i--] << "->";
  cout << dest << ".\n";
}

/*** Example Usage

Sample Output:
The shortest distance from 0 to 3 is 2.
Take the path: 0->1->3.

***/

int main() {
  int start = 0, dest = 3;
  adj[0].push_back(1);
  adj[0].push_back(3);
  adj[1].push_back(2);
  adj[1].push_back(3);
  adj[2].push_back(3);
  adj[0].push_back(3);
  bfs(start);
  cout << "The shortest distance from " << start;
  cout << " to " << dest << " is " << dist[dest] << ".\n";
  print_path(dest);
  return 0;
}
