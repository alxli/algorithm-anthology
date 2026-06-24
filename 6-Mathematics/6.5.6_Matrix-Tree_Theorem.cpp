/*

Kirchhoff's matrix-tree theorem counts the spanning trees of an undirected graph. Form the Laplacian
matrix $L = D - A$, where $D$ is the diagonal matrix of degrees and $A$ is the adjacency matrix
(with multiplicities for parallel edges). Every cofactor of $L$ is equal, and that common value is
the number of spanning trees. Equivalently, delete any one row and the matching column from $L$ and
take the determinant of what remains.

The determinant is evaluated modulo a prime by Gaussian elimination, so the count is returned modulo
`MOD`. Spanning-tree counts grow astronomically, so a modular count is the usual contest form; run
the routine under a few different primes and combine with the Chinese remainder theorem to recover a
large exact value. The same construction handles weighted graphs (sum edge weights into the
Laplacian to get the weighted tree count) and, with the directed Laplacian and a fixed deleted row,
counts rooted spanning arborescences.

- `count_spanning_trees(n, edges)` returns the number of spanning trees of the undirected graph on
  nodes $[0, `n`)$ with the given edge list, modulo `MOD`. Parallel edges are honored; self-loops do
  not affect the count. The result is $0$ when the graph is disconnected, and $1$ when `n` is $1$.

Time Complexity:
- O(n^3) per call, where $n$ is the number of nodes.

Space Complexity:
- O(n^2) auxiliary heap space.

*/

#include <cstdint>
#include <utility>
#include <vector>

const int64_t MOD = 998244353;

int64_t powmod(int64_t b, int64_t e, int64_t m) {
  int64_t res = 1;
  for (b %= m; e > 0; e >>= 1) {
    if (e & 1) {
      res = res * b % m;
    }
    b = b * b % m;
  }
  return res;
}

int64_t count_spanning_trees(int n, const std::vector<std::pair<int, int>> &edges) {
  if (n <= 1) {
    return n == 1 ? 1 : 0;
  }
  // Build the Laplacian, then drop the last row and column to form an (n-1) by (n-1) cofactor.
  int m = n - 1;
  std::vector<std::vector<int64_t>> lap(m, std::vector<int64_t>(m, 0));
  for (const auto &e : edges) {
    int u = e.first, v = e.second;
    if (u == v) {
      continue;  // Self-loops contribute nothing to the Laplacian.
    }
    if (u < m) {
      lap[u][u] = (lap[u][u] + 1) % MOD;
    }
    if (v < m) {
      lap[v][v] = (lap[v][v] + 1) % MOD;
    }
    if (u < m && v < m) {
      lap[u][v] = (lap[u][v] - 1 + MOD) % MOD;
      lap[v][u] = (lap[v][u] - 1 + MOD) % MOD;
    }
  }
  // Determinant modulo MOD by Gaussian elimination.
  int64_t det = 1;
  for (int col = 0; col < m; col++) {
    int pivot = -1;
    for (int r = col; r < m; r++) {
      if (lap[r][col] != 0) {
        pivot = r;
        break;
      }
    }
    if (pivot == -1) {
      return 0;  // Singular cofactor: the graph is disconnected.
    }
    if (pivot != col) {
      std::swap(lap[pivot], lap[col]);
      det = (MOD - det) % MOD;
    }
    det = det * lap[col][col] % MOD;
    int64_t pinv = powmod(lap[col][col], MOD - 2, MOD);
    for (int r = col + 1; r < m; r++) {
      int64_t factor = lap[r][col] * pinv % MOD;
      if (factor == 0) {
        continue;
      }
      for (int k = col; k < m; k++) {
        lap[r][k] = (lap[r][k] - factor * lap[col][k]) % MOD;
        if (lap[r][k] < 0) {
          lap[r][k] += MOD;
        }
      }
    }
  }
  return det;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // A path 0-1-2 and a single triangle each have a known number of spanning trees.
  assert(count_spanning_trees(3, {{0, 1}, {1, 2}}) == 1);          // A tree has exactly one.
  assert(count_spanning_trees(3, {{0, 1}, {1, 2}, {2, 0}}) == 3);  // Triangle: drop any one edge.

  // Complete graphs: Cayley's formula gives n^(n-2) spanning trees.
  auto complete = [](int n) {
    vector<pair<int, int>> e;
    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        e.emplace_back(i, j);
      }
    }
    return count_spanning_trees(n, e);
  };
  assert(complete(4) == 16);   // 4^2.
  assert(complete(5) == 125);  // 5^3.

  // Disconnected graphs have no spanning tree.
  assert(count_spanning_trees(4, {{0, 1}, {2, 3}}) == 0);
  return 0;
}
