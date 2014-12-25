/*

1.6.1 - Kruskal's Algorithm (Minimum Spanning Tree)

Description: Given an undirected graph, its minimum spanning tree (MST)
is a tree connecting all nodes with a subset of its edges such that their
total weight is minimized. The input graph is stored in an edge list.

Complexity: O(E*log(V)), where E and V are the number of edges and vertices.

*/

#include <algorithm> /* std::sort() */
#include <iostream>
#include <vector>
using namespace std;

const int MAX_N = 101;
int nodes, edges, a, b, weight, root[MAX_N];
vector< pair<int, pair<int, int> > > E;
vector< pair<int, int> > MST;

int find_root(int x) {
    if (root[x] != x) root[x] = find_root(root[x]);
    return root[x];
}

int main() {
    cin >> nodes >> edges;
    for (int i = 0; i < edges; i++) {
        cin >> a >> b >> weight;
        E.push_back(make_pair(weight, make_pair(a, b)));
    }

/* If you already have a disjoint set data structure:
    disjoint_set_forest<int> F;
    sort(E.begin(), E.end());
    for (int i = 1; i <= nodes; i++) F.make_set(i);
    int totalDistance = 0;
    for (int i = 0; i < E.size(); i++) {
        a = E[i].second.first;
        b = E[i].second.second;
        if (!F.is_united(a, b)) {
            MST.push_back(E[i].second);
            totalDistance += E[i].first;
            F.unite(a, b);
        }
    }
  Otherwise, do the following:
*/
    sort(E.begin(), E.end());
    for (int i = 1; i <= nodes; i++) root[i] = i;
    int totalDistance = 0;
    for (int i = 0; i < E.size(); i++) {
        a = find_root(E[i].second.first);
        b = find_root(E[i].second.second);
        if (a != b) {
            MST.push_back(E[i].second);
            totalDistance += E[i].first;
            root[a] = root[b];
        }
    }
    
    for (int i = 0; i < MST.size(); i++) {
        cout << MST[i].first << "<->";
        cout << MST[i].second << endl;
    }
    cout << "Total distance: " << totalDistance << endl;
    return 0;
} 

/*

=~=~=~=~= Sample Input =~=~=~=~=
7 7
1 2 4
2 3 6
3 1 3
4 5 1
5 6 2
6 7 3
7 5 4

=~=~=~=~= Sample Output =~=~=~=~=
4<->5
5<->6
3<->1
6<->7
1<->2
Total distance: 13

*/
