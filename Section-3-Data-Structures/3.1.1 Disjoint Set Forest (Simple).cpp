/*

3.1.1 - Disjoint Set Forest (Simple)

Description: This data structure dynamically keeps track
of items partitioned into non-overlapping sets (a disjoint
set forest). It is also known as a union-find data structure.

Time Complexity: Every function below is O(a(N)) amortized
on the number of items in the set due to the optimizations
of union by rank and path compression. Here, a(N) is the
extremely slow growing inverse of the Ackermann function.
For all practical values of n, a(n) is less than 5.

Space Complexity: O(N) total.

*/

const int MAXN = 1000;
int num_sets = 0, root[MAXN+1], rank[MAXN+1];

int find_root(int x) {
  if (root[x] != x) root[x] = find_root(root[x]);
  return root[x];
}

void make_set(int x) {
  root[x] = x;
  rank[x] = 0;
  num_sets++;
}

bool is_united(int x, int y) {
  return find_root(x) == find_root(y);
}

void unite(int x, int y) {
  int X = find_root(x), Y = find_root(y);
  if (X == Y) return;
  num_sets--;
  if (rank[X] < rank[Y]) root[X] = Y;
  else if (rank[X] > rank[Y]) root[Y] = X;
  else rank[root[Y] = X]++;
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  for (char c = 'a'; c <= 'g'; c++) make_set(c);
  unite('a', 'b');
  unite('b', 'f');
  unite('d', 'e');
  unite('e', 'g');
  assert(num_sets == 3);
  assert(is_united('a', 'b'));
  assert(!is_united('a', 'c'));
  assert(!is_united('b', 'g'));
  assert(is_united('d', 'g'));
  return 0;
}
