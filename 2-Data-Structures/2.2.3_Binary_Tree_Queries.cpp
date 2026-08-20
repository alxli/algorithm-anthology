/*

Nearly every question about a binary tree is answered by one recursion over it, and the questions
differ only in what that recursion carries. Three shapes cover almost all of them. The first passes
information upward, each node combining what its children return, as `height()` and `size()` do. The
second passes information downward, each node extending what its parent handed it, as a running path
sum does. The third does both at once and is the shape most often gotten wrong, because the value a
node returns is not the answer recorded there: in `diameter()` a node returns the longest downward
path through it but records the sum of its two longest, which is a path no parent can extend.

For a tree held as an adjacency list, or one of arbitrary degree, these computations become the tree
dynamic programming of sections 4.1.4 through 4.1.7.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <utility>
#include <vector>

template<typename T>
struct TreeNode {
  T value;
  TreeNode *left, *right;

  explicit TreeNode(const T &value) : value(value), left(nullptr), right(nullptr) {}
};

/*

Balance is a condition on every node, not only the root. Checking it by calling `height()` at each
node costs O(n^2) on a skewed tree, so the recursion below returns a height instead and reserves
$-1$ for "already unbalanced", answering in one pass.

- `height(root)` returns the number of nodes on the longest root-to-leaf path, so an empty tree has
  height $0$ and a leaf has height $1$.
- `size(root)` returns the number of nodes.
- `is_balanced(root)` returns whether every node's two subtrees differ in height by at most $1$.

Time Complexity:
- O(n) per call, where $n$ is the number of nodes.

Space Complexity:
- O(h) auxiliary stack space, where $h$ is the height of the tree, which is O(n) in the worst case.

*/

template<typename T>
int height(TreeNode<T> *root) {
  return root == nullptr ? 0 : 1 + std::max(height(root->left), height(root->right));
}

template<typename T>
int size(TreeNode<T> *root) {
  return root == nullptr ? 0 : 1 + size(root->left) + size(root->right);
}

template<typename T>
bool is_balanced(TreeNode<T> *root) {
  // Returns the height, or -1 once any subtree below has been found unbalanced.
  auto rec = [&](auto &&rec, TreeNode<T> *node) -> int {
    if (node == nullptr) {
      return 0;
    }
    int left = rec(rec, node->left), right = rec(rec, node->right);
    if (left < 0 || right < 0 || std::abs(left - right) > 1) {
      return -1;
    }
    return 1 + std::max(left, right);
  };
  return rec(rec, root) >= 0;
}

/*

Validating a search tree is where carrying information downward is not optional: comparing a node
only against its own children accepts trees that are not search trees at all, since a value deep in
a left subtree can exceed the root while every local comparison passes. The recursion carries the
open interval a subtree may occupy, using a null pointer for an absent bound so that no sentinel
value of `T` is needed. Walking the tree in order and checking that values increase, using the
traversal of section 2.2.1, is the alternative.

- `has_path_sum(root, target)` returns whether some root-to-leaf path has values summing to
  `target`. An empty tree has no such path, whatever the target.
- `root_to_leaf_paths(root)` returns every root-to-leaf path, each as the values along it.
- `depth(root, target)` returns the number of edges from `root` down to the node `target`, or $-1$
  if that node is not in the tree.
- `is_bst(root)` returns whether the tree satisfies the binary search tree property, using strict
  ordering so that equal values are rejected.

Time Complexity:
- O(n) per call, where $n$ is the number of nodes.

Space Complexity:
- O(h) auxiliary stack space, where $h$ is the height of the tree, which is O(n) in the worst case.
- O(n*h) for the paths returned by `root_to_leaf_paths()`.

*/

template<typename T>
bool has_path_sum(TreeNode<T> *root, const T &target) {
  if (root == nullptr) {
    return false;
  }
  if (root->left == nullptr && root->right == nullptr) {
    return root->value == target;
  }
  T rest = target - root->value;
  return has_path_sum(root->left, rest) || has_path_sum(root->right, rest);
}

template<typename T>
std::vector<std::vector<T>> root_to_leaf_paths(TreeNode<T> *root) {
  std::vector<std::vector<T>> res;
  std::vector<T> path;
  auto rec = [&](auto &&rec, TreeNode<T> *node) {
    if (node == nullptr) {
      return;
    }
    path.push_back(node->value);
    if (node->left == nullptr && node->right == nullptr) {
      res.push_back(path);
    } else {
      rec(rec, node->left);
      rec(rec, node->right);
    }
    path.pop_back();
  };
  rec(rec, root);
  return res;
}

template<typename T>
int depth(TreeNode<T> *root, TreeNode<T> *target) {
  if (root == nullptr || root == target) {
    return root == nullptr ? -1 : 0;
  }
  int left = depth(root->left, target);
  if (left >= 0) {
    return left + 1;
  }
  int right = depth(root->right, target);
  return right >= 0 ? right + 1 : -1;
}

template<typename T>
bool is_bst(TreeNode<T> *root) {
  // A null bound means unconstrained, so no sentinel value of T is required.
  auto rec = [&](auto &&rec, TreeNode<T> *node, const T *lo, const T *hi) -> bool {
    if (node == nullptr) {
      return true;
    }
    if ((lo != nullptr && !(*lo < node->value)) || (hi != nullptr && !(node->value < *hi))) {
      return false;
    }
    return rec(rec, node->left, lo, &node->value) && rec(rec, node->right, &node->value, hi);
  };
  return rec(rec, root, nullptr, nullptr);
}

/*

Counting downward paths by trying every start node is O(n^2). Instead, keep the running sum from the
root and note that a path ending here with sum `target` belongs to an ancestor whose own running sum
was smaller by exactly `target`: the prefix-sum trick of section 1.2.1, lifted from an array to a
root-to-node path. A map counts the ancestors' running sums, each node undoing its own entry on the
way back out so that only genuine ancestors match. It is ordered here for genericity, costing a
logarithmic factor that an `unordered_map` removes for hashable `T`. Storing another key instead
counts paths whose sum is divisible by `k`, or whose values exclusive-or to a target.

- `diameter(root)` returns the number of edges on the longest path between any two nodes, which is
  $0$ for a single node and for an empty tree.
- `max_path_sum(root)` returns the largest sum along any downward-then-upward path between two
  nodes, for a nonempty tree whose values may be negative. A child subtree contributes only when its
  best downward sum is positive, so an all-negative tree returns its largest single value.
- `count_paths_with_sum(root, target)` returns the number of downward paths, not necessarily
  starting at the root, whose values sum to `target`.

Time Complexity:
- O(n) per call to `diameter()` and `max_path_sum()`, where $n$ is the number of nodes.
- O(n log n) per call to `count_paths_with_sum()` from the ordered map, or O(n) expected with an
  unordered one.

Space Complexity:
- O(h) auxiliary stack space, where $h$ is the height of the tree, which is O(n) in the worst case.
- O(h) auxiliary for the running sums of `count_paths_with_sum()`, one per ancestor.

*/

template<typename T>
int diameter(TreeNode<T> *root) {
  int best = 0;
  // Returns the edge count of the longest downward path, while recording the longest path through.
  auto rec = [&](auto &&rec, TreeNode<T> *node) -> int {
    if (node == nullptr) {
      return -1;
    }
    int left = rec(rec, node->left) + 1, right = rec(rec, node->right) + 1;
    best = std::max(best, left + right);
    return std::max(left, right);
  };
  rec(rec, root);
  return best;
}

template<typename T>
T max_path_sum(TreeNode<T> *root) {
  T best = root->value;
  auto rec = [&](auto &&rec, TreeNode<T> *node) -> T {
    if (node == nullptr) {
      return T{};
    }
    // A subtree joins the path only when it adds something, so a negative branch is dropped.
    T left = std::max(T{}, rec(rec, node->left)), right = std::max(T{}, rec(rec, node->right));
    best = std::max(best, node->value + left + right);
    return node->value + std::max(left, right);
  };
  rec(rec, root);
  return best;
}

template<typename T>
int64_t count_paths_with_sum(TreeNode<T> *root, const T &target) {
  std::map<T, int> seen{{T{}, 1}};  // The empty prefix, so paths starting at the root are counted.
  int64_t res = 0;
  auto rec = [&](auto &&rec, TreeNode<T> *node, T prefix) {
    if (node == nullptr) {
      return;
    }
    prefix = prefix + node->value;
    auto it = seen.find(prefix - target);
    if (it != seen.end()) {
      res += it->second;
    }
    seen[prefix]++;
    rec(rec, node->left, prefix);
    rec(rec, node->right, prefix);
    if (--seen[prefix] == 0) {  // Undone on the way out, leaving only true ancestors behind.
      seen.erase(prefix);
    }
  };
  rec(rec, root, T{});
  return res;
}

/*

Comparing against the two targets while unwinding is all `lca()` needs, with no parent pointers and
no preprocessing: a node whose subtrees each report a find is the meeting point, and a node
reporting one find passes it upward. That is O(n) per query rather than the O(1) after preprocessing
of sections 2.8.4 and 2.8.5, so prefer it for a handful of queries. A tree is symmetric exactly when
`same_tree()` holds between one half and the mirror of the other, so no separate routine is needed.

- `invert(root)` mirrors the tree in place by swapping every node's children.
- `same_tree(a, b)` returns whether two trees have the same shape and the same values.
- `lca(root, a, b)` returns the lowest node having both `a` and `b` in its subtree, counting a node
  as its own ancestor. If only one target belongs to the tree, that target is returned; if neither
  does, `nullptr` is returned.
- `distance(root, a, b)` returns the number of edges on the path between the two nodes, which is the
  depth of each below their lowest common ancestor. Both nodes must belong to the tree.

Time Complexity:
- O(n) per call, where $n$ is the number of nodes.

Space Complexity:
- O(h) auxiliary stack space, where $h$ is the height of the tree, which is O(n) in the worst case.

*/

template<typename T>
void invert(TreeNode<T> *root) {
  if (root != nullptr) {
    std::swap(root->left, root->right);
    invert(root->left);
    invert(root->right);
  }
}

template<typename T>
bool same_tree(TreeNode<T> *a, TreeNode<T> *b) {
  if (a == nullptr || b == nullptr) {
    return a == b;
  }
  return a->value == b->value && same_tree(a->left, b->left) && same_tree(a->right, b->right);
}

template<typename T>
TreeNode<T> *lca(TreeNode<T> *root, TreeNode<T> *a, TreeNode<T> *b) {
  if (root == nullptr || root == a || root == b) {
    return root;
  }
  TreeNode<T> *left = lca(root->left, a, b), *right = lca(root->right, a, b);
  if (left != nullptr && right != nullptr) {
    return root;  // Targets found on opposite sides, so this node is where they meet.
  }
  return left != nullptr ? left : right;
}

template<typename T>
int distance(TreeNode<T> *root, TreeNode<T> *a, TreeNode<T> *b) {
  TreeNode<T> *meet = lca(root, a, b);
  int da = depth(meet, a), db = depth(meet, b);
  assert(da >= 0 && db >= 0);
  return da + db;
}

/*** Example Usage ***/

using namespace std;

int main() {
  //     4
  //    / \.
  //   2   6
  //  / \   \.
  // 1   3   7
  TreeNode<int> n1(1), n2(2), n3(3), n4(4), n6(6), n7(7);
  n4.left = &n2;
  n4.right = &n6;
  n2.left = &n1;
  n2.right = &n3;
  n6.right = &n7;

  assert(height(&n4) == 3 && size(&n4) == 6);
  assert(height<int>(nullptr) == 0 && size<int>(nullptr) == 0);
  assert(is_balanced(&n4));

  assert(has_path_sum(&n4, 7));   // 4 + 2 + 1
  assert(has_path_sum(&n4, 17));  // 4 + 6 + 7
  assert(!has_path_sum(&n4, 6));  // 4 + 2 stops at an internal node.
  assert((root_to_leaf_paths(&n4) == vector<vector<int>>{{4, 2, 1}, {4, 2, 3}, {4, 6, 7}}));

  assert(diameter(&n4) == 4);       // The path 1, 2, 4, 6, 7 has four edges.
  assert(max_path_sum(&n4) == 22);  // The path 3, 2, 4, 6, 7.

  assert(lca(&n4, &n1, &n3) == &n2);
  assert(lca(&n4, &n1, &n7) == &n4);
  assert(lca(&n4, &n6, &n7) == &n6);  // A node is its own ancestor.
  assert(depth(&n4, &n7) == 2 && depth(&n4, &n4) == 0);
  assert(distance(&n4, &n1, &n7) == 4 && distance(&n4, &n1, &n3) == 2);

  // The fixture happens to be a search tree, since its inorder is 1, 2, 3, 4, 6, 7.
  assert(is_bst(&n4));

  // Every local comparison passes here, yet 4 sits in the right subtree of 5.
  //     5
  //    / \.
  //   1   6
  //      / \.
  //     4   7
  TreeNode<int> b5(5), b1(1), b6(6), b4(4), b7(7);
  b5.left = &b1;
  b5.right = &b6;
  b6.left = &b4;
  b6.right = &b7;
  assert(!is_bst(&b5));

  // Downward paths summing to 7: the whole left spine 4, 2, 1 and the single node 7.
  assert(count_paths_with_sum(&n4, 7) == 2);
  assert(count_paths_with_sum(&n4, 100) == 0);

  // A left-leaning chain is as unbalanced as a tree gets, and every value is on one path.
  TreeNode<int> a(1), b(2), c(3);
  c.left = &b;
  b.left = &a;
  assert(!is_balanced(&c) && height(&c) == 3 && diameter(&c) == 2);

  // Negative values: only branches that add something join the best path.
  TreeNode<int> neg(-5), worse(-9);
  neg.left = &worse;
  assert(max_path_sum(&neg) == -5);

  TreeNode<int> m1(1), m2(2), m3(3);
  m2.left = &m1;
  m2.right = &m3;
  assert(same_tree(&m2, &n2));  // A separate copy of the subtree rooted at 2.
  invert(&m2);
  assert(m2.left == &m3 && m2.right == &m1 && !same_tree(&m2, &n2));
  return 0;
}
