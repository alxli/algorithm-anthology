/*

Given a flow network with integer capacities, find the maximum flow from a given source node to a
given sink node. The flow along each edge may not exceed its capacity, and flow is conserved at
every node other than the source and sink.

Rather than augmenting along whole paths, the push-relabel algorithm maintains a preflow and a
height label on each node, repeatedly pushing excess flow to lower-labeled neighbors and relabeling
(raising) a node whose excess cannot yet be pushed, until no node other than the source and sink
holds excess.

- `push_relabel(source, sink)` returns the maximum flow for a global capacity matrix `cap`, whose
  row and column indices represent the nodes.
- `min_cut(source)` returns the source side of a minimum cut after `push_relabel()` has been called.

Unlike Ford-Fulkerson, the O(n^3) bound does not improve when the integral maximum flow $f$ is
small; the matrix-based Ford-Fulkerson implementation runs in O(n^2*f).

Time Complexity:
- O(n^3) per call to `push_relabel()`, where $n$ is the number of nodes.
- O(n^2) per call to `min_cut()`.

Space Complexity:
- O(n^2) for storage of the flow network, where $n$ is the number of nodes.
- O(n) auxiliary for `push_relabel()`.

*/

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <queue>
#include <vector>

std::vector<std::vector<int64_t>> cap;
std::vector<std::vector<int64_t>> f;

int64_t push_relabel(int source, int sink) {
  int n = static_cast<int>(cap.size());
  assert(source != sink);
  f.assign(n, std::vector<int64_t>(n, 0));
  std::vector<int64_t> excess(n, 0);
  std::vector<int> height(n, 0), max_height(n, 0);
  height[source] = n - 1;
  for (int i = 0; i < n; i++) {
    f[source][i] = cap[source][i];
    f[i][source] = -f[source][i];
    excess[i] = cap[source][i];
  }
  int size = 0;
  while (true) {
    if (size == 0) {
      for (int i = 0; i < n; i++) {
        if (i != source && i != sink && excess[i] > 0) {
          if (size != 0 && height[i] > height[max_height[0]]) {
            size = 0;
          }
          max_height[size++] = i;
        }
      }
    }
    if (size == 0) {
      break;
    }
    while (size != 0) {
      int i = max_height[size - 1];
      bool pushed = false;
      for (int j = 0; j < n && excess[i] != 0; j++) {
        if (height[i] == height[j] + 1 && cap[i][j] - f[i][j] > 0) {
          int64_t df = std::min(cap[i][j] - f[i][j], excess[i]);
          f[i][j] += df;
          f[j][i] -= df;
          excess[i] -= df;
          excess[j] += df;
          if (excess[i] == 0) {
            size--;
          }
          pushed = true;
        }
      }
      if (pushed) {
        continue;
      }
      height[i] = INT_MAX / 2;
      for (int j = 0; j < n; j++) {
        if (height[i] > height[j] + 1 && cap[i][j] - f[i][j] > 0) {
          height[i] = height[j] + 1;
        }
      }
      if (height[i] > height[max_height[0]]) {
        size = 0;
        break;
      }
    }
  }
  int64_t max_flow = 0;
  for (int i = 0; i < n; i++) {
    max_flow += f[source][i];
  }
  return max_flow;
}

std::vector<char> min_cut(int source) {
  int n = static_cast<int>(cap.size());
  std::vector<char> reachable(n);
  std::queue<int> q;
  reachable[source] = true;
  q.push(source);
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int v = 0; v < n; v++) {
      if (!reachable[v] && cap[u][v] - f[u][v] > 0) {
        reachable[v] = true;
        q.push(v);
      }
    }
  }
  return reachable;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  // Example graph after max flow, with each edge labeled flow/capacity:
  //            2/2
  //       1 --------> 3
  //      / \          |
  // 3/4 /   \ 1/1     | 2/2
  //    /     v        v
  //   0       4 ----> 5
  //    \     ^   3/3
  // 2/3 \   / 2/2
  //      v /
  //       2
  int nodes = 6;
  cap.assign(nodes, std::vector<int64_t>(nodes));
  cap[0][1] = 4;
  cap[0][2] = 3;
  cap[1][3] = 2;
  cap[1][4] = 1;
  cap[2][4] = 2;
  cap[3][5] = 2;
  cap[4][5] = 3;
  assert(push_relabel(0, 5) == 5);
  std::vector<char> cut = min_cut(0);
  assert(cut[0] && !cut[5]);
  return 0;
}
