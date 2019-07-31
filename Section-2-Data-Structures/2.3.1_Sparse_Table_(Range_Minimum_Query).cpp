/*

Given a static array with indices from 0 to n - 1, precompute a table that may
later be used perform range minimum queries on the array in constant time. This
version is simplified to only work on integer arrays.

The dynamic programming state dp[i][j] holds the index of the minimum value in
the sub-array starting at i and having length 2^j. Each dp[i][j] will always be
equal to either dp[i][j - 1] or dp[i + 2^(j - 1) - 1)][j - 1], whichever of the
indices corresponds to the smaller value in the array.

Time Complexity:
- O(n log n) per call to build(), where n is the size of the array.
- O(1) per call to query_min().

Space Complexity:
- O(n log n) for storage of the sparse table, where n is the size of the array.
- O(1) auxiliary for query().

*/

#include <vector>

const int MAXN = 1000;
std::vector<int> table, dp[MAXN];

void build(int n, int a[]) {
  table.resize(n + 1);
  for (int i = 2; i <= n; i++) {
    table[i] = table[i >> 1] + 1;
  }
  for (int i = 0; i < n; i++) {
    dp[i].resize(table[n] + 1);
    dp[i][0] = i;
  }
  for (int j = 1; (1 << j) < n; j++) {
    for (int i = 0; i + (1 << j) <= n; i++) {
      int x = dp[i][j - 1];
      int y = dp[i + (1 << (j - 1))][j - 1];
      dp[i][j] = (a[x] < a[y]) ? x : y;
    }
  }
}

int query(int a[], int lo, int hi) {
  int j = table[hi - lo];
  int x = dp[lo][j];
  int y = dp[hi - (1 << j) + 1][j];
  return (a[x] < a[y]) ? a[x] : a[y];
}

/*** Example Usage ***/

#include <cassert>

int main() {
  int arr[5] = {6, -2, 1, 8, 10};
  build(5, arr);
  assert(query(arr, 0, 3) == -2);
  return 0;
}
