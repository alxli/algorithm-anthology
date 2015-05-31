/*

1.8.2 - Strongly Connected Components (Tarjan's Algorithm)

Description: Determines the strongly connected components
from a given directed graph. The input is stored in an
adjacency list.

In this implementation, a vector is used to emulate a stack
for the sake of simplicity. One useful property of Tarjan’s
algorithm is that, while there is nothing special about the
ordering of nodes within each component, the resulting DAG
is produced in reverse topological order.

Complexity: O(V + E) on the number of vertices and edges.

=~=~=~=~= Sample Input =~=~=~=~=
8 14
0 1
1 2
1 4
1 5
2 3
2 6
3 2
3 7
4 0
4 5
5 6
6 5
7 3
7 6

=~=~=~=~= Sample Output =~=~=~=~=
Component 1: 5 6
Component 2: 7 3 2
Component 3: 4 1 0

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int nodes, edges, a, b, counter, ncomp;
int num[MAXN], low[MAXN];
vector<bool> vis(MAXN);
vector<int> adj[MAXN], stack;

void DFS(int a) {
  int b;
  low[a] = num[a] = ++counter;
  stack.push_back(a);
  for (int j = 0; j < adj[a].size(); j++) {
    b = adj[a][j];
    if (vis[b]) continue;
    if (num[b] == -1) {
      DFS(b);
      low[a] = min(low[a], low[b]);
    } else {
      low[a] = min(low[a], num[b]);
    }
  }
  if (num[a] != low[a]) return;
  cout << "Component " << ++ncomp << ":";
  do {
    vis[b = stack.back()] = true;
    stack.pop_back();
    cout << " " << b;
  } while (a != b);
  cout << "\n";
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  for (int i = 0; i < nodes; i++) {
    num[i] = low[i] = -1;
  }
  counter = ncomp = 0;
  for (int i = 0; i < nodes; i++)
    if (num[i] == -1) DFS(i);
  return 0;
} 
