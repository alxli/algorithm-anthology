/*

Description: Given a flow network, find a flow from a single
source node to a single sink node that is maximized. Note
that in this implementation, the adjacency matrix cap[][]
will be modified by the function ford_fulkerson() after it's
been called. Make a back-up if you require it afterwards.

Complexity: O(V^2*|F|), where V is the number of
vertices and |F| is the magnitude of the max flow.

Real-valued capacities:
The Ford-Fulkerson algorithm is only optimal on graphs with
integer capacities; there exists certain real capacity inputs
for which it will never terminate. The Edmonds-Karp algorithm
is an improvement using BFS, supporting real number capacities.

=~=~=~=~= Sample Input =~=~=~=~=
6 8
0 1 3
0 2 3
1 2 2
1 3 3
2 4 2
3 4 1
3 5 2
4 5 3
0 5

=~=~=~=~= Sample Output =~=~=~=~=
5

*/

#include <algorithm> /* std::fill() */
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100, INF = 0x3f3f3f3f;
int nodes, source, sink, cap[MAXN][MAXN];
vector<bool> vis(MAXN);

int dfs(int u, int f) {
  if (u == sink) return f;
  vis[u] = true;
  for (int v = 0; v < nodes; v++) {
    if (!vis[v] && cap[u][v] > 0) {
      int df = dfs(v, min(f, cap[u][v]));
      if (df > 0) {
        cap[u][v] -= df;
        cap[v][u] += df;
        return df;
      }
    }
  }
  return 0;
}

int ford_fulkerson() {
  int max_flow = 0;
  for (;;) {
    fill(vis.begin(), vis.end(), false);
    int df = dfs(source, INF);
    if (df == 0) break;
    max_flow += df;
  }
  return max_flow;
}

int main() {
  int edges, u, v, capacity;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v >> capacity;
    cap[u][v] = capacity;
  }
  cin >> source >> sink;
  cout << ford_fulkerson() << "\n";
  return 0;
}
