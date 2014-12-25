/*

1.1 - Depth First Search

Description: Given an unweighted graph, traverse all reachable
nodes from a source node. Each branch is explored as deep as
possible before more branches are visited. DFS only uses as
much space as the length of the longest branch. When DFS'ing
recursively, the internal call-stack could overflow, so
sometimes it is safer to use an explicit stack data structure.

Complexity: O(number of edges) for explicit graphs traversed
without repetition. O(b^d) for implicit graphs with a branch
factor of b, searched to depth d.

=~=~=~=~= Sample Input =~=~=~=~=
12 11 1
1 2
1 7
1 8
2 3
2 6
3 4
3 5
8 9
8 12
9 10
9 11

=~=~=~=~= Sample Output =~=~=~=~=
Nodes visited:
1 2 3 4 5 6 7 8 9 10 11 12

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAX_N = 101;
int nodes, edges, start, a, b;
bool visit[MAX_N] = {0};
vector<int> adj[MAX_N];

void DFS(int n) {
    visit[n] = true;
    cout << " " << n;
    for (int j = 0; j < adj[n].size(); j++)
        if (!visit[adj[n][j]]) DFS(adj[n][j]);
}

int main() {
    cin >> nodes >> edges >> start;
    for (int i = 0; i < edges; i++) {
        cin >> a >> b;
        adj[a].push_back(b);
    }
    cout << "Nodes visited:\n";
    DFS(start);
    return 0;
}
