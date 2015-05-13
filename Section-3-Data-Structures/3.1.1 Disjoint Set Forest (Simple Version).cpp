/*

3.1.1 - Disjoint Set Forest (Simple Version for Ints)

Description: This data structure dynamically keeps track
of items partitioned into non-overlapping sets.

Time Complexity: Every function below is O(α(N))
amortized on the number of items in the set, due to the
optimizations of union by rank and path compression.
α(N) is the extremely slow growing inverse of the
Ackermann function. α(n) < 5 for all practical values of n.

Space Complexity: O(N) total.

*/

const int MAX_N = 1001;
int num_sets = 0, root[MAX_N], rank[MAX_N];

int find_root(int x) {
    if (root[x] != x) root[x] = find_root(root[x]);
    return root[x];
}

void make_set(int x) {
    root[i] = i; rank[i] = 0; num_sets++;
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

#include <iostream>
using namespace std;

int main() {
  disjoint_set_forest<char> d;
  for (char c='a'; c<='g'; c++) d.make_set(c);
  d.unite('a', 'b'); d.unite('b', 'f');
  d.unite('d', 'e'); d.unite('e', 'g');
  cout << "Elements: " << d.elements();
  cout << ", Sets: " << d.sets() << endl;
  vector< vector<char> > V = d.get_all_sets();
  for (int i = 0; i < V.size(); i++) {
    cout << "{ ";
    for (int j = 0; j < V[i].size(); j++)
      cout << V[i][j] << " ";
    cout << "}";
  }
  return 0;
}

/*
=~=~=~=~= Sample Output =~=~=~=~=
Elements: 7, Sets: 3
{ a b f }{ c }{ d e g }
*/
