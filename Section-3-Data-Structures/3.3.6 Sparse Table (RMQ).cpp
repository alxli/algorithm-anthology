/*

Description: The static range minimum query problem can be solved
using a sparse table data structure. The RMQ for sub arrays of
length 2^k is pre-processed using dynamic programming with formula:

dp[i][j] = dp[i][j-1], if A[dp[i][j-1]] <= A[dp[i+2^(j-1)-1][j-1]]
           dp[i+2^(j-1)-1][j-1], otherwise

where dp[i][j] is the index of the minimum value in the sub array
starting at i having length 2^j.

Time Complexity: O(N log N) for build() and O(1) for min_idx()
Space Complexity: O(N log N) on the size of the array.

Note: This implementation is 0-based, meaning that all
indices from 0 to N - 1, inclusive, are valid.

*/

#include <vector>

const int MAXN = 100;
std::vector<int> logtable, dp[MAXN];

void build(int n, int a[]) {
  logtable.resize(n + 1);
  for (int i = 2; i <= n; i++)
    logtable[i] = logtable[i >> 1] + 1;
  for (int i = 0; i < n; i++) {
    dp[i].resize(logtable[n] + 1);
    dp[i][0] = i;
  }
  for (int k = 1; (1 << k) < n; k++) {
    for (int i = 0; i + (1 << k) <= n; i++) {
      int x = dp[i][k - 1];
      int y = dp[i + (1 << (k - 1))][k - 1];
      dp[i][k] = a[x] <= a[y] ? x : y;
    }
  }
}

//returns index of min element in [lo, hi]
int min_idx(int a[], int lo, int hi) {
  int k = logtable[hi - lo];
  int x = dp[lo][k];
  int y = dp[hi - (1 << k) + 1][k];
  return a[x] <= a[y] ? x : y;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int a[] = {7, -10, 5, 20};
  build(4, a);
  cout << min_idx(a, 0, 3) << "\n"; //1
  return 0;
}
