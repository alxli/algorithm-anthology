/*

1.8.1 Kosaraju's Algorithm for Strongly Connected Components

Description: Determines the strongly connected components
from a given directed graph. The input is stored in an
adjacency list.

Complexity: Θ(V + E) on the number of vertices and edges.

Comparison with other SCC algorithms:
The strongly connected components of a graph can be efficiently
computed using Kosaraju’s algorithm, Tarjan’s algorithm, or the
path-based strong component algorithm. Tarjan’s algorithm can
be seen as an improved version of Kosaraju’s because it performs
a single DFS rather than two. Though they both have the same
complexity, Tarjan’s algorithm is much more efficient in
practice. However, Kosaraju’s algorithm is conceptually simpler.

*/

#include <algorithm> /* std::reverse() */
#include <iostream>
#include <vector>
using namespace std;

const int MAX_N = 101;
int nodes, edges, a, b;
bool visit[MAX_N] = {0};
vector<int> adj[MAX_N], rev[MAX_N], order;
vector< vector<int> > SCC;

void DFS(vector<int> graph[], vector<int> &res, int i) {
    visit[i] = true;
    for (int j = 0; j < graph[i].size(); j++)
        if (!visit[graph[i][j]])
            DFS(graph, res, graph[i][j]);
    res.push_back(i);
}

int main() {
    cin >> nodes >> edges;
    for (int i = 0; i < edges; i++) {
        cin >> a >> b;    a--, b--;
        adj[a].push_back(b);
    }

    for (int i = 0; i < nodes; i++) visit[i] = false;
    for (int i = 0; i < nodes; i++)
        if (!visit[i]) DFS(adj, order, i);
    for (int i = 0; i < nodes; i++)
        for (int j = 0; j < adj[i].size(); j++)
            rev[adj[i][j]].push_back(i);
    for (int i = 0; i < nodes; i++) visit[i] = false;
    reverse(order.begin(), order.end());
    for (int i = 0; i < order.size(); i++)
        if (!visit[order[i]]) {
            vector<int> component;
            DFS(rev, component, order[i]);
            SCC.push_back(component);
        }

    for (int i = 0; i < SCC.size(); i++) {
        cout << "Component " << i + 1 << ":";
        for (int j = 0; j < SCC[i].size(); j++)
            cout << " " << (SCC[i][j] + 1);
        cout << endl;
    }
    return 0;
}


/*

=~=~=~=~= Sample Input =~=~=~=~=
8 14
1 2
2 3
2 5
2 6
3 4
3 7
4 3
4 8
5 1
5 6
6 7
7 6
8 4
8 7

=~=~=~=~= Sample Output =~=~=~=~=
Component 1: 2 5 1
Component 2: 8 4 3
Component 3: 6 7

*/
