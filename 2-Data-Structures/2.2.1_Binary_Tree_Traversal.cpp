/*

The three depth-first orders of a binary tree differ only in when a node is reported relative to its
subtrees: preorder before both, inorder between them, postorder after both. Recursion expresses this
directly but costs stack space proportional to the height, which overflows on a degenerate tree of a
hundred thousand nodes, so the versions below replace the call stack with an explicit one. Postorder
is the one that resists a direct translation, since a node must be reported only after returning
from its right child; visiting root, right, then left is a mirrored preorder, and reversing that
sequence yields postorder.

- `preorder(root)`, `inorder(root)`, and `postorder(root)` return the values of the tree in each
  depth-first order, using an explicit stack.
- `level_order(root)` returns the values one depth at a time, left to right, using a queue.

Morris traversal removes the stack entirely by threading. Before descending into a left subtree, it
finds that subtree's rightmost node, which is the inorder predecessor of the current node, and
points that node's empty right pointer back at the current node. On reaching the end of the subtree
the walk follows the thread back up, and seeing it a second time signals that the left subtree is
finished, so the node is reported and the borrowed pointer restored. Each edge is traversed at most
three times, keeping the walk linear. Flattening uses the same predecessor search but keeps the
rewiring: splicing the right subtree onto that node and moving the left subtree across leaves a
right-leaning chain in preorder.

- `morris_inorder(root)` returns the values in inorder using O(1) auxiliary space, temporarily
  rewriting the tree's null pointers and restoring them before returning.
- `flatten(root)` rewrites the tree in place into a right-leaning chain whose order is the tree's
  preorder, leaving every left pointer null and destroying the original shape.

Since `morris_inorder()` mutates the tree as it runs, it is unsafe on a tree shared between threads
and leaves threads behind if an exception escapes. Computing over a tree rather than listing it is
dynamic programming on a tree; see sections 4.1.4 through 4.1.7 for general rooted trees.

Time Complexity:
- O(n) per call to every traversal and to `flatten()`, where $n$ is the number of nodes.

Space Complexity:
- O(n) for each returned vector.
- O(h) auxiliary for the stack-based and queue-based traversals, where $h$ is the height of the
  tree, which is O(n) in the worst case.
- O(1) auxiliary for `morris_inorder()` and `flatten()`.

*/

#include <algorithm>
#include <queue>
#include <vector>

template<typename T>
struct TreeNode {
  T value;
  TreeNode *left, *right;

  explicit TreeNode(const T &value) : value(value), left(nullptr), right(nullptr) {}
};

template<typename T>
std::vector<T> preorder(TreeNode<T> *root) {
  std::vector<T> res;
  std::vector<TreeNode<T> *> st;
  if (root != nullptr) {
    st.push_back(root);
  }
  while (!st.empty()) {
    TreeNode<T> *node = st.back();
    st.pop_back();
    res.push_back(node->value);
    if (node->right != nullptr) {  // Pushed first so that the left child is popped first.
      st.push_back(node->right);
    }
    if (node->left != nullptr) {
      st.push_back(node->left);
    }
  }
  return res;
}

template<typename T>
std::vector<T> inorder(TreeNode<T> *root) {
  std::vector<T> res;
  std::vector<TreeNode<T> *> st;
  for (TreeNode<T> *node = root; node != nullptr || !st.empty();) {
    for (; node != nullptr; node = node->left) {
      st.push_back(node);
    }
    node = st.back();
    st.pop_back();
    res.push_back(node->value);
    node = node->right;
  }
  return res;
}

template<typename T>
std::vector<T> postorder(TreeNode<T> *root) {
  std::vector<T> res;
  std::vector<TreeNode<T> *> st;
  if (root != nullptr) {
    st.push_back(root);
  }
  while (!st.empty()) {  // Root, right, left, which is postorder reversed.
    TreeNode<T> *node = st.back();
    st.pop_back();
    res.push_back(node->value);
    if (node->left != nullptr) {
      st.push_back(node->left);
    }
    if (node->right != nullptr) {
      st.push_back(node->right);
    }
  }
  std::reverse(res.begin(), res.end());
  return res;
}

template<typename T>
std::vector<T> level_order(TreeNode<T> *root) {
  std::vector<T> res;
  std::queue<TreeNode<T> *> q;
  if (root != nullptr) {
    q.push(root);
  }
  while (!q.empty()) {
    TreeNode<T> *node = q.front();
    q.pop();
    res.push_back(node->value);
    if (node->left != nullptr) {
      q.push(node->left);
    }
    if (node->right != nullptr) {
      q.push(node->right);
    }
  }
  return res;
}

template<typename T>
std::vector<T> morris_inorder(TreeNode<T> *root) {
  std::vector<T> res;
  for (TreeNode<T> *node = root; node != nullptr;) {
    if (node->left == nullptr) {
      res.push_back(node->value);
      node = node->right;
      continue;
    }
    TreeNode<T> *pred = node->left;
    while (pred->right != nullptr && pred->right != node) {
      pred = pred->right;
    }
    if (pred->right == nullptr) {  // First visit: thread the predecessor back to this node.
      pred->right = node;
      node = node->left;
    } else {  // Second visit: the left subtree is done, so restore and report.
      pred->right = nullptr;
      res.push_back(node->value);
      node = node->right;
    }
  }
  return res;
}

template<typename T>
void flatten(TreeNode<T> *root) {
  for (TreeNode<T> *node = root; node != nullptr; node = node->right) {
    if (node->left != nullptr) {
      TreeNode<T> *pred = node->left;
      while (pred->right != nullptr) {
        pred = pred->right;
      }
      pred->right = node->right;  // Splice the right subtree below the left subtree's last node.
      node->right = node->left;
      node->left = nullptr;
    }
  }
}

/*** Example Usage ***/

#include <cassert>
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

  assert((preorder(&n4) == vector<int>{4, 2, 1, 3, 6, 7}));
  assert((inorder(&n4) == vector<int>{1, 2, 3, 4, 6, 7}));
  assert((postorder(&n4) == vector<int>{1, 3, 2, 7, 6, 4}));
  assert((level_order(&n4) == vector<int>{4, 2, 6, 1, 3, 7}));
  assert((morris_inorder(&n4) == vector<int>{1, 2, 3, 4, 6, 7}));

  // The threads are undone, so the tree is unchanged and a second walk agrees.
  assert(n3.right == nullptr && n1.right == nullptr);
  assert(morris_inorder(&n4) == inorder(&n4));
  assert(inorder<int>(nullptr).empty() && morris_inorder<int>(nullptr).empty());

  // A left-leaning chain, the shape that makes a recursive walk overflow the stack.
  TreeNode<int> a(1), b(2), c(3);
  c.left = &b;
  b.left = &a;
  assert((morris_inorder(&c) == vector<int>{1, 2, 3}));
  assert((postorder(&c) == vector<int>{1, 2, 3}));

  // Flattening leaves a right-leaning chain in preorder, so a walk of it is that preorder.
  vector<int> before = preorder(&n4);
  flatten(&n4);
  assert(n4.left == nullptr && preorder(&n4) == before);
  for (TreeNode<int> *node = &n4; node != nullptr; node = node->right) {
    assert(node->left == nullptr);
  }
  return 0;
}
