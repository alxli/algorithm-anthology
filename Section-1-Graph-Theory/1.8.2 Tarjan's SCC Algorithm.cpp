/*

1.8.2 - Tarjan's Algorithm for Strongly Connected Components

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
1 2
2 3
2 5
2 6
3 4
3 7
4 3
4 8
5 1
5 6
6 7
7 6
8 4
8 7

=~=~=~=~= Sample Output =~=~=~=~=
Component 1: 6 7
Component 2: 8 4 3
Component 3: 5 2 1

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int nodes, edges, a, b, counter, ncomp;
int num[MAXN], low[MAXN];
bool visit[MAXN] = {0};
vector<int> adj[MAXN], stack;

void DFS(int a) {
  int b;
  low[a] = num[a] = ++counter;
  stack.push_back(a);
  for (int j = 0; j < adj[a].size(); j++) {
    b = adj[a][j];
    if (visit[b]) continue;
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
    visit[b = stack.back()] = true;
    stack.pop_back();
    cout << " " << (b + 1);
  } while (a != b);
  cout << "\n";
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[--a].push_back(--b);
  }
  for (int i = 0; i < nodes; i++) {
    num[i] = low[i] = -1;
    visit[i] = false;
  }
  counter = ncomp = 0;
  for (int i = 0; i < nodes; i++)
    if (num[i] == -1) DFS(i);
  return 0;
} 
