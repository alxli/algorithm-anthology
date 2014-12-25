/*

1.5.3 - Floyd-Warshall Algorithm (All-Pairs Shortest Path)

Description: Given a directed graph with positive or negative weights
but no negative cycles, find the shortest distance between all pairs
of nodes. The input graph is stored using an adjacency matrix.

Complexity: O(V^3) on the number of vertices.

*/

#include <iostream>
using namespace std;

const int MAX_N = 101, INF = 1<<28;
int nodes, edges, a, b, weight, start, dest;
int dist[MAX_N][MAX_N], next[MAX_N][MAX_N];

void print_path(int i, int j) {
    if (next[i][j] != -1) {
        print_path(i, next[i][j]);
        cout << next[i][j];
        print_path(next[i][j], j);
    } else cout << "->";
}

int main() {
    cin >> nodes >> edges;
    for (int i = 0; i < edges; i++) {
        cin >> a >> b >> weight;
        dist[a][b] = weight;
    }
    cin >> start >> dest;
    for (int i = 1; i <= nodes; i++)
        for (int j = 1; j <= nodes; j++) {
            dist[i][j] = (i == j) ? 0 : INF;
            next[i][j] = -1;
        }
    for (int k = 1; k <= nodes; k++)
     for (int i = 1; i <= nodes; i++)
      for (int j = 1; j <= nodes; j++)
        if (dist[i][j] > dist[i][k] + dist[k][j]) {
            dist[i][j] = dist[i][k] + dist[k][j];
            next[i][j] = k;
        }
    cout << "The shortest path from " << start;
    cout << " to " << dest << " is ";
    cout << dist[start][dest] << ".\n";

    /* Use next[][] to recursively print the path */
    cout << "Take the path " << start;
    print_path(start, dest);
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
