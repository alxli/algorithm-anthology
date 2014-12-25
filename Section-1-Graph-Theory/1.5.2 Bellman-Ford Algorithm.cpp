/*

1.5.2 - Bellman-Ford Algorithm (Single-Source Shortest Path)

Description: Given a directed graph with positive or negative weights
but no negative cycles, find the shortest distance to all nodes from
a single starting node. The input graph is stored using an edge list.

Complexity: O(V*E) on the number of vertices and edges, respectively.

*/

#include <iostream>
using namespace std;

const int MAX_N=101, MAX_E = MAX_N*MAX_N, INF=1<<28;
int nodes, edges, a, b, weight, start, dest;
int E[MAX_E][3], dist[MAX_N], pred[MAX_N];

int main() {
    cin >> nodes >> edges;
    for (int i = 0; i < edges; i++)
        cin >> E[i][0] >> E[i][1] >> E[i][2];
    cin >> start >> dest;

    for (int i = 1; i <= nodes; i++) {
        dist[i] = INF;
        pred[i] = -1;
    }
    dist[start] = 0;
    for (int i = 1; i <= nodes; i++)
      for (int j = 0; j < edges; j++)
        if (dist[E[j][1]] > dist[E[j][0]] + E[j][2]) {
            dist[E[j][1]] = dist[E[j][0]] + E[j][2];
            pred[E[j][1]] = E[j][0];
        }
    cout << "The shortest path from " << start;
    cout << " to " << dest << " is ";
    cout << dist[dest] << "." << endl;

    /* Optional: Report negative-weight cycles */
    for (int i = 0; i < edges; i++) 
      if (dist[E[i][0]] + E[i][2] < dist[E[i][1]])
        cout << "Negative-weight cycle detected!\n";

    /* Use pred[] to backtrack and print the path */
    int i = 0, j = dest, path[MAX_N + 1];
    while (pred[j] != -1) j = path[++i] = pred[j];
    cout << "Take the path: ";
    while (i > 0) cout << path[i--] << "->";
    cout << dest << ".\n";
    return 0;
}

/*

=~=~=~=~= Sample Input =~=~=~=~=
3 3
1 2 1
2 3 2
1 3 5
1 3 

=~=~=~=~= Sample Output =~=~=~=~=
The shortest path from 1 to 3 is 3.
Take the path 1->2->3. 

*/
