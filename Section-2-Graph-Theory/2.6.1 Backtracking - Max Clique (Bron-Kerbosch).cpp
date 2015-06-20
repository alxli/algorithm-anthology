/*

2.6.1 - Backtracking: Maximum Clique (Bron-Kerbosch Algorithm)

Description: Given an undirected graph, determine a subset of
the graph's vertices such that every pair of vertices in the
subset are connected by an edge, and that the subset is as
large as possible. For the weighted version, each vertex is
assigned a weight and the objective is to find the clique in
the graph that has maximum total weight.

Complexity: O(3^(V/3)) where V is the number of vertices.

=~=~=~=~= Sample Input =~=~=~=~=
5 8
0 1
0 2
0 3
1 2
1 3
2 3
3 4
4 2
10 20 30 40 50

=~=~=~=~= Sample Output =~=~=~=~=
Max unweighted clique: 4
Max weighted clique: 120

*/

#include <algorithm> /* std::fill(), std::max() */
#include <bitset>
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 35;
typedef bitset<MAXN> bits;
typedef unsigned long long ull;

int w[MAXN];
bool adj[MAXN][MAXN];

int rec(int nodes, bits & curr, bits & pool, bits & excl) {
  if (pool.none() && excl.none()) return curr.count();
  int ans = 0, u;
  for (int v = 0; v < nodes; v++)
    if (pool[v] || excl[v]) u = v;
  for (int v = 0; v < nodes; v++) {
    if (!pool[v] || adj[u][v]) continue;
    bits ncurr, npool, nexcl;
    for (int i = 0; i < nodes; i++) ncurr[i] = curr[i];
    ncurr[v] = true;
    for (int j = 0; j < nodes; j++) {
      npool[j] = pool[j] && adj[v][j];
      nexcl[j] = excl[j] && adj[v][j];
    }
    ans = max(ans, rec(nodes, ncurr, npool, nexcl));
    pool[v] = false;
    excl[v] = true;
  }
  return ans;
}

int bron_kerbosch(int nodes) {
  bits curr, excl, pool;
  pool.flip();
  return rec(nodes, curr, pool, excl);
}

//This is a fast implementation using bitmasks.
//Precondition: the number of nodes must be less than 64.
int bron_kerbosch_weighted(int nodes, ull g[], ull curr, ull pool, ull excl) {
  if (pool == 0 && excl == 0) {
    int res = 0, u = __builtin_ctzll(curr);
    while (u < nodes) {
      res += w[u];
      u += __builtin_ctzll(curr >> (u + 1)) + 1;
    }
    return res;
  }
  if (pool == 0) return -1;
  int res = -1, pivot = __builtin_ctzll(pool | excl);
  ull z = pool & ~g[pivot];
  int u = __builtin_ctzll(z);
  while (u < nodes) {
    res = max(res, bron_kerbosch_weighted(nodes, g, curr | (1LL << u),
                                          pool & g[u], excl & g[u]));
    pool ^= 1LL << u;
    excl |= 1LL << u;
    u += __builtin_ctzll(z >> (u + 1)) + 1;
  }
  return res;
}

int bron_kerbosch_weighted(int nodes) {
  ull g[nodes];
  for (int i = 0; i < nodes; i++) {
    g[i] = 0;
    for (int j = 0; j < nodes; j++)
      if (adj[i][j]) g[i] |= 1LL << j;
  }
  return bron_kerbosch_weighted(nodes, g, 0, (1LL << nodes) - 1, 0);
}

int main() {
  int nodes, edges, u, v;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u][v] = adj[v][u] = true;
  }
  for (int i = 0; i < nodes; i++) cin >> w[i];
  cout << "Max unweighted clique: ";
  cout << bron_kerbosch(nodes) << "\n";
  cout << "Max weighted clique: ";
  cout << bron_kerbosch_weighted(nodes) << "\n";
  return 0;
}
