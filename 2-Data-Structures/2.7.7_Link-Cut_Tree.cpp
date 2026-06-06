/*

Maintain a forest of trees with values associated with its nodes, while supporting both dynamic
queries and dynamic updates of all values on any path between two nodes in a given tree. In
addition, support testing of whether two nodes are connected in the forest, as well as the merging
and spliting of trees by adding or removing specific edges. Link/cut forests divide each of its
trees into vertex-disjoint paths, each represented by a splay tree.

The query operation is defined by an associative `combine()` function which satisfies
`combine(x, combine(y, z)) = combine(combine(x, y), z)` for all values `x`, `y`, and `z` in the
forest. The default code below assumes a numerical forest type, defining queries for the "min" of
the target range. Another possible query operation is "sum", in which case `combine(a, b)` should
return $a + b$. For direction-independent path queries, `combine()` should also be commutative;
otherwise, store enough information in each aggregate to combine paths in the required order.

The update operation is defined by `apply_delta()` and `compose_deltas()`. A delta must act on an
aggregate summary of a path of length `len`: `apply_delta(v, d, len)` returns the aggregate after
applying update `d` to every element represented by aggregate `v`. Pending deltas are combined in
chronological order by `compose_deltas(old, d)`, meaning "apply `old`, then apply `d`". These hooks
do not support arbitrary query/update pairings; the delta operation must distribute over
`combine()`, and composed deltas must have the same effect as applying their updates sequentially.
The default code below defines updates that "set" a path's nodes to a new value. For range increment
updates, `apply_delta(v, d, len)` would return `v + d` for min/max queries, or `v + d * len` for sum
queries, and `compose_deltas(old, d)` would return `old + d`.

- `LinkCutForest()` constructs an empty forest with no trees.
- `size()` returns the number of nodes in the forest.
- `trees()` returns the number of trees in the forest.
- `make_root(i, v)` creates a new tree in the forest consisting of a single node labeled with the
  integer `i` and value initialized to `v`.
- `is_connected(a, b)` returns whether nodes `a` and `b` are connected.
- `link(a, b)` adds an edge between the nodes `a` and `b`, both of which must exist and not be
  connected.
- `cut(a, b)` removes the edge between the nodes `a` and `b`, both of which must exist and be
  connected.
- `query(a, b)` returns the result of `combine()` applied to all values on the path from the node
  `a` to node `b`.
- `update(a, b, d)` modifies all the values on the path from node `a` to node `b` by respectively
  applying the delta `d`.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `trees()`.
- O(log n) amortized per call to all other operations, where $n$ is the number of nodes.

Space Complexity:
- O(n) for storage of the forest, where $n$ is the number of nodes.
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <unordered_map>

template<class T>
class LinkCutForest {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &v, const T &d, int len) { return d; }
  static T compose_deltas(const T &old, const T &d) { return d; }

  struct Node {
    T value, subtree_value, delta;
    int size;
    bool rev, pending;
    Node *left, *right, *parent;

    explicit Node(const T &v)
        : value(v),
          subtree_value(v),
          size(1),
          rev(false),
          pending(false),
          left(nullptr),
          right(nullptr),
          parent(nullptr) {}

    inline bool is_root() const {
      return parent == nullptr || (parent->left != this && parent->right != this);
    }

    inline T get_subtree_value() const {
      return pending ? apply_delta(subtree_value, delta, size) : subtree_value;
    }

    void push() {
      if (rev) {
        rev = false;
        std::swap(left, right);
        if (left != nullptr) {
          left->rev = !left->rev;
        }
        if (right != nullptr) {
          right->rev = !right->rev;
        }
      }
      if (pending) {
        value = apply_delta(value, delta, 1);
        subtree_value = apply_delta(subtree_value, delta, size);
        if (left != nullptr) {
          left->delta = left->pending ? compose_deltas(left->delta, delta) : delta;
          left->pending = true;
        }
        if (right != nullptr) {
          right->delta = right->pending ? compose_deltas(right->delta, delta) : delta;
          right->pending = true;
        }
        pending = false;
      }
    }

    void update() {
      size = 1;
      subtree_value = value;
      if (left != nullptr) {
        subtree_value = combine(subtree_value, left->get_subtree_value());
        size += left->size;
      }
      if (right != nullptr) {
        subtree_value = combine(subtree_value, right->get_subtree_value());
        size += right->size;
      }
    }
  };

  int num_trees;
  std::unordered_map<int, Node *> nodes;

  static void connect(Node *child, Node *parent, bool is_left) {
    if (child != nullptr) {
      child->parent = parent;
    }
    if (is_left) {
      parent->left = child;
    } else {
      parent->right = child;
    }
  }

  static void rotate(Node *n) {
    Node *parent = n->parent, *grandparent = parent->parent;
    bool parent_is_root = parent->is_root(), is_left = (n == parent->left);
    connect(is_left ? n->right : n->left, parent, is_left);
    connect(parent, n, !is_left);
    if (parent_is_root) {
      if (n != nullptr) {
        n->parent = grandparent;
      }
    } else {
      connect(n, grandparent, parent == grandparent->left);
    }
    parent->update();
  }

  static void splay(Node *n) {
    while (!n->is_root()) {
      Node *parent = n->parent, *grandparent = parent->parent;
      if (!parent->is_root()) {
        grandparent->push();
      }
      parent->push();
      n->push();
      if (!parent->is_root()) {
        if ((n == parent->left) == (parent == grandparent->left)) {
          rotate(parent);
        } else {
          rotate(n);
        }
      }
      rotate(n);
    }
    n->push();
    n->update();
  }

  static Node *expose(Node *n) {
    Node *prev = nullptr;
    for (Node *curr = n; curr != nullptr; curr = curr->parent) {
      splay(curr);
      curr->left = prev;
      prev = curr;
    }
    splay(n);
    return prev;
  }

  // Helper variables.
  Node *u, *v;

  void get_uv(int a, int b) {
    auto it1 = nodes.find(a);
    auto it2 = nodes.find(b);
    if (it1 == nodes.end() || it2 == nodes.end()) {
      throw std::runtime_error("Queried node ID does not exist in forest.");
    }
    u = it1->second;
    v = it2->second;
  }

 public:
  LinkCutForest() : num_trees(0) {}

  ~LinkCutForest() {
    for (auto &[key, node] : nodes) {
      delete node;
    }
  }
  LinkCutForest(const LinkCutForest &) = delete;
  LinkCutForest &operator=(const LinkCutForest &) = delete;

  int size() const { return nodes.size(); }
  int trees() const { return num_trees; }

  void make_root(int i, const T &v = T()) {
    if (auto it = nodes.find(i); it != nodes.end()) {
      throw std::runtime_error("Cannot make a root with an existing ID.");
    }
    Node *n = new Node(v);
    expose(n);
    n->rev = !n->rev;
    nodes[i] = n;
    num_trees++;
  }

  bool is_connected(int a, int b) {
    get_uv(a, b);
    if (a == b) {
      return true;
    }
    expose(u);
    expose(v);
    return u->parent != nullptr;
  }

  void link(int a, int b) {
    if (is_connected(a, b)) {
      throw std::runtime_error("Cannot link nodes that are already connected.");
    }
    get_uv(a, b);
    expose(u);
    u->rev = !u->rev;
    u->parent = v;
    num_trees--;
  }

  void cut(int a, int b) {
    get_uv(a, b);
    expose(u);
    u->rev = !u->rev;
    expose(v);
    if (v->right != u || u->left != nullptr) {
      throw std::runtime_error("Cannot cut edge that does not exist.");
    }
    v->right->parent = nullptr;
    v->right = nullptr;
    num_trees++;
  }

  T query(int a, int b) {
    if (!is_connected(a, b)) {
      throw std::runtime_error("Cannot query nodes that are not connected.");
    }
    get_uv(a, b);
    expose(u);
    u->rev = !u->rev;
    expose(v);
    return v->get_subtree_value();
  }

  void update(int a, int b, const T &d) {
    if (!is_connected(a, b)) {
      throw std::runtime_error("Cannot update nodes that are not connected.");
    }
    get_uv(a, b);
    expose(u);
    u->rev = !u->rev;
    expose(v);
    v->delta = v->pending ? compose_deltas(v->delta, d) : d;
    v->pending = true;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  LinkCutForest<int> lcf;
  lcf.make_root(0, 10);
  lcf.make_root(1, 40);
  lcf.make_root(2, 20);
  lcf.make_root(3, 10);
  lcf.make_root(4, 30);
  assert(lcf.size() == 5);
  assert(lcf.trees() == 5);
  lcf.link(0, 1);
  lcf.link(1, 2);
  lcf.link(2, 3);
  lcf.link(2, 4);
  assert(lcf.trees() == 1);

  // v=10      v=40      v=20      v=10
  //  0---------1---------2---------3
  //                      |
  //                      ----------4
  //                               v=30
  assert(lcf.query(1, 4) == 20);
  lcf.update(1, 1, 100);
  lcf.update(2, 4, 100);

  // v=10     v=100     v=100      v=10
  //  0---------1---------2---------3
  //                      |
  //                      ----------4
  //                              v=100
  assert(lcf.query(4, 4) == 100);
  assert(lcf.query(0, 4) == 10);
  assert(lcf.query(3, 4) == 10);
  lcf.cut(1, 2);

  // v=10     v=100     v=100      v=0
  //  0---------1         2---------3
  //                      |
  //                      ----------4
  //                              v=100
  assert(lcf.trees() == 2);
  assert(!lcf.is_connected(1, 2));
  assert(!lcf.is_connected(0, 4));
  assert(lcf.is_connected(2, 3));
  return 0;
}
