/*

Given a flow network with integer capacities, find the maximum flow from a given source node to a
given sink node. The flow along each edge may not exceed its capacity, and flow is conserved at
every node other than the source and sink.

- `ford_fulkerson()` uses global `source` and `sink`, modifies the global residual capacity matrix
  `cap`, and returns maximum flow. Nodes are numbered from 0 to `cap.size() - 1`.

The Ford-Fulkerson algorithm should only be used on graphs with integer capacities, as there exists
certain real-valued flow inputs for which the algorithm never terminates. The Edmonds-Karp algorithm
is an improvement using breadth-first search, addressing this problem.

Time Complexity:
- O(n^2*f) per call to `ford_fulkerson()`, where $n$ is the number of nodes and $f$ is the maximum
  flow.

Space Complexity:
- O(n^2) for storage of the flow network, where $n$ is the number of nodes.
- O(n) auxiliary stack space for `ford_fulkerson()`.

*/

#include <algorithm>
#include <climits>
#include <vector>

const int INF = INT_MAX / 2;
int source, sink;
std::vector<std::vector<int>> cap;
std::vector<bool> visit;

int dfs(int u, int f) {
  int nodes = cap.size();
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

long long ford_fulkerson() {
  int nodes = cap.size();
  long long max_flow = 0;
  for (;;) {
    visit.assign(nodes, false);
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
  int nodes = 6;
  source = 0;
  sink = 5;
  cap.assign(nodes, std::vector<int>(nodes));
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
