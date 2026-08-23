/*

Maintain a set of elements partitioned into non-overlapping subsets using a collection of trees.
Each partition is identified by a unique representative called its root. Each subset is stored as a
tree whose nodes point toward its root: finding a representative follows parent pointers,
redirecting visited nodes straight to the root along the way (path compression), while unions attach
the shallower tree beneath the root of the deeper one (union-by-rank). Union-by-rank is
interchangeable with union-by-size as used by the plain Disjoint Set Union, yielding the same
complexity bounds; rank stores only the tree-depth estimate, so it cannot answer set-size queries.

This version uses an `std::unordered_map` for storage and coordinate compression (thus, element
types must meet the requirements of key types for `std::unordered_map`). The order of sets returned
by `get_all_sets()` is unspecified.

- `SparseDSU<T>()` constructs an empty set.
- `make_set(u)` adds `u` as a singleton partition, or does nothing if `u` is already present.
- `size()` returns the number of elements that have been added.
- `sets()` returns the current number of disjoint sets.
- `is_united(u, v)` returns whether elements `u` and `v`, which must have been added with
  `make_set()`, belong to the same partition.
- `unite(u, v)` replaces the partitions containing `u` and `v` with a single new partition
  consisting of the union of elements in the original partitions, returning `true` if a merge
  occurred or `false` if `u` and `v` already belonged to the same partition. Both elements must have
  been added with `make_set()`.
- `get_all_sets()` returns all current partitions as a vector of vectors.

Time Complexity:
- O(1) per call to the constructor.
- O(1) per call to `size()` and `sets()`.
- O(1) on average per call to `make_set()`.
- O(alpha(n)) on average per call to `is_united()` and `unite()`, where $n$ is the number of
  elements that have been added via `make_set()` so far, and $\alpha(n)$ is the extremely slow
  growing inverse of the Ackermann function (effectively a very small constant for all practical
  values of $n$).
- O(n) per call to `get_all_sets()`.

Space Complexity:
- O(n) for storage of the disjoint set forest elements.
- O(n) auxiliary for `get_all_sets()`.
- O(log n) auxiliary stack space for `is_united()` and `unite()`.
- O(1) auxiliary for all other operations.

*/

#include <cassert>
#include <unordered_map>
#include <utility>
#include <vector>

template<typename T>
class SparseDSU {
  int num_elements, num_sets;
  std::unordered_map<T, int> id;
  std::vector<int> root, rank;

  int find(int u) { return root[u] == u ? u : root[u] = find(root[u]); }

  int get_id(const T &u) const {
    auto it = id.find(u);
    assert(it != id.end());
    return it->second;
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
  bool is_united(const T &u, const T &v) { return find(get_id(u)) == find(get_id(v)); }

  bool unite(const T &u, const T &v) {
    int ru = find(get_id(u)), rv = find(get_id(v));
    if (ru == rv) {
      return false;
    }
    num_sets--;
    if (rank[ru] < rank[rv]) {
      std::swap(ru, rv);
    }
    root[rv] = ru;
    if (rank[ru] == rank[rv]) {
      rank[ru]++;
    }
    return true;
  }

  std::vector<std::vector<T>> get_all_sets() {
    std::unordered_map<int, std::vector<T>> tmp;
    for (auto &[key, val] : id) {
      tmp[find(val)].push_back(key);
    }
    std::vector<std::vector<T>> res;
    res.reserve(tmp.size());
    for (auto &[key, val] : tmp) {
      res.push_back(std::move(val));
    }
    return res;
  }
};

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
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
  for (auto &group : s) {
    sort(group.begin(), group.end());
  }
  sort(s.begin(), s.end());
  assert((s == vector<vector<char>>{{'a', 'b', 'f'}, {'c'}, {'d', 'e', 'g'}}));
  return 0;
}
