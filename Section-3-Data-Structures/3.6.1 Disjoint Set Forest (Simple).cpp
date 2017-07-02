/*

Maintain a set of elements partitioned into non-overlapping subsets. Each
partition is assigned a unique representative known as the parent, or root. The
following implements two well-known optimizations known as union-by-rank and
path compression. This version is simplified to only work on integer elements.

- init_dsf() resets the data structure.
- make_set(x) creates a new partition consisting of the single element x, which
  must not have been previously added to the data structure.
- find_root(x) returns the unique representative of the partition containing x.
- is_united(x, y) returns whether elements x and y belong to the same partition.
- unite(x, y) replaces the partitions containing x and y with a single new
  partition consisting of the union of elements in the original partitions.

A precondition to the last three operations is that make_set() must have been
previously called on their arguments.

Time Complexity:
- O(1) per call to init_dsf() and make_set().
- O(a(n)) per call to find_root(), is_united(), and unite(), where n is the
  number of elements that has been added via make_set() so far, and a(n) is the
  extremely slow growing inverse of the Ackermann function (effectively a very
  small constant for all practical values of n).

Space Complexity:
- O(n) for storage of the disjoint set forest elements.
- O(1) auxiliary per call to all operations.

*/

const int MAXN = 1000;
int num_sets, root[MAXN], rank[MAXN];

void init_dsf() {
  num_sets = 0;
}

void make_set(int x) {
  root[x] = x;
  rank[x] = 0;
  num_sets++;
}

int find_root(int x) {
  if (root[x] != x) {
    root[x] = find_root(root[x]);
  }
  return root[x];
}

bool is_united(int x, int y) {
  return find_root(x) == find_root(y);
}

void unite(int x, int y) {
  int r1 = find_root(x);
  int r2 = find_root(y);
  if (r1 == r2)
    return;
  num_sets--;
  if (rank[r1] < rank[r2]) {
    root[r1] = r2;
  } else {
    root[r2] = r1;
    if (rank[r1] == rank[r2]) {
      rank[r1]++;
    }
  }
}

/*** Example Usage ***/

#include <cassert>

int main() {
  init_dsf();
  for (char c = 'a'; c <= 'g'; c++) {
    make_set(c);
  }
  unite('a', 'b');
  unite('b', 'f');
  unite('d', 'e');
  unite('d', 'g');
  assert(num_sets == 3);
  assert(is_united('a', 'b'));
  assert(!is_united('a', 'c'));
  assert(!is_united('b', 'g'));
  assert(is_united('e', 'g'));
  return 0;
}
