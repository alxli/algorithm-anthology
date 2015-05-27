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
12 11 1
1 2
1 3
1 4
2 5
2 6
4 7
4 8
5 9
5 10
7 11
7 12 

=~=~=~=~= Sample Output =~=~=~=~=
Nodes visited:
1 2 3 4 5 6 7 8 9 10 11 12

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAXN = 100;
int nodes, edges, start, a, b;
bool visit[MAXN+1] = {0};
vector<int> adj[MAXN+1];

int main() {
  cin >> nodes >> edges >> start;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  cout << "Nodes visited:\n";
  queue<int> q;
  for (q.push(start); !q.empty(); q.pop()) {
    int n = q.front();
    visit[n] = true;
    cout << " " << n;
    for (int j = 0; j < adj[n].size(); j++)
      if (!visit[adj[n][j]]) q.push(adj[n][j]);
  }
  return 0;
}
