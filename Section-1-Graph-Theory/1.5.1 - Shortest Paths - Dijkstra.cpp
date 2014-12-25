/*

1.5 - Shortest Path Problems

Description: Shortest path problems mainly fall into two categories:
single-source, or all-pairs. Dijkstra’s and Bellman-Ford’s algorithms
can be used to solve the former while the Floyd-Warshall algorithm
can be used to solve the latter. BFS is a special case of Dijkstra’s
algorithm where the priority queue becomes a FIFO queue on unweighted
graphs. Dijkstra’s algorithm is a special case of A* search, where the
heuristic is zero. The all-pairs shortest path on sparse graphs is
best computed with Johnson’s algorithm (a combination of Bellman-Ford
and Dijkstra’s). However, Johnson’s algorithm and A* search are both
rare in contests, and thus are ommitted.


1.5.1 - Dijkstra's Algorithm

Complexity: The simplest version runs in O(E+V^2) where V is the number
of vertices and E is the number of edges. Using an adjacency list and
priority queue (internally a binary heap), the implementation here is
Θ((E+V) log(V)), dominated by Θ(E log(V)) if the graph is connected.

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

Implementation Notes: The graph is stored using an adjacency list.
This implementation negates distances before adding them to the 
priority queue, since the container is a max-heap by default. This
method is suggested in contests because it is easier than defining
special comparators. An alternative would be declaring the queue
with template parameters (obviously it is ugly):
 priority_queue< pair<int,int>, vector< pair<int,int> >, greater< pair<int,int> > > pq;
If the path is to be computed for only a single pair of nodes, one
may break out of the loop as soon as the destination is reached,
by inserting the line "if (a == dest) break;"  after the line  "pq.pop();"

Shortest Path Faster Algorithm: The code for Dijkstra’s algorithm
here can be easily modified to become the Shortest Path Faster
Algorithm (SPFA) by simply commenting out "visit[a] = true;" and changing
the priority queue to a FIFO queue like BFS. SPFA is a faster version of
the Bellman-Ford algorithm, working on negative path lengths (whereas
Dijkstra’s cannot). However, certain graphs can be crafted to make
the SPFA very slow.

=~=~=~=~= Sample Input =~=~=~=~=
4 5
1 2 2
1 4 8
2 3 2
2 4 4
3 4 1
1 4

=~=~=~=~= Sample Output =~=~=~=~=
The shortest distance from 1 to 4 is 5.
Take the path 1->2->3->4.

*/
