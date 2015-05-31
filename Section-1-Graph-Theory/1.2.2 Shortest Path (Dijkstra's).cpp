/*

1.2.2 - Dijkstra's Algorithm (Single Source Shortest Path)

Complexity: The simplest version runs in O(E+V^2) where V is the number
of vertices and E is the number of edges. Using an adjacency list and
priority queue (internally a binary heap), the implementation here is
Θ((E+V) log(V)), dominated by Θ(E log(V)) if the graph is connected.

Implementation Notes: The graph is stored using an adjacency list.
This implementation negates distances before adding them to the 
priority queue, since the container is a max-heap by default. This
method is suggested in contests because it is easier than defining
special comparators. An alternative would be declaring the queue
with template parameters (obviously it is ugly):
  priority_queue< pair<int,int>, vector< pair<int,int> >, greater< pair<int,int> > > pq;
If the path is to be computed for only a single pair of nodes, one
may break out of the loop as soon as the destination is reached,
by inserting the line "if (a == dest) break;" after the line "pq.pop();"

Shortest Path Faster Algorithm: The code for Dijkstra’s algorithm
here can be easily modified to become the Shortest Path Faster
Algorithm (SPFA) by simply commenting out "visit[a] = true;" and changing
the priority queue to a FIFO queue like BFS. SPFA is a faster version of
the Bellman-Ford algorithm, working on negative path lengths (whereas
Dijkstra’s cannot). However, certain graphs can be crafted to make
the SPFA very slow.

=~=~=~=~= Sample Input =~=~=~=~=
4 5
0 1 2
0 3 8
1 2 2
1 3 4
2 3 1
0 3

=~=~=~=~= Sample Output =~=~=~=~=
The shortest distance from 0 to 3 is 5.
Take the path: 0->1->2->3.

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAXN = 100, INF = 0x3F3F3F3F;
int nodes, edges, a, b, weight, start, dest;
int dist[MAXN], pred[MAXN];
vector<bool> vis(MAXN);
vector<pair<int, int> > adj[MAXN+1];

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> weight;
    adj[a].push_back(make_pair(b, weight));
  }
  cin >> start >> dest;
  for (int i = 0; i < nodes; i++) {
    dist[i] = INF;
    pred[i] = -1;
  }
  dist[start] = 0;
  priority_queue<pair<int, int> > pq;
  pq.push(make_pair(0, start));
  while (!pq.empty()) {
    a = pq.top().second;
    pq.pop();
    vis[a] = true;
    for (int j = 0; j < adj[a].size(); j++) {
      b = adj[a][j].first;
      if (vis[b]) continue;
      if (dist[b] > dist[a] + adj[a][j].second) {
        dist[b] = dist[a] + adj[a][j].second;
        pred[b] = a;
        pq.push(make_pair(-dist[b], b));
      }
    }
  }
  cout << "The shortest distance from " << start;
  cout << " to " << dest << " is " << dist[dest] << ".\n";

  //Optional: Use pred[] to backtrack and print the path
  int i = 0, j = dest, path[MAXN];
  while (pred[j] != -1) j = path[++i] = pred[j];
  cout << "Take the path: ";
  while (i > 0) cout << path[i--] << "->";
  cout << dest << ".\n";
  return 0;
} 
