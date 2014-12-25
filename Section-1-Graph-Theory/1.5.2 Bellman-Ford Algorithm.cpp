/*

1.5.2 - Bellman-Ford Algorithm

Description: Given a directed graph with positive or negative weights
but no negative cycles, find the shortest distance to all nodes from
a single starting node. The input graph is stored using an edge list.

Complexity: O(V*E) on the number of vertices and edges, respectively.

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAX_N = 101, INF = 1 << 28;
int nodes, edges, a, b, weight, start, dest;
int dist[MAX_N], pred[MAX_N];
bool visit[MAX_N] = {0};
vector< pair<int, int> > adj[MAX_N];

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
        visit[i] = false;
    }
    dist[start] = 0;
    priority_queue< pair<int, int> > pq;
    pq.push(make_pair(0, start));
    while (!pq.empty()) {
        a = pq.top().second;
        pq.pop();
        visit[a] = true;
        for (int j = 0; j < adj[a].size(); j++) {
            b = adj[a][j].first;
            if (visit[b]) continue;
            if (dist[b] > dist[a] + adj[a][j].second) {
                dist[b] = dist[a] + adj[a][j].second;
                pred[b] = a;
                pq.push(make_pair(-dist[b], b));
            }
        }
    }
    cout << "The shortest distance from " << start;
    cout << " to " << dest << " is " << dist[dest] << ".\n";

    /* Use pred[] to backtrack and print the path */
    int i = 0, j = dest, path[MAX_N];
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
