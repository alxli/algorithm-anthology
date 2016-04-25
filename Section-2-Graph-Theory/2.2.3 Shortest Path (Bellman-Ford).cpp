/*

Given a starting node in a weighted, directed graph with possibly
negative weights, traverse to every connected node and determine the
shortest distance to each. Optionally, detect if there exists a
negative-weighted cycle (in which case there is no shortest path).
Optionally, output the shortest path to a specific destination node
using the shortest-path tree precomputed into the pred[] array.

Time Complexity: bellman_ford() is O(nm) where n is the number of nodes
and m is the number of edges. print_path() is O(n) on the number of
nodes in the shortest path to be printed.

Space Complexity: O(n) on the number of edges to store the input graph
as an adjacency list and O(n) auxiliary on the number of nodes.

*/

#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;

struct edge { int u, v, w; };

const int MAXN = 100, INF = 0x3f3f3f3f;
int dist[MAXN], pred[MAXN];
vector<edge> e;

void bellman_ford(int start) {
  for (int i = 0; i < MAXN; i++) {
    dist[i] = INF;
    pred[i] = -1;
  }
  dist[start] = 0;
  for (int i = 0; i < MAXN; i++) {
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
      throw std::runtime_error("Negative-weight cycle found.");
}

void print_path(int dest) {
  int i = 0, j = dest, path[MAXN];
  while (pred[j] != -1)
    j = path[++i] = pred[j];
  cout << "Take the path: ";
  while (i > 0)
    cout << path[i--] << "->";
  cout << dest << ".\n";
}

/*** Example Usage

Sample Output:
The shortest distance from 0 to 2 is 3.
Take the path: 0->1->2.

***/

int main() {
  int start = 0, dest = 2;
  e.push_back((edge){0, 1, 1});
  e.push_back((edge){1, 2, 2});
  e.push_back((edge){0, 2, 5});
  bellman_ford(start);
  cout << "The shortest distance from " << start;
  cout << " to " << dest << " is " << dist[dest] << ".\n";
  print_path(dest);
  return 0;
}
