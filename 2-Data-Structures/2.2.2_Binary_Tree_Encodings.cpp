/*

An encoding of a binary tree is a sequence the tree can be rebuilt from, and a single traversal is
not one, since many shapes share the same preorder. A pair of traversals usually is: an inorder
together with either a preorder or a postorder pins the shape down, because the inorder splits the
values into a left group and a right group once the root is known, and the other traversal is what
identifies that root. Preorder puts the root first, postorder puts it last, and each recursion then
knows exactly how many values belong to each side.

Written naively, locating the root inside the inorder range costs a linear scan and the construction
degenerates to O(n^2) on a skewed tree. Hashing each value to its inorder position first makes every
lookup O(1) expected, so the whole build is linear expected time. This is why the values must be
distinct: with duplicates, the position of a root in the inorder sequence is ambiguous and no
traversal pair determines a unique tree.

- `build_from_preorder_inorder(pre, in)` returns the root of the tree with the given preorder and
  inorder traversals, or `nullptr` when they are empty. Both sequences must be the same length and
  contain the same distinct values.
- `build_from_postorder_inorder(post, in)` does the same from a postorder and an inorder traversal.

The construction routines allocate each node with `new` and return raw pointers. Thus the caller
owns the resulting tree and should eventually delete its nodes in long-running programs. Contest
programs commonly omit this cleanup when the tree is needed until program termination, since the
operating system reclaims the process's memory on exit.

A third encoding needs only one traversal, because recording the null children removes the ambiguity
that made a pair necessary. A preorder in which every absent child becomes a sentinel token
reconstructs the tree by reading the tokens back in order, and unlike a traversal pair it tolerates
duplicate values, the shape now being explicit rather than inferred.

- `serialize(root)` returns the tree as a whitespace-separated preorder string, writing `#` for each
  null child. The value type must support stream output.
- `deserialize(s)` rebuilds the tree that `serialize()` produced. The value type must support stream
  input. Its returned nodes are allocated with `new` and have the same ownership convention as the
  construction routines above.

Serialization is what lets a subtree be used as a key: two subtrees are identical exactly when their
serializations match, which turns finding duplicate subtrees, or memoizing over tree shapes, into
hashing strings. Prefer hashing a canonical encoding this way over comparing trees pairwise. Note
that this encoding is specific to binary trees; section 4.1.8 encodes labeled general trees instead.

Time Complexity:
- O(n) expected per call to both construction routines, where n is the number of nodes, from the
  hash lookups of inorder positions.
- O(n) per call to `serialize()` and `deserialize()`.

Space Complexity:
- O(n) for the allocated tree nodes.
- O(n) auxiliary for the position map, and O(h) auxiliary stack space, where h is the height of the
  tree.
- O(n) for the string returned by `serialize()`.

*/

#include <cassert>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

template<typename T>
struct TreeNode {
  T value;
  TreeNode *left, *right;

  explicit TreeNode(const T &value) : value(value), left(nullptr), right(nullptr) {}
};

template<typename T>
TreeNode<T> *build_from_preorder_inorder(const std::vector<T> &pre, const std::vector<T> &in) {
  assert(pre.size() == in.size());
  int n = static_cast<int>(pre.size());
  std::unordered_map<T, int> pos;
  for (int i = 0; i < n; i++) {
    pos[in[i]] = i;
  }
  assert(static_cast<int>(pos.size()) == n);
  int next = 0;
  auto rec = [&](auto &&rec, int lo, int hi) -> TreeNode<T> * {
    if (lo > hi) {
      return nullptr;
    }
    const T &value = pre[next++];
    int mid = pos[value];
    TreeNode<T> *node = new TreeNode<T>(value);
    node->left = rec(rec, lo, mid - 1);
    node->right = rec(rec, mid + 1, hi);
    return node;
  };
  return rec(rec, 0, n - 1);
}

template<typename T>
TreeNode<T> *build_from_postorder_inorder(const std::vector<T> &post, const std::vector<T> &in) {
  assert(post.size() == in.size());
  int n = static_cast<int>(post.size());
  std::unordered_map<T, int> pos;
  for (int i = 0; i < n; i++) {
    pos[in[i]] = i;
  }
  assert(static_cast<int>(pos.size()) == n);
  int next = n - 1;
  auto rec = [&](auto &&rec, int lo, int hi) -> TreeNode<T> * {
    if (lo > hi) {
      return nullptr;
    }
    const T &value = post[next--];
    TreeNode<T> *node = new TreeNode<T>(value);
    int mid = pos[value];
    node->right = rec(rec, mid + 1, hi);  // Postorder ends with the root, so the right side first.
    node->left = rec(rec, lo, mid - 1);
    return node;
  };
  return rec(rec, 0, n - 1);
}

template<typename T>
std::string serialize(TreeNode<T> *root) {
  std::ostringstream out;
  auto rec = [&](auto &&rec, TreeNode<T> *node) -> void {
    if (node == nullptr) {
      out << "# ";
      return;
    }
    out << node->value << " ";
    rec(rec, node->left);
    rec(rec, node->right);
  };
  rec(rec, root);
  return out.str();
}

template<typename T>
TreeNode<T> *deserialize(const std::string &s) {
  std::istringstream in(s);
  auto rec = [&](auto &&rec) -> TreeNode<T> * {
    std::string token;
    if (!(in >> token) || token == "#") {
      return nullptr;
    }
    std::istringstream parse(token);
    T value;
    parse >> value;
    TreeNode<T> *node = new TreeNode<T>(value);
    node->left = rec(rec);
    node->right = rec(rec);
    return node;
  };
  return rec(rec);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

template<typename T>
vector<T> inorder(TreeNode<T> *root) {
  vector<T> res;
  auto rec = [&](auto &&rec, TreeNode<T> *node) -> void {
    if (node != nullptr) {
      rec(rec, node->left);
      res.push_back(node->value);
      rec(rec, node->right);
    }
  };
  rec(rec, root);
  return res;
}

int main() {
  //     4
  //    / \.
  //   2   6
  //  / \   \.
  // 1   3   7
  vector<int> pre{4, 2, 1, 3, 6, 7}, in{1, 2, 3, 4, 6, 7}, post{1, 3, 2, 7, 6, 4};

  TreeNode<int> *root = build_from_preorder_inorder(pre, in);
  assert(root->value == 4 && root->left->value == 2 && root->right->value == 6);
  assert(root->right->left == nullptr && root->right->right->value == 7);
  assert(inorder(root) == in);

  TreeNode<int> *same = build_from_postorder_inorder(post, in);
  assert(serialize(same) == serialize(root));

  // The serialized form round-trips, and matching strings mean identical subtrees.
  assert(inorder(deserialize<int>(serialize(root))) == in);
  assert(serialize(root->left) != serialize(root->right));

  assert(build_from_preorder_inorder(vector<int>{}, vector<int>{}) == nullptr);
  assert(serialize<int>(nullptr) == "# ");
  assert(deserialize<int>("# ") == nullptr);
  return 0;
}
