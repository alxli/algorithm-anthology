/*

Maintain a set of elements partitioned into non-overlapping subsets using a collection of trees.
Each partition is assigned a unique representative known as the parent, or root. Each subset is
stored as a tree whose nodes point toward its root: finding a representative follows parent
pointers, redirecting visited nodes straight to the root along the way (path compression), while
unions attach the shallower tree beneath the root of the deeper one (union-by-rank). Union-by-rank
is interchangeable with union-by-size as used by the plain Disjoint Set Union, yielding the same
complexity bounds; rank stores only the tree-depth estimate, so it cannot answer set-size queries.

This version uses an `std::unordered_map` for storage and coordinate compression (thus, element
types must meet the requirements of key types for `std::unordered_map`). The order of sets returned
by `get_all_sets()` is unspecified.

- `SparseDSU<T>()` constructs an empty set.
- `make_set(u)` creates a new partition consisting of the single element `u`, which must not have
  been previously added to the data structure.
- `size()` returns the number of elements that have been added.
- `sets()` returns the current number of disjoint sets.
- `is_united(u, v)` returns whether elements `u` and `v` belong to the same partition.
- `unite(u, v)` replaces the partitions containing `u` and `v` with a single new partition
  consisting of the union of elements in the original partitions, returning `true` if a merge
  occurred or `false` if `u` and `v` already belonged to the same partition.
- `get_all_sets()` returns all current partitions as a vector of vectors.

A precondition to the last three operations is that `make_set()` must have been previously called on
their arguments.

Time Complexity:
- O(1) per call to the constructor.
- O(1) per call to `size()` and `sets()`.
- O(1) on average per call to `make_set()`, where $n$ is the number of elements that have been added
  via `make_set()` so far.
- O(alpha(n)) on average per call to `is_united()` and `unite()`, where $n$ is the number of
  elements that have been added via `make_set()` so far, and $\alpha(n)$ is the extremely slow
  growing inverse of the Ackermann function (effectively a very small constant for all practical
  values of $n$).
- O(n) per call to `get_all_sets()`.

Space Complexity:
- O(n) for storage of the disjoint set forest elements.
- O(n) auxiliary heap space for `get_all_sets()`.
- O(1) auxiliary for all other operations.

*/

#include <unordered_map>
#include <vector>

template<typename T>
class SparseDSU {
  int num_elements, num_sets;
  std::unordered_map<T, int> id;
  std::vector<int> root, rank;

  int find_root(int u) {
    if (root[u] != u) {
      root[u] = find_root(root[u]);
    }
    return root[u];
  }

 public:
  SparseDSU() : num_elements(0), num_sets(0) {}

  void make_set(const T &u) {
    if (id.find(u) != id.end()) {
      return;
    }
    id[u] = num_elements;
    root.push_back(num_elements++);
    rank.push_back(0);
    num_sets++;
  }

  int size() const { return num_elements; }
  int sets() const { return num_sets; }
  bool is_united(const T &u, const T &v) { return find_root(id[u]) == find_root(id[v]); }

  bool unite(const T &u, const T &v) {
    int ru = find_root(id[u]), rv = find_root(id[v]);
    if (ru == rv) {
      return false;
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
    return true;
  }

  std::vector<std::vector<T>> get_all_sets() {
    std::unordered_map<int, std::vector<T>> tmp;
    for (auto &[key, val] : id) {
      tmp[find_root(val)].push_back(key);
    }
    std::vector<std::vector<T>> res;
    for (auto &[key, val] : tmp) {
      res.push_back(val);
    }
    return res;
  }
};

/*** Example Usage and Output:

[a, b, f], [c], [d, e, g]

***/

#include <algorithm>
#include <cassert>
#include <iostream>
using namespace std;

int main() {
  SparseDSU<char> dsu;
  for (char c = 'a'; c <= 'g'; c++) {
    dsu.make_set(c);
  }
  assert(dsu.unite('a', 'b'));
  assert(dsu.unite('b', 'f'));
  assert(dsu.unite('d', 'e'));
  assert(dsu.unite('d', 'g'));
  assert(!dsu.unite('a', 'f'));  // Already united.
  assert(dsu.size() == 7);
  assert(dsu.sets() == 3);
  auto s = dsu.get_all_sets();
  for (auto &set : s) {
    sort(set.begin(), set.end());
  }
  sort(s.begin(), s.end());
  bool first_set = true;
  for (const auto &set : s) {
    cout << (first_set ? "[" : ", [");
    first_set = false;
    bool first_value = true;
    for (char value : set) {
      cout << (first_value ? "" : ", ") << value;
      first_value = false;
    }
    cout << "]";
  }
  cout << endl;
  return 0;
}
