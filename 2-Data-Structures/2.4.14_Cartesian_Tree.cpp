/*

Build the min-Cartesian tree of an array in linear time. A Cartesian tree is a binary tree whose
inorder traversal is the original array order and whose heap property is determined by array values.
The comparator `comp` defines the value ordering and defaults to `std::less<>`, producing a
min-Cartesian tree in which each parent precedes or is equivalent to its children. Each subtree
therefore represents a contiguous array interval whose first value in comparator order is at the
subtree root. Equivalent values are broken by position, so the earlier one becomes an ancestor of
the later one.

With the default comparator, this gives a direct connection to range minimum queries: the minimum
index in $[`lo`, `hi`]$ is the lowest common ancestor of indices `lo` and `hi`. The included search
finds that node without extra LCA preprocessing. Starting at the root, it moves right while the
current index is left of the range and left while it is right of the range; the first index inside
the range is its minimum. This takes O(h) time for tree height $h$, which can be O(n) for a skewed
tree. For many worst-case-fast queries, preprocess the tree for LCA or use the sparse table in 2.4.1
instead. The explicit tree is also useful for interval divide-and-conquer and tree DP; the
largest-rectangle application is implemented directly with a monotone stack in 1.2.4.

- `CartesianTree(a, comp = std::less<>())` constructs the tree for array `a`.
- `root` is the root index, or $-1$ if the array is empty. The arrays `parent`, `left`, and `right`
  store the neighboring node indices, using $-1$ when a neighbor is absent.
- `range_min_index(lo, hi)` returns the index of the first value in comparator order within
  $[`lo`, `hi`]$.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the array size.
- O(h) per call to `range_min_index()`, where $h$ is the tree height and may be O(n).

Space Complexity:
- O(n) for tree arrays and the monotone stack.

*/

#include <cassert>
#include <functional>
#include <vector>

struct CartesianTree {
  int root;
  std::vector<int> parent, left, right;

  template<typename T, typename Compare = std::less<>>
  explicit CartesianTree(const std::vector<T> &a, Compare comp = Compare{})
      : root(-1), parent(a.size(), -1), left(a.size(), -1), right(a.size(), -1) {
    std::vector<int> st;
    for (int i = 0, n = static_cast<int>(a.size()); i < n; i++) {
      int last = -1;
      while (!st.empty() && comp(a[i], a[st.back()])) {
        last = st.back();
        st.pop_back();
      }
      if (!st.empty()) {
        right[st.back()] = i;
        parent[i] = st.back();
      }
      if (last != -1) {
        left[i] = last;
        parent[last] = i;
      }
      st.push_back(i);
    }
    root = st.empty() ? -1 : st[0];
  }

  int size() const { return static_cast<int>(parent.size()); }

  int range_min_index(int lo, int hi) const {
    assert(0 <= lo && lo <= hi && hi < static_cast<int>(parent.size()));
    int i = root;
    while (i < lo || hi < i) {
      i = i < lo ? right[i] : left[i];
    }
    return i;
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  vector<int> a{3, 2, 6, 1, 9};
  CartesianTree t(a);
  assert(t.size() == 5);
  assert(t.root == 3);  // The global minimum is a[3] = 1.
  assert(t.parent[1] == 3);
  assert(t.left[3] == 1);
  assert(t.right[3] == 4);
  assert(t.range_min_index(0, 2) == 1);  // min({3, 2, 6}) is a[1].
  assert(t.range_min_index(2, 4) == 3);  // min({6, 1, 9}) is a[3].

  vector<int> equal{4, 2, 2, 3};
  CartesianTree equal_tree(equal);
  assert(equal_tree.range_min_index(1, 3) == 1);  // Earlier equal minima win ties.

  CartesianTree max_tree(a, greater<int>());
  assert(max_tree.root == 4);
  assert(max_tree.range_min_index(0, 2) == 2);  // First under greater<int>: the numeric maximum.
  return 0;
}
