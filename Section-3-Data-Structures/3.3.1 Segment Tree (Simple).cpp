/*

Description: A segment tree is a data structure used for
solving the dynamic range query problem, which asks to
determine the minimum (or maximum) value in any given
range in an array that is constantly being updated.

Time Complexity: Assuming merge() is O(1), build is O(n)
while query() and update() are O(log n). If merge() is
not O(1), then all running times are multiplied by a
factor of whatever complexity merge() runs in.

Space Complexity: O(MAXN). Note that a segment tree with
N leaves requires 2^(log2(N) - 1) = 4*N total nodes.

Note: This implementation is 0-based, meaning that all
indices from 0 to MAXN - 1, inclusive, are accessible.

=~=~=~=~= Sample Input =~=~=~=~=
5 10
35232
390942
649675
224475
18709
Q 1 3
M 4 475689
Q 2 3
Q 1 3
Q 1 2
Q 3 3
Q 2 3
M 2 645514
M 2 680746
Q 0 4

=~=~=~=~= Sample Output =~=~=~=~=
224475
224475
224475
390942
224475
224475
35232

*/

const int MAXN = 100000;
int N, M, a[MAXN], t[4*MAXN];

//define your custom nullv and merge() below.
//merge(x, nullv) must return x for all x

const int nullv = 1 << 30;

inline int merge(int a, int b) { return a < b ? a : b; }

void build(int n, int lo, int hi) {
  if (lo == hi) {
    t[n] = a[lo];
    return;
  }
  build(2*n + 1, lo, (lo + hi)/2);
  build(2*n + 2, (lo + hi)/2 + 1, hi);
  t[n] = merge(t[2*n + 1], t[2*n + 2]);
}

//x and y must be manually set before each call to the
//functions below. For query(), [x, y] is the range to
//be considered. For update(), a[x] is to be set to y.
int x, y;

//merge(a[i] for i = x..y, inclusive)
int query(int n, int lo, int hi) {
  if (hi < x || lo > y) return nullv;
  if (lo >= x && hi <= y) return t[n];
  return merge(query(2*n + 1, lo, (lo + hi) / 2),
               query(2*n + 2, (lo + hi) / 2 + 1, hi));
}

//a[x] = y
void update(int n, int lo, int hi) {
  if (hi < x || lo > x) return;
  if (lo == hi) {
    t[n] = y;
    return;
  }
  update(2*n + 1, lo, (lo + hi)/2);
  update(2*n + 2, (lo + hi)/2 + 1, hi);
  t[n] = merge(t[2*n + 1], t[2*n + 2]);
}

/*** Example Usage (wcipeg.com/problem/segtree) ***/

#include <cstdio>

int main() {
  scanf("%d%d", &N, &M);
  for (int i = 0; i < N; i++) scanf("%d", &a[i]);
  build(0, 0, N - 1);
  char op;
  for (int i = 0; i < M; i++) {
    scanf(" %c%d%d", &op, &x, &y);
    if (op == 'Q') {
      printf("%d\n", query(0, 0, N - 1));
    } else if (op == 'M') {
      update(0, 0, N - 1);
    }
  }
  return 0;
}
