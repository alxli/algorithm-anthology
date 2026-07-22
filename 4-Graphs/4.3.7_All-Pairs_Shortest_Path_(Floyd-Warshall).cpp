/*

Given a weighted, directed graph with possibly negative weights, determine the minimum distance
between all pairs of start and destination nodes in the graph. Optionally, reconstruct a shortest
path between two nodes using the precomputed next-hop matrix `next_node`.

Floyd-Warshall's algorithm is a dynamic program over intermediate nodes: for each node $k$ in turn,
every pair $(i, j)$ is relaxed by considering a path through $k$, so once all $k$ have been
processed the matrix holds the all-pairs shortest distances.

- `init_floyd(n)` initializes `dist` and `next_node` for a graph of `n` nodes numbered $[0, `n`)$.
- `floyd_warshall()` updates the global adjacency matrix `dist` so `dist[u][v]` stores the shortest
  path from $u$ to $v$, and updates `next_node` for path reconstruction. If the graph contains
  negative-weighted cycles, there is no shortest path and an error will be thrown.
- `get_path(start, dest)` returns the shortest path from `start` to `dest`, or an empty vector if
  `dest` is unreachable from `start`, after the latest call to `floyd_warshall()`.

For path reconstruction, `next_node[i][j]` stores the next node to visit after `i` on a current
shortest path from `i` to `j`. It is initialized to `j` for every pair and, when a shorter route `i`
$\to$ `k` $\to$ `j` is found, becomes `next_node[i][k]`. Repeatedly setting `i` to `next_node[i][j]`
therefore walks the path from source to destination. This value is meaningful only when
`dist[i][j] != INF`.

Time Complexity:
- O(n^2) per call to `init_floyd()`, where $n$ is the number of nodes.
- O(n^3) per call to `floyd_warshall()`.
- O(p) per call to `get_path()`, where $p$ is the number of nodes in the returned path.

Space Complexity:
- O(n^2) for storage of the graph, where $n$ is the number of nodes.
- O(1) auxiliary per call to `floyd_warshall()`.
- O(p) for the path returned by `get_path()`.

*/

#include <cstdint>
#include <stdexcept>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::vector<int64_t>> dist;
std::vector<std::vector<int>> next_node;

void init_floyd(int n) {
  dist.assign(n, std::vector<int64_t>(n));
  next_node.assign(n, std::vector<int>(n));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      dist[i][j] = (i == j) ? 0 : INF;
      next_node[i][j] = j;
    }
  }
}

void floyd_warshall() {
  int n = static_cast<int>(dist.size());
  for (int k = 0; k < n; k++) {
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        // The INF guards avoid relaxing through an unreachable intermediate: with a negative edge,
        // INF + w < INF would otherwise give an unreachable pair a bogus finite distance.
        if (dist[i][k] != INF && dist[k][j] != INF && dist[i][j] > dist[i][k] + dist[k][j]) {
          dist[i][j] = dist[i][k] + dist[k][j];
          next_node[i][j] = next_node[i][k];
        }
      }
    }
  }
  // Optional: Report negative-weighted cycles.
  for (int i = 0; i < n; i++) {
    if (dist[i][i] < 0) {
      throw std::runtime_error("Negative-weight cycle found.");
    }
  }
}

std::vector<int> get_path(int start, int dest) {
  if (dist[start][dest] == INF) {
    return {};
  }
  std::vector<int> path{start};
  while (start != dest) {
    start = next_node[start][dest];
    path.push_back(start);
  }
  return path;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  //    w=1      w=2
  // 0 -----> 1 -----> 2
  // |                 ^
  // +-----------------+
  //         w=5
  init_floyd(3);
  dist[0][1] = 1;
  dist[1][2] = 2;
  dist[0][2] = 5;
  floyd_warshall();
  int start = 0, dest = 2;
  assert(dist[start][dest] == 3);
  assert((get_path(start, dest) == vector<int>{0, 1, 2}));
  return 0;
}
