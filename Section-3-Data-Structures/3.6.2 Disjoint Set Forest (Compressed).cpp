/*

Maintain a set of elements partitioned into non-overlapping subsets using a
collection of trees. Each partition is assigned a unique representative known as
the parent, or root. The following implements two well-known optimizations known
as union-by-rank and path compression. This version uses an std::map for storage
and coordinate compression (thus, element types must meet the requirements of
key types for std::map).

- make_set(u) creates a new partition consisting of the single element u, which
  must not have been previously added to the data structure.
- is_united(u, v) returns whether elements u and v belong to the same partition.
- unite(u, v) replaces the partitions containing u and v with a single new
  partition consisting of the union of elements in the original partitions.
- get_all_sets() returns all current partitions as a vector of vectors.

A precondition to the last three operations is that make_set() must have been
previously called on their arguments.

Time Complexity:
- O(1) per call to the constructor.
- O(log n) per call to make_set(), where n is the number of elements that have
  been added via make_set() so far.
- O(a(n) log n) per call to is_united() and unite(), where n is the number of
  elements that have been added via make_set() so far, and a(n) is the extremely
  slow growing inverse of the Ackermann function (effectively a very small
  constant for all practical values of n).
- O(n) per call to get_all_sets().

Space Complexity:
- O(n) for storage of the disjoint set forest elements.
- O(n) auxiliary heap space for get_all_sets().
- O(1) auxiliary for all other operations.

*/

#include <map>
#include <vector>

template<class T>
class disjoint_set_forest {
  int num_elements, num_sets;
  std::map<T, int> id;
  std::vector<int> root, rank;

  int find_root(int u) {
    if (root[u] != u) {
      root[u] = find_root(root[u]);
    }
    return root[u];
  }

 public:
  disjoint_set_forest() : num_elements(0), num_sets(0) {}

  int size() const {
    return num_elements;
  }

  int sets() const {
    return num_sets;
  }

  void make_set(const T &u) {
    if (id.find(u) != id.end()) {
      return;
    }
    id[u] = num_elements;
    root.push_back(num_elements++);
    rank.push_back(0);
    num_sets++;
  }

  bool is_united(const T &u, const T &v) {
    return find_root(id[u]) == find_root(id[v]);
  }

  void unite(const T &u, const T &v) {
    int ru = find_root(id[u]), rv = find_root(id[v]);
    if (ru == rv) {
      return;
    }
    num_sets--;
    if (rank[ru] < rank[rv]) {
      root[ru] = rv;
    } else {
      root[rv] = ru;
      if (rank[ru] == rank[rv]) {
        rank[ru]++;
      }
    }
  }

  std::vector<std::vector<T> > get_all_sets() {
    std::map<int, std::vector<T> > tmp;
    for (typename std::map<T, int>::iterator it = id.begin(); it != id.end();
         ++it) {
      tmp[find_root(it->second)].push_back(it->first);
    }
    std::vector<std::vector<T> > res;
    for (typename std::map<int, std::vector<T> >::iterator it = tmp.begin();
         it != tmp.end(); ++it) {
      res.push_back(it->second);
    }
    return res;
  }
};

/*** Example Usage and Output:

[a, b, f], [c], [d, e, g]

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  disjoint_set_forest<char> dsf;
  for (char c = 'a'; c <= 'g'; c++) {
    dsf.make_set(c);
  }
  dsf.unite('a', 'b');
  dsf.unite('b', 'f');
  dsf.unite('d', 'e');
  dsf.unite('d', 'g');
  assert(dsf.size() == 7);
  assert(dsf.sets() == 3);
  vector< vector<char> > s = dsf.get_all_sets();
  for (int i = 0; i < (int)s.size(); i++) {
    cout << (i > 0 ? ", [" : "[");
    for (int j = 0; j < (int)s[i].size(); j++) {
      cout << (j > 0 ? ", " : "") << s[i][j];
    }
    cout << "]";
  }
  cout << endl;
  return 0;
}
