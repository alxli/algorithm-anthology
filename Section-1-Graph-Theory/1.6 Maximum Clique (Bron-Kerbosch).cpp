/*

1.6 - Maximum Clique (Bron-Kerbosch Algorithm)

Description: Given an undirected graph, determine a subset
of the graph's vertices such that every pair of vertices
in the subset are connected by an edge, and that the
subset is as large as possible.

Complexity: O(3^(V/3)) where V is the number of vertices.

=~=~=~=~= Sample Input =~=~=~=~=
5 7
0 1
0 2
0 3
1 2
1 3
2 3
3 4

=~=~=~=~= Sample Output =~=~=~=~=
4

*/

#include <bitset>
#include <iostream>
using namespace std;

const int MAXN = 35;

typedef bitset<MAXN> bits;
bool graph[MAXN][MAXN];
int nodes, edges, a, b;

int max_clique(bits & curr, bits & pool, bits & excl, int depth) {
  if (pool.none() && excl.none()) return curr.count();
  int ans = 0, u;
  for (int v = 0; v < nodes; v++)
    if (pool[v] || excl[v]) u = v;
  for (int v = 0; v < nodes; v++) {
    if (!pool[v] || graph[u][v]) continue;
    bits ncurr, npool, nexcl;
    for (int i = 0; i < nodes; i++) ncurr[i] = curr[i];
    ncurr[v] = true;
    for (int j = 0; j < nodes; j++) {
      npool[j] = pool[j] && graph[v][j];
      nexcl[j] = excl[j] && graph[v][j];
    }
    ans = max(ans, max_clique(ncurr, npool, nexcl, depth + 1));
    pool[v] = false;
    excl[v] = true;
  }
  return ans;
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    graph[a][b] = graph[b][a] = true;
  }
  bits curr, excl, pool;
  for (int v = 0; v < nodes; v++) pool[v] = true;
  cout << max_clique(curr, pool, excl, 0) << "\n";
  return 0;
}
