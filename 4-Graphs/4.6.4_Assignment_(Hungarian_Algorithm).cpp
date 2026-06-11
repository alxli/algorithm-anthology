/*

Solves the minimum-cost assignment problem for a rectangular cost matrix. Given $n$ workers and $m$
jobs (with $n \leq m$) where assigning worker `i` to complete job `j` would cost `cost[i][j]`,
choose a distinct job for each worker so that the total assignment cost is minimized.

The classical Hungarian algorithm using potentials transforms a cost matrix using the principle that
adding or subtracting constants from rows or columns does not change the optimal assignment. Workers
are assigned one row at a time: each new row triggers a search for a least-cost augmenting path over
zero-slack edges, with the potentials adjusted along the way to expose new zero-slack edges until
the path reaches a free job. Note that while the input matrix is 0-based here, the internal
calculations are 1-based.

- `hungarian(cost, &assignment)` returns the minimum total assignment cost for a 0-based `cost`
  matrix, while populating `assignment` with $n$ values where `assignment[i]` is the chosen job for
  worker `i`, using 0-based job numbers.

Time Complexity:
- O(n^2*m), where `cost` has $n$ rows and $m$ columns.

Space Complexity:
- O(n + m) auxiliary heap space, not counting the input matrix and returned assignment.

*/

#include <algorithm>
#include <cassert>
#include <climits>
#include <vector>

long long hungarian(const std::vector<std::vector<long long>> &cost, std::vector<int> *assignment) {
  int n = static_cast<int>(cost.size());
  int m = cost.empty() ? 0 : static_cast<int>(cost[0].size());
  assert(n <= m);
  const long long INF = LLONG_MAX / 4;
  std::vector<long long> u(n + 1), v(m + 1);
  std::vector<int> p(m + 1), way(m + 1);
  for (int i = 1; i <= n; i++) {
    p[0] = i;
    int j0 = 0;
    std::vector<long long> minv(m + 1, INF);
    std::vector<char> used(m + 1, false);
    do {
      used[j0] = true;
      int i0 = p[j0], j1 = 0;
      long long delta = INF;
      for (int j = 1; j <= m; j++) {
        if (used[j]) {
          continue;
        }
        long long cur = cost[i0 - 1][j - 1] - u[i0] - v[j];
        if (cur < minv[j]) {
          minv[j] = cur;
          way[j] = j0;
        }
        if (minv[j] < delta) {
          delta = minv[j];
          j1 = j;
        }
      }
      for (int j = 0; j <= m; j++) {
        if (used[j]) {
          u[p[j]] += delta;
          v[j] -= delta;
        } else {
          minv[j] -= delta;
        }
      }
      j0 = j1;
    } while (p[j0] != 0);
    do {
      int j1 = way[j0];
      p[j0] = p[j1];
      j0 = j1;
    } while (j0 != 0);
  }
  assignment->assign(n, -1);
  for (int j = 1; j <= m; j++) {
    if (p[j] != 0) {
      (*assignment)[p[j] - 1] = j - 1;
    }
  }
  return -v[0];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<long long>> cost{
      {9, 2, 7, 8},
      {6, 4, 3, 7},
      {5, 8, 1, 8},
  };
  vector<int> assignment;
  assert(hungarian(cost, &assignment) == 9);
  assert(assignment[0] == 1);
  assert(assignment[1] == 0);
  assert(assignment[2] == 2);
  return 0;
}
