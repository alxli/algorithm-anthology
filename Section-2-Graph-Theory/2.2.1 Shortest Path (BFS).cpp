/*

Description: Given an unweighted graph, traverse all reachable
nodes from a source node and determine the shortest path.

Complexity: O(V+E) on the number of vertices and edges.

Note: The line "for (q.push(start); !q.empty(); q.pop())"
is simply a mnemonic for looping a BFS with a FIFO queue.
This will not work as intended with a priority queue, such as in
Dijkstra's algorithm for solving weighted shortest paths

=~=~=~=~= Sample Input =~=~=~=~=
4 5
0 1
0 3
1 2
1 3
2 3
0 3

=~=~=~=~= Sample Output =~=~=~=~=
The shortest distance from 0 to 3 is 2.
Take the path: 0->1->3.

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAXN = 100, INF = 0x3f3f3f3f;
int dist[MAXN], pred[MAXN];
vector<int> adj[MAXN];

void bfs(int nodes, int start) {
  vector<bool> vis(nodes, false);
  for (int i = 0; i < nodes; i++) {
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

//Use the precomputed pred[] array to print the path
void print_path(int dest) {
  int i = 0, j = dest, path[MAXN];
  while (pred[j] != -1) j = path[++i] = pred[j];
  cout << "Take the path: ";
  while (i > 0) cout << path[i--] << "->";
  cout << dest << ".\n";
}

int main() {
  int nodes, edges, u, v, start, dest;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
  }
  cin >> start >> dest;
  bfs(nodes, start);
  cout << "The shortest distance from " << start;
  cout << " to " << dest << " is " << dist[dest] << ".\n";
  print_path(dest);
  return 0;
}
