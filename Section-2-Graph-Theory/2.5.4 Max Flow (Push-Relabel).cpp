/*

Given a flow network with integer capacities, find the maximum flow from a given
source node to a given sink node. The flow of a given edge u -> v is defined as
the minimum of its capacity and the sum of the flows of all incoming edges of u.
push_relabel() applies to a global adjacency matrix cap[][] and returns the
maximum flow.

Although the push-relabel algorithm is considered one of the most efficient
maximum flow algorithms, it cannot take advantage of the magnitude of the
maximum flow being less than n^3 (in which case the Ford-Fulkerson or
Edmonds-Karp algorithms may be more efficient).

Time Complexity:
- O(n^3) per call to push_relabel(), where n is the number of nodes.

Space Complexity:
- O(n^2) for storage of the flow network, where n is the number of nodes.
- O(n) auxiliary heap space for push_relabel().

*/

#include <algorithm>
#include <vector>

const int MAXN = 100, INF = 0x3f3f3f3f;
int cap[MAXN][MAXN], f[MAXN][MAXN];

int push_relabel(int nodes, int source, int sink) {
  std::vector<int> e(nodes, 0), h(nodes, 0), maxh(nodes, 0);
  for (int i = 0; i < nodes; i++) {
    std::fill(f[i], f[i] + nodes, 0);
  }
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
  int max_flow = 0;
  for (int i = 0; i < nodes; i++) {
    max_flow += f[source][i];
  }
  return max_flow;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  cap[0][1] = 3;
  cap[0][2] = 3;
  cap[1][2] = 2;
  cap[1][3] = 3;
  cap[2][4] = 2;
  cap[3][4] = 1;
  cap[3][5] = 2;
  cap[4][5] = 3;
  assert(push_relabel(6, 0, 5) == 5);
  return 0;
}
