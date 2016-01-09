/*

An unweighted tree possesses a center, centroid, and diameter. The
following functions apply to a global, prepopulated adjacency matrix
adj which satisfies the precondition that for every node v in adj[u],
the node u also exists in adj[v]. All node indices must be between 0
and the number of nodes minus 1 as given in the function arguments.

find_centers() returns one or two tree centers. The center (or Jordan
center) of a graph is the set of all nodes with minimum eccentricity,
that is, the set of all nodes where the max distance to all other nodes
is minimal.

find_centroid() returns a node where all of its subtrees have a size
less than or equal to n/2, where n is the number of nodes in the tree.

diameter() returns the greatest distance between any two nodes in the
tree, using the famous double DFS technique.

Time Complexity: O(n) on the number of nodes in the tree.
Space Complexity: O(n) auxiliary on the number of nodes in the tree.

*/

#include <vector>
#include <utility> /* std::pair */

const int MAXN = 100;
std::vector<int> adj[MAXN];

std::vector<int> find_centers(int nodes) {
  std::vector<int> leaves, degree(nodes);
  for (int i = 0; i < nodes; i++) {
    degree[i] = adj[i].size();
    if (degree[i] <= 1) leaves.push_back(i);
  }
  int removed = leaves.size();
  while (removed < nodes) {
    std::vector<int> nleaves;
    for (int i = 0; i < (int)leaves.size(); i++) {
      int u = leaves[i];
      for (int j = 0; j < (int)adj[u].size(); j++) {
        int v = adj[u][j];
        if (--degree[v] == 1)
          nleaves.push_back(v);
      }
    }
    leaves = nleaves;
    removed += leaves.size();
  }
  return leaves;
}

int find_centroid(int nodes, int u = 0, int p = -1) {
  int cnt = 1, v;
  bool good_center = true;
  for (int j = 0; j < (int)adj[u].size(); j++) {
    if ((v = adj[u][j]) == p) continue;
    int res = find_centroid(nodes, v, u);
    if (res >= 0) return res;
    int size = -res;
    good_center &= (size <= nodes / 2);
    cnt += size;
  }
  good_center &= (nodes - cnt <= nodes / 2);
  return good_center ? u : -cnt;
}

std::pair<int, int> dfs(int u, int p, int depth) {
  std::pair<int, int> res = std::make_pair(depth, u);
  for (int j = 0; j < (int)adj[u].size(); j++)
    if (adj[u][j] != p)
      res = max(res, dfs(adj[u][j], u, depth + 1));
  return res;
}

int diameter() {
  int furthest_vertex = dfs(0, -1, 0).second;
  return dfs(furthest_vertex, -1, 0).first;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  int nodes = 6;
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(2);
  adj[2].push_back(1);
  adj[1].push_back(4);
  adj[4].push_back(1);
  adj[3].push_back(4);
  adj[4].push_back(3);
  adj[4].push_back(5);
  adj[5].push_back(4);
  vector<int> centers = find_centers(nodes);
  assert(centers.size() == 2 && centers[0] == 1 && centers[1] == 4);
  assert(find_centroid(nodes) == 4);
  assert(diameter() == 3);
  return 0;
}
