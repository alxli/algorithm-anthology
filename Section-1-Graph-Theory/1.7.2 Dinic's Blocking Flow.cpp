/*

1.7.2 - Dinic's Blocking Flow Algorithm

Description: Given a flow network, find a flow from a single
source node to a single sink node that is maximized.

Complexity: O(V^2*E) on the number of vertices and edges.

Comparison with Edmonds-Karp Algorithm:
Dinic’s algorithm is similar to the Edmonds-Karp algorithm in that
it uses the shortest augmenting path. The introduction of the
concepts of the level graph and blocking flow enable Dinic's
algorithm to achieve its better performance. Hence, Dinic’s
algorithm is also called Dinic’s blocking flow algorithm.

*/

#include <cstdio>
#include <vector>
 
const int MAX_N = 2005, INF = 1 << 30;
struct edge { int to, rev, f; };
 
int nodes, source, sink;
std::vector<edge> adj[MAX_N];
 
void add_edge(int s, int t, int cap) {
    adj[s].push_back((edge){t, (int)adj[t].size(), cap});
    adj[t].push_back((edge){s, (int)adj[s].size()-1, 0});
}
 
int dist[MAX_N], queue[MAX_N], work[MAX_N];
 
bool dinic_bfs() {
    for (int i = 0; i < nodes; i++) dist[i] = -1;
    dist[source] = 0;
    int qh = 0, qt = 0;                 /* queue<int> q; */
    queue[qt++] = source;             /* q.push(source); */
    while (qh < qt) {            /* while (!q.empty()) { */
        int u = queue[qh++];  /* u = q.front(); q.pop(); */
        for (int j = 0; j < adj[u].size(); j++) {
            edge &e = adj[u][j];
            int v = e.to;
            if (dist[v] < 0 && e.f) {
                dist[v] = dist[u] + 1;
                queue[qt++] = v;            /* q.push(v) */
            }
        }
    }
    return dist[sink] >= 0;
}
 
int dinic_dfs(int u, int f) {
    if (u == sink) return f;
    for (int &i = work[u]; i < adj[u].size(); i++) {
        edge &e = adj[u][i];
        if (!e.f) continue;
        int v = e.to, df;
        if (dist[v] == dist[u] + 1 &&
           (df = dinic_dfs(v, std::min(f, e.f))) > 0) {
            e.f -= df;
            adj[v][e.rev].f += df;
            return df;
        }
    }
    return 0;
}
 
int dinic() {
    int result = 0;
    while (dinic_bfs()) {
        for (int i = 0; i < nodes; i++) work[i] = 0;
        while (int delta = dinic_dfs(source, INF))
            result += delta;
    }
    return result;
}

#include <iostream>
using namespace std;

int main() {
    int edges, a, b, capacity;
    cin >> nodes >> edges;
    for (int i = 0; i < edges; i++) {
        cin >> a >> b >> capacity;
        add_edge(--a, --b, capacity);
    }
    cin >> source >> sink;
    source--; sink--;
    cout << dinic() << endl;
    return 0;
}

/*

=~=~=~=~= Sample Input =~=~=~=~=
6 8
1 2 3
1 3 3
2 3 2
2 4 3
3 5 2
4 5 1
4 6 2
5 6 3
1 6

=~=~=~=~= Sample Output =~=~=~=~=
5

*/
