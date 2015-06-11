/*

1.5.1 - Maximum Flow (Ford-Fulkerson Algorithm)

Description: Given a flow network, find a flow from a single
source node to a single sink node that is maximized. Note
that in this implementation, the adjacency matrix cap[][] will be
modified by the function ford_fulkerson() after it's been called.

Complexity: O(V^2*|max_flow|), where V is the number of vertices.

Comparison with Edmonds-Karp Algorithm:
The Ford-Fulkerson algorithm is only optimal on graphs with integer
capacities; there exists certain real-valued inputs for which it
will never terminate. The Edmonds-Karp algorithm is an improvement
using a breadth-first search, supporting real-valued capacities.

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

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100, INF = 0x3f3f3f3f;
int nodes, source, sink, cap[MAXN][MAXN];
bool vis[MAXN];

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
    fill(vis, vis + MAXN, false);
    int df = dfs(source, INF);
    if (df == 0) break;
    max_flow += df;
  }
  return max_flow;
}

int main() {
  int edges, a, b, capacity;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> capacity;
    cap[a][b] = capacity;
  }
  cin >> source >> sink;
  cout << ford_fulkerson() << "\n";
  return 0;
}
