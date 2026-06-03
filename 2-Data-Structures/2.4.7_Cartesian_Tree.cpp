/*

Builds the min Cartesian tree of an array in linear time. A Cartesian tree is a binary tree whose
inorder traversal is the original array order and whose heap property is determined by array values.
For a min Cartesian tree, each parent has value less than or equal to its children.

Cartesian trees connect arrays, stacks, range minimum queries, treaps, and largest rectangle style
problems. Equal values are broken by position, so earlier equal values become ancestors of later
equal values.

- `cartesian_tree(a)` constructs the tree for array `a`.
- `root` stores the root index.
- `parent[i]`, `left[i]`, and `right[i]` store neighboring node indices, or `-1` if absent.

Time Complexity:
- O(n) construction time, where $n$ is the array size.

Space Complexity:
- O(n) for tree arrays and the monotone stack.

*/

#include <vector>

struct cartesian_tree {
  int root;
  std::vector<int> parent, left, right;

  explicit cartesian_tree(const std::vector<int> &a)
      : root(-1), parent(a.size(), -1), left(a.size(), -1), right(a.size(), -1) {
    std::vector<int> st;
    for (int i = 0; i < (int)a.size(); i++) {
      int last = -1;
      while (!st.empty() && a[i] < a[st.back()]) {
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
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

void inorder(const cartesian_tree &t, int u, vector<int> *order) {
  if (u == -1) {
    return;
  }
  inorder(t, t.left[u], order);
  order->push_back(u);
  inorder(t, t.right[u], order);
}

int main() {
  int raw[] = {3, 2, 6, 1, 9};
  vector<int> a(raw, raw + 5);
  cartesian_tree t(a);
  assert(t.root == 3);
  assert(t.parent[1] == 3);
  assert(t.parent[4] == 3);

  vector<int> order;
  inorder(t, t.root, &order);
  for (int i = 0; i < 5; i++) {
    assert(order[i] == i);
  }
  return 0;
}
