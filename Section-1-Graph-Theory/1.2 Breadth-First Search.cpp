/*

1.2 - Breadth First Search

Description: Given an unweighted graph, traverse all reachable
nodes from a source node. Each branch is explored as deep as
possible before more branches are visited. DFS only uses as
much space as the length of the longest branch. When DFS'ing
recursively, the internal call-stack could overflow, so
sometimes it is safer to use an explicit stack data structure.

Complexity: O(number of edges) for explicit graphs traversed
without repetition. O(b^d) for implicit graphs with a branch
factor of b, searched to depth d.

Note: The line "for (q.push(start); !q.empty(); q.pop())"
is simply a fancy mnemonic for looping with a FIFO queue.
This will not work as intended with a priority queue,
such as in Dijkstra’s algorithm.

=~=~=~=~= Sample Input =~=~=~=~=
12 11 0
0 1
0 2
0 3
1 4
1 5
3 6
3 7
4 8
4 9
6 10
6 11

=~=~=~=~= Sample Output =~=~=~=~=
Nodes visited: 0 1 2 3 4 5 6 7 8 9 10 11

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAXN = 100;
int nodes, edges, start, a, b;
vector<bool> vis(MAXN);
vector<int> adj[MAXN];

int main() {
  cin >> nodes >> edges >> start;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  cout << "Nodes visited:";
  queue<int> q;
  for (q.push(start); !q.empty(); q.pop()) {
    int n = q.front();
    vis[n] = true;
    cout << " " << n;
    for (int j = 0; j < adj[n].size(); j++)
      if (!vis[adj[n][j]]) q.push(adj[n][j]);
  }
  return 0;
}
