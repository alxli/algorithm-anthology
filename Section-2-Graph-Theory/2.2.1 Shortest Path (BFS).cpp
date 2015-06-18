/*

2.2.1 - Shortest Path (Breadth First Search)

Description: Given an unweighted graph, traverse all reachable
nodes from a source node and determine the shortest path.

Complexity: O(number of edges) for explicit graphs traversed
without repetition. O(b^d) for implicit graphs with a branch
factor of b, searched to depth d.

Note: The line "for (q.push(start); !q.empty(); q.pop())"
is simply a fancy mnemonic for looping with a FIFO queue.
This will not work as intended with a priority queue, such as in
Dijkstra’s algorithm for solving weighted shortest paths

=~=~=~=~= Sample Input =~=~=~=~=
4 5
0 1
0 3
1 2
1 3
2 3
0 3

=~=~=~=~= Sample Output =~=~=~=~=
The shortest distance from 0 to 3 is 1.

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAXN = 100;
int nodes, edges, a, b, start, dest;
vector<bool> vis(MAXN);
vector<int> adj[MAXN];

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  cin >> start >> dest;
  queue<pair<int, int> > q;
  q.push(make_pair(start, 0));
  while (!q.empty()) {
    a = q.front().first;
    int d = q.front().second;
    if (a == dest) {
      cout << "The shortest distance from " << start;
      cout << " to " << dest << " is " << d << ".\n";
      break;
    }
    q.pop();
    vis[a] = true;
    for (int j = 0; j < adj[a].size(); j++) {
      b = adj[a][j];
      if (!vis[b]) q.push(make_pair(b, d + 1));
    }
  }
  return 0;
}
