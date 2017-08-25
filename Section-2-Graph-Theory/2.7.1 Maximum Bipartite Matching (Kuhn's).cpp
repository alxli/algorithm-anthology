/*

Given two sets of nodes A = {0, 1, ..., n1} and B = {0, 1, ..., n2} such that
n1 < n2, as well as a set of edges E mapping nodes from set A to set B, find the
largest possible subset of E containing no edges that share the same node.
kuhn() applies to a global, pre-populated adjacency list adj[] which must only
consist of nodes numbered with integers between 0 (inclusive) and the total
number of nodes (exclusive), as passed in the function argument.

Time Complexity:
- O(m*(n1 + n2)) per call to kuhn(), where m is the number of edges.

Space Complexity:
- O(n1 + n2) auxiliary stack space for kuhn().

*/

#include <algorithm>
#include <vector>

const int MAXN = 100;
int match[MAXN];
std::vector<bool> visit(MAXN);
std::vector<int> adj[MAXN];

bool dfs(int u) {
  visit[u] = true;
  for (int j = 0; j < (int)adj[u].size(); j++) {
    int v = match[adj[u][j]];
    if (v == -1 || (!visit[v] && dfs(v))) {
      match[adj[u][j]] = u;
      return true;
    }
  }
  return false;
}

int kuhn(int n1, int n2) {
  std::fill(visit.begin(), visit.end(), false);
  std::fill(match, match + n2, -1);
  int matches = 0;
  for (int i = 0; i < n1; i++) {
    std::fill(visit.begin(), visit.begin() + n1, false);
    if (dfs(i)) {
      matches++;
    }
  }
  return matches;
}

/*** Example Usage and Output:

Matched 3 pair(s):
1 0
0 1
2 2

***/

#include <iostream>
using namespace std;

int main() {
  int n1 = 3, n2 = 4;
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(1);
  adj[1].push_back(2);
  adj[2].push_back(2);
  adj[2].push_back(3);
  cout << "Matched " << kuhn(n1, n2) << " pair(s):" << endl;
  for (int i = 0; i < n2; i++) {
    if (match[i] != -1) {
      cout << match[i] << " " << i << endl;
    }
  }
  return 0;
}
