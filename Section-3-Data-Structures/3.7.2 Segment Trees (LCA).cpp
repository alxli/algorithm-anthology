/*

3.7.2 - Segment Trees for Lowest Common Ancestor

Description: Given a rooted tree, the lowest common ancestor (LCA)
of two nodes v and w is the lowest (i.e. deepest) node that has
both v and w as descendants, where we define each node to be a
descendant of itself (so if v has a direct connection from w, w
is the lowest common ancestor). This problem can be reduced to the
range minimum query problem using Eulerian tours.

Time Complexity: O(N log N) for build() and O(log N) for lca(),
where N is the number of nodes in the tree.

Space Complexity: O(N log N).

*/

#include <algorithm> /* std::fill() */
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 1000;
int nodes, len, counter;
int depth[MAXN], dfs_order[2*MAXN], first[MAXN], minpos[8*MAXN];
vector<int> adj[MAXN];

void dfs(int u, int d) {
  depth[u] = d;
  dfs_order[counter++] = u;
  for (int j = 0, v; j < adj[u].size(); j++) {
    if (depth[v = adj[u][j]] == -1) {
      dfs(v, d + 1);
      dfs_order[counter++] = u;
    }
  }
}

void build_tree(int n, int l, int h) {
  if (l == h) {
    minpos[n] = dfs_order[l];
    return;
  }
  build_tree(2*n + 1, l, (l + h)/2);
  build_tree(2*n + 2, (l + h)/2 + 1, h);
  minpos[n] = depth[minpos[2*n + 1]] < depth[minpos[2*n + 2]] ?
              minpos[2*n + 1] : minpos[2*n + 2];
}

void build(int root) {
  fill(depth, depth + nodes, -1);
  fill(first, first + nodes, -1);
  len = 2*nodes - 1;
  counter = 0;
  dfs(root, 0);
  build_tree(0, 0, len - 1);
  for (int i = 0; i < len; i++)
    if (first[dfs_order[i]] == -1)
      first[dfs_order[i]] = i;
}

int get_minpos(int a, int b, int n, int l, int h) {
  if (a == l && b == h) return minpos[n];
  int mid = (l + h) >> 1;
  if (a <= mid && b > mid) {
    int p1 = get_minpos(a, std::min(b, mid), 2*n + 1, l, mid);
    int p2 = get_minpos(std::max(a, mid + 1), b, 2*n + 2, mid + 1, h);
    return depth[p1] < depth[p2] ? p1 : p2;
  }
  if (a <= mid) return get_minpos(a, std::min(b, mid), 2*n + 1, l, mid);
  return get_minpos(std::max(a, mid + 1), b, 2*n + 2, mid + 1, h);
}

int lca(int a, int b) {
  return get_minpos(std::min(first[a], first[b]),
                    std::max(first[a], first[b]), 0, 0, len - 1);
}

int main() {
  nodes = 5;
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(2);
  adj[2].push_back(1);
  adj[3].push_back(1);
  adj[1].push_back(3);
  adj[0].push_back(4);
  adj[4].push_back(0);
  build(0);
  cout << lca(3, 2) << "\n"; //1
  cout << lca(2, 4) << "\n"; //0
  return 0;
}
