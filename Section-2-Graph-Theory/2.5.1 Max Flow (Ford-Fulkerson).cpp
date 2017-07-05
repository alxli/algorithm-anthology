/*

Given a flow network with integer capacities, find the maximum flow from a given
source node to a given sink node. The flow of a given edge u -> v is defined as
the minimum of its capacity and the sum of the flows of all incoming edges of u.
ford_fulkerson() applies to global variables nodes, source, sink, and cap[][]
which is an adjacency matrix that will be modified by the function call.

The Ford-Fulkerson algorithm is only optimal on graphs with integer capacities,
as there exists certain real-valued flow inputs for which the algorithm never
terminates. The Edmonds-Karp algorithm is an improvement using breadth-first
search, addressing this problem.

Time Complexity:
- O(n^2*f) per call to ford_fulkerson(), where n is the number of nodes and f
  is the maximum flow.

Space Complexity:
- O(n^2) for storage of the flow network, where n is the number of nodes.
- O(n) auxiliary stack space for ford_fulkerson().

*/

#include <algorithm>
#include <vector>

const int MAXN = 100, INF = 0x3f3f3f3f;
int nodes, source, sink, cap[MAXN][MAXN];
std::vector<bool> visit(MAXN);

int dfs(int u, int f) {
  if (u == sink) {
    return f;
  }
  visit[u] = true;
  for (int v = 0; v < nodes; v++) {
    if (!visit[v] && cap[u][v] > 0) {
      int flow = dfs(v, std::min(f, cap[u][v]));
      if (flow > 0) {
        cap[u][v] -= flow;
        cap[v][u] += flow;
        return flow;
      }
    }
  }
  return 0;
}

int ford_fulkerson() {
  int max_flow = 0;
  for (;;) {
    std::fill(visit.begin(), visit.end(), false);
    int flow = dfs(source, INF);
    if (flow == 0) {
      break;
    }
    max_flow += flow;
  }
  return max_flow;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  nodes = 6;
  source = 0;
  sink = 5;
  cap[0][1] = 3;
  cap[0][2] = 3;
  cap[1][2] = 2;
  cap[1][3] = 3;
  cap[2][4] = 2;
  cap[3][4] = 1;
  cap[3][5] = 2;
  cap[4][5] = 3;
  assert(ford_fulkerson() == 5);
  return 0;
}
