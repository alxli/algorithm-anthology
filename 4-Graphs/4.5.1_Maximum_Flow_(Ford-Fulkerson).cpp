/*

Given a flow network with integer capacities, find the maximum flow from a given source node to a
given sink node. The flow along each edge may not exceed its capacity, and flow is conserved at
every node other than the source and sink.

The Ford-Fulkerson method repeatedly finds any augmenting path from source to sink in the residual
graph (here by depth-first search) and pushes as much flow as the path allows, until no augmenting
path remains. This algorithm should only be used on graphs with integer capacities, as there exists
certain real-valued flow inputs for which the algorithm never terminates. The Edmonds-Karp algorithm
is an improvement using breadth-first search, addressing this problem.

- `ford_fulkerson()` uses global `source` and `sink`, modifies the global residual capacity matrix
  `cap`, and returns maximum flow. Nodes are numbered $[0, `n`)$, where `n` is `cap.size()`.

Time Complexity:
- O(n^2*f) per call, where $n$ is the number of nodes and $f$ is the maximum flow.

Space Complexity:
- O(n^2) for storage of the flow network, where $n$ is the number of nodes.
- O(n) auxiliary stack space for `ford_fulkerson()`.

*/

#include <algorithm>
#include <climits>
#include <cstdint>
#include <vector>

const int INF = INT_MAX / 2;
std::vector<std::vector<int>> cap;
std::vector<char> visit;

int dfs(int u, int f, int sink) {
  int n = static_cast<int>(cap.size());
  if (u == sink) {
    return f;
  }
  visit[u] = true;
  for (int v = 0; v < n; v++) {
    if (!visit[v] && cap[u][v] > 0) {
      int flow = dfs(v, std::min(f, cap[u][v]), sink);
      if (flow > 0) {
        cap[u][v] -= flow;
        cap[v][u] += flow;
        return flow;
      }
    }
  }
  return 0;
}

int64_t ford_fulkerson(int source, int sink) {
  int n = static_cast<int>(cap.size());
  int64_t max_flow = 0;
  while (true) {
    visit.assign(n, false);
    int flow = dfs(source, INF, sink);
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
  assert(ford_fulkerson(0, 5) == 5);
  return 0;
}
