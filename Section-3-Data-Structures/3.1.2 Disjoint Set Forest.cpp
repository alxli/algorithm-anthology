/*

Description: This data structure dynamically keeps track
of items partitioned into non-overlapping sets (a disjoint
set forest). It is also known as a union-find data structure.
This particular templatized version employs an std::map for
built in storage and coordinate compression. That is, the
magnitude of values inserted is not limited.

Time Complexity: make_set(), unite() and is_united() are
O(a(N) + log N) = O(log N) on the number of elements in the
disjoint set forest. get_all_sets() is O(N). find() is is
O(a(N)) amortized on the number of items in the set due to
the optimizations of union by rank and path compression.
Here, a(N) is the extremely slow growing inverse of the
Ackermann function. For all practical values of n, a(n) is
less than 5.

Space Complexity: O(N) storage and auxiliary.

=~=~=~=~= Sample Output =~=~=~=~=
Elements: 7, Sets: 3
[[a,b,f],[c],[d,e,g]]

*/

#include <map>
#include <vector>

template<class T> class disjoint_set_forest {
  int num_elements, num_sets;
  std::map<T, int> ID;
  std::vector<int> root, rank;

  int find_root(int x) {
    if (root[x] != x) root[x] = find_root(root[x]);
    return root[x];
  }

 public:
  disjoint_set_forest(): num_elements(0), num_sets(0) {}
  int elements() { return num_elements; }
  int sets() { return num_sets; }

  bool is_united(const T & x, const T & y) {
    return find_root(ID[x]) == find_root(ID[y]);
  }

  void make_set(const T & x) {
    if (ID.find(x) != ID.end()) return;
    root.push_back(ID[x] = num_elements++);
    rank.push_back(0);
    num_sets++;
  }

  void unite(const T & x, const T & y) {
    int X = find_root(ID[x]), Y = find_root(ID[y]);
    if (X == Y) return;
    num_sets--;
    if (rank[X] < rank[Y]) root[X] = Y;
    else if (rank[X] > rank[Y]) root[Y] = X;
    else rank[root[Y] = X]++;
  }

  std::vector<std::vector<T> > get_all_sets() {
    std::map<int, std::vector<T> > tmp;
    for (typename std::map<T, int>::iterator
         it = ID.begin(); it != ID.end(); it++)
      tmp[find_root(it->second)].push_back(it->first);
    std::vector<std::vector<T> > ret;
    for (typename std::map<int, std::vector<T> >::
         iterator it = tmp.begin(); it != tmp.end(); it++)
      ret.push_back(it->second);
    return ret;
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  disjoint_set_forest<char> d;
  for (char c = 'a'; c <= 'g'; c++) d.make_set(c);
  d.unite('a', 'b');
  d.unite('b', 'f');
  d.unite('d', 'e');
  d.unite('e', 'g');
  cout << "Elements: " << d.elements();
  cout << ", Sets: " << d.sets() << endl;
  vector<vector<char> > s = d.get_all_sets();
  cout << "[";
  for (int i = 0; i < (int)s.size(); i++) {
    cout << (i > 0 ? ",[" : "[");
    for (int j = 0; j < (int)s[i].size(); j++)
      cout << (j > 0 ? "," : "") << s[i][j];
    cout << "]";
  }
  cout << "]\n";
  return 0;
}
