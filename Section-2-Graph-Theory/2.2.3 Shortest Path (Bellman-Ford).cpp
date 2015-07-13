/*

2.2.3 - Bellman-Ford Algorithm (Single-Source Shortest Path)

Description: Given a directed graph with positive or negative weights
but no negative cycles, find the shortest distance to all nodes from
a single starting node. The input graph is stored using an edge list.

Complexity: O(V*E) on the number of vertices and edges, respectively.

=~=~=~=~= Sample Input =~=~=~=~=
3 3
0 1 1
1 2 2
0 2 5
0 2

=~=~=~=~= Sample Output =~=~=~=~=
The shortest distance from 0 to 2 is 3.
Take the path: 0->1->2.

*/

#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;

struct edge { int u, v, w; };

const int MAXN = 100, INF = 0x3f3f3f3f;
int dist[MAXN], pred[MAXN];
vector<edge> e;

void bellman_ford(int nodes, int start) {
  for (int i = 0; i < nodes; i++) {
    dist[i] = INF;
    pred[i] = -1;
  }
  dist[start] = 0;
  for (int i = 0; i < nodes; i++) {
    for (int j = 0; j < (int)e.size(); j++) {
      if (dist[e[j].v] > dist[e[j].u] + e[j].w) {
        dist[e[j].v] = dist[e[j].u] + e[j].w;
        pred[e[j].v] = e[j].u;
      }
    }
  }
  //optional: report negative-weight cycles
  for (int i = 0; i < (int)e.size(); i++)
    if (dist[e[i].v] > dist[e[i].u] + e[i].w)
      throw std::runtime_error("Negative-weight found");
}

//Use the precomputed pred[] array to print the path
void print_path(int dest) {
  int i = 0, j = dest, path[MAXN];
  while (pred[j] != -1) j = path[++i] = pred[j];
  cout << "Take the path: ";
  while (i > 0) cout << path[i--] << "->";
  cout << dest << ".\n";
}

int main() {
  int nodes, edges, u, v, w, start, dest;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v >> w;
    e.push_back((edge){u, v, w});
  }
  cin >> start >> dest;
  bellman_ford(nodes, start);
  cout << "The shortest distance from " << start;
  cout << " to " << dest << " is " << dist[dest] << ".\n";
  print_path(dest);
  return 0;
}
