/*

Given a flow network with integer capacities, find the maximum flow from a given source node to a
given sink node. The flow along each edge may not exceed its capacity, and flow is conserved at
every node other than the source and sink.

Rather than augmenting along whole paths, the push-relabel algorithm maintains a preflow and a
height label on each node, repeatedly pushing excess flow to lower-labeled neighbors and relabeling
(raising) a node whose excess cannot yet be pushed, until no node other than the source and sink
holds excess.

- `push_relabel(source, sink)` returns maximum flow for a global capacity matrix `cap` whose nodes
  are numbered from 0 to `cap.size() - 1`.
- `min_cut(source)` returns the source side of a minimum cut after `push_relabel()` has been called.

Although the push-relabel algorithm is considered one of the most efficient maximum flow algorithms,
it cannot take advantage of the magnitude of the maximum flow being less than $n^3$ (in which case
the Ford-Fulkerson or Edmonds-Karp algorithms may be more efficient).

Time Complexity:
- O(n^3) per call to `push_relabel()`, where $n$ is the number of nodes.
- O(n^2) per call to `min_cut()`.

Space Complexity:
- O(n^2) for storage of the flow network, where $n$ is the number of nodes.
- O(n) auxiliary heap space for `push_relabel()`.

*/

#include <algorithm>
#include <climits>
#include <cstdint>
#include <queue>
#include <vector>

const int INF = INT_MAX / 2;
std::vector<std::vector<int>> cap;
std::vector<std::vector<int>> f;

int64_t push_relabel(int source, int sink) {
  int nodes = static_cast<int>(cap.size());
  f.assign(nodes, std::vector<int>(nodes, 0));
  std::vector<int> e(nodes, 0), h(nodes, 0), maxh(nodes, 0);
  h[source] = nodes - 1;
  for (int i = 0; i < nodes; i++) {
    f[source][i] = cap[source][i];
    f[i][source] = -f[source][i];
    e[i] = cap[source][i];
  }
  int size = 0;
  for (;;) {
    if (size == 0) {
      for (int i = 0; i < nodes; i++) {
        if (i != source && i != sink && e[i] > 0) {
          if (size != 0 && h[i] > h[maxh[0]]) {
            size = 0;
          }
          maxh[size++] = i;
        }
      }
    }
    if (size == 0) {
      break;
    }
    while (size != 0) {
      int i = maxh[size - 1];
      bool pushed = false;
      for (int j = 0; j < nodes && e[i] != 0; j++) {
        if (h[i] == h[j] + 1 && cap[i][j] - f[i][j] > 0) {
          int df = std::min(cap[i][j] - f[i][j], e[i]);
          f[i][j] += df;
          f[j][i] -= df;
          e[i] -= df;
          e[j] += df;
          if (e[i] == 0) {
            size--;
          }
          pushed = true;
        }
      }
      if (pushed) {
        continue;
      }
      h[i] = INF;
      for (int j = 0; j < nodes; j++) {
        if (h[i] > h[j] + 1 && cap[i][j] - f[i][j] > 0) {
          h[i] = h[j] + 1;
        }
      }
      if (h[i] > h[maxh[0]]) {
        size = 0;
        break;
      }
    }
  }
  int64_t max_flow = 0;
  for (int i = 0; i < nodes; i++) {
    max_flow += f[source][i];
  }
  return max_flow;
}

std::vector<bool> min_cut(int source) {
  int nodes = static_cast<int>(cap.size());
  std::vector<bool> reachable(nodes);
  std::queue<int> q;
  reachable[source] = true;
  q.push(source);
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int v = 0; v < nodes; v++) {
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
  int nodes = 6;
  cap.assign(nodes, std::vector<int>(nodes));
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
  cap[0][1] = 4;
  cap[0][2] = 3;
  cap[1][3] = 2;
  cap[1][4] = 1;
  cap[2][4] = 2;
  cap[3][5] = 2;
  cap[4][5] = 3;
  assert(push_relabel(0, 5) == 5);
  std::vector<bool> cut = min_cut(0);
  assert(cut[0] && !cut[5]);
  return 0;
}
