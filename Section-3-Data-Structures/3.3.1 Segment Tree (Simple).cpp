/*

3.3.1 - 1D Segment Tree (Simple Version for ints)

Description: A segment tree is a data structure used for
solving the dynamic range query problem, which asks to
determine the minimum (or maximum) value in any given
range in an array that is constantly being updated.

Time Complexity: Assuming merge() is O(1), build is O(N)
                 while query() and update() are O(log(N)).

Space Complexity: O(N) on the size of the array. A segment
                  tree needs 2^(log2(N)-1) = 4N nodes.

Note: This implementation is 0-based, meaning that all
indices from 0 to N - 1, inclusive, are accessible.

*/

const int MAXN = 100000;
int N, M, a[MAXN], t[4*MAXN];

//merge(x, INF) must return x for all x
const int INF = 1 << 30;
int merge(int a, int b) {
  return (a < b) ? a : b;
}

void build(int n, int lo, int hi) { //update a[lo..hi]
  if (lo == hi) {
    t[n] = a[lo];
    return;
  }
  build(2*n + 1, lo, (lo + hi)/2);
  build(2*n + 2, (lo + hi)/2 + 1, hi);
  t[n] = merge(t[2*n + 1], t[2*n + 2]);
}

int A, B; //must be set before calling below functions

int query(int n, int lo, int hi) { //merge(a[i] for i = A..B)
  if (hi < A || lo > B) return INF;
  if (lo >= A && hi <= B) return t[n];
  return merge(query(2*n + 1, lo, (lo + hi)/2),
               query(2*n + 2, (lo + hi)/2 + 1, hi));
}

void update(int n, int lo, int hi) { //a[A] = B
  if (hi < A || lo > A) return;
  if (lo == hi) {
    t[n] = B;
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
    scanf(" %c%d%d", &op, &A, &B);
    if (op == 'Q') {
      printf("%d\n", query(0, 0, N - 1));
    } else if (op == 'M') {
      update(0, 0, N - 1);
    }
  }
  return 0;
}
