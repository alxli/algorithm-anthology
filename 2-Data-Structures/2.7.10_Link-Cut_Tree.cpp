/*

Maintain a forest of trees with values associated with its nodes, while supporting both dynamic
queries and dynamic updates of all values on any path between two nodes in a given tree. In
addition, support testing of whether two nodes are connected in the forest, as well as the merging
and splitting of trees by adding or removing specific edges. Link/cut forests divide each of its
trees into node-disjoint paths, each represented by a splay tree.

The query operation is defined by an associative `combine()` function. The default code below
assumes a numerical forest type, defining queries for the "min" of the target range. Another
possible query operation is "sum", in which case `combine(a, b)` should return `a + b`. For
direction-independent path queries, `combine()` should also be commutative; otherwise, store enough
information in each aggregate to combine paths in the required order.

The update operation is defined by `apply_delta()` and `compose_deltas()`. A delta must act on an
aggregate summary of a path of length `len`: `apply_delta(v, d, len)` returns the aggregate after
applying update `d` to every element represented by aggregate `v`. Pending deltas are combined in
chronological order by `compose_deltas(old, d)`, meaning "apply `old`, then apply `d`". These hooks
do not support arbitrary query/update pairings; the delta operation must distribute over
`combine()`, and composed deltas must have the same effect as applying their updates sequentially.
The default code below defines updates that "set" a path's nodes to a new value. For range increment
updates, `apply_delta(v, d, len)` would return `v + d` for min/max queries, or `v + d * len` for sum
queries, and `compose_deltas(old, d)` would return `old + d`.

- `LinkCut<T>()` constructs an empty forest.
- `size()` returns the number of nodes in the forest.
- `trees()` returns the number of trees in the forest.
- `add_node(u, value = T())` adds a new single-node tree to the forest, labeled with the integer `u`
  and with value initialized to `value`.
- `connected(u, v)` returns whether nodes `u` and `v` are connected.
- `link(u, v)` adds an edge between the nodes `u` and `v`, both of which must exist and not be
  connected.
- `cut(u, v)` removes the edge between the nodes `u` and `v`, both of which must exist and be
  connected.
- `query(u, v)` returns the result of `combine()` applied to all values on the path from node `u` to
  node `v`.
- `update(u, v, d)` modifies all the values on the path from node `u` to node `v` by applying the
  delta `d`.
- `reroot(u)` makes node `u` the root of its tree.
- `find_root(u)` returns the label of the root of the tree containing node `u`.
- `lca(u, v)` returns the lowest common ancestor of `u` and `v` relative to the tree's current root.

The forest is unrooted: a tree's root is whichever node was most recently established as such.
`reroot(u)` sets it explicitly, while `link`, `query`, and `update` reroot implicitly (the latter
two at their first argument). `find_root(u)` and `lca(u, v)` are therefore relative to the current
root, so call `reroot(r)` first whenever a specific root `r` is intended.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `trees()`.
- O(log n) amortized per call to all other operations, where $n$ is the number of nodes.

Space Complexity:
- O(n) for storage of the forest, where $n$ is the number of nodes.
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <utility>

template<typename T>
class LinkCut {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &v, const T &d, int len) { return d; }
  static T compose_deltas(const T &old, const T &d) { return d; }

  struct Node {
    int id;
    T value, subtree_value, delta;
    int size;
    bool rev, pending;
    Node *left, *right, *parent;

    Node(int id, const T &value)
        : id(id),
          value(value),
          subtree_value(value),
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
        // Combine in in-order (left, value, right) so non-commutative aggregates stay correct.
        subtree_value = combine(left->get_subtree_value(), subtree_value);
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

  // Convention: a node's left child leads toward the tree's root, so an exposed path is ordered by
  // depth with the root as its leftmost node (matching the standard link/cut tree presentation).
  // cut() and find_root() rely on this orientation.
  static Node *expose(Node *n) {
    Node *prev = nullptr;
    for (Node *curr = n; curr != nullptr; curr = curr->parent) {
      splay(curr);
      curr->right = prev;
      curr->update();
      prev = curr;
    }
    splay(n);
    return prev;
  }

  Node *find_node(int u) const {
    auto it = nodes.find(u);
    assert(it != nodes.end());
    return it->second;
  }

 public:
  LinkCut() : num_trees(0) {}

  ~LinkCut() {
    for (auto &[key, node] : nodes) {
      delete node;
    }
  }

  LinkCut(const LinkCut &) = delete;
  LinkCut &operator=(const LinkCut &) = delete;
  int size() const { return static_cast<int>(nodes.size()); }
  int trees() const { return num_trees; }

  void add_node(int u, const T &value = T()) {
    assert(nodes.find(u) == nodes.end());
    Node *n = new Node(u, value);
    expose(n);
    n->rev = !n->rev;
    nodes[u] = n;
    num_trees++;
  }

  bool connected(int u, int v) {
    Node *nu = find_node(u), *nv = find_node(v);
    if (u == v) {
      return true;
    }
    expose(nu);
    expose(nv);
    return nu->parent != nullptr;
  }

  void link(int u, int v) {
    assert(!connected(u, v));
    Node *nu = find_node(u), *nv = find_node(v);
    expose(nu);
    nu->rev = !nu->rev;
    nu->parent = nv;
    num_trees--;
  }

  void cut(int u, int v) {
    Node *nu = find_node(u), *nv = find_node(v);
    expose(nu);
    nu->rev = !nu->rev;
    expose(nv);
    assert(nv->left == nu && nu->right == nullptr);
    nv->left->parent = nullptr;
    nv->left = nullptr;
    num_trees++;
  }

  T query(int u, int v) {
    assert(connected(u, v));
    Node *nu = find_node(u), *nv = find_node(v);
    expose(nu);
    nu->rev = !nu->rev;
    expose(nv);
    return nv->get_subtree_value();
  }

  void update(int u, int v, const T &d) {
    assert(connected(u, v));
    Node *nu = find_node(u), *nv = find_node(v);
    expose(nu);
    nu->rev = !nu->rev;
    expose(nv);
    nv->delta = nv->pending ? compose_deltas(nv->delta, d) : d;
    nv->pending = true;
  }

  void reroot(int u) {
    Node *n = find_node(u);
    expose(n);
    n->rev = !n->rev;
  }

  int find_root(int u) {
    Node *n = find_node(u);
    expose(n);
    while (n->left != nullptr) {  // The leftmost node of the exposed path is the tree's root.
      n = n->left;
      n->push();
    }
    splay(n);
    return n->id;
  }

  int lca(int u, int v) {
    Node *nu = find_node(u), *nv = find_node(v);
    if (u == v) {
      return u;
    }
    expose(nu);
    expose(nv);
    assert(nu->parent != nullptr);
    splay(nu);
    // nu->parent is now where nu's path rejoins nv's exposed path, i.e. their LCA.
    return nu->parent != nullptr ? nu->parent->id : nu->id;
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  // v=10      v=40      v=20      v=10
  //  0---------1---------2---------3
  //                      |
  //                      +---------4
  //                               v=30
  LinkCut<int> lcf;
  lcf.add_node(0, 10);
  lcf.add_node(1, 40);
  lcf.add_node(2, 20);
  lcf.add_node(3, 10);
  lcf.add_node(4, 30);
  assert(lcf.size() == 5);
  assert(lcf.trees() == 5);
  lcf.link(0, 1);
  lcf.link(1, 2);
  lcf.link(2, 3);
  lcf.link(2, 4);
  assert(lcf.trees() == 1);
  assert(lcf.query(1, 4) == 20);

  // find_root and lca are relative to the current root, set explicitly via reroot.
  lcf.reroot(0);
  assert(lcf.find_root(3) == 0 && lcf.find_root(4) == 0);
  assert(lcf.lca(3, 4) == 2);  // Paths 3-2 and 4-2 meet at 2 under root 0.
  assert(lcf.lca(1, 4) == 1);  // 1 is an ancestor of 4 under root 0.
  lcf.reroot(4);
  assert(lcf.lca(0, 3) == 2);  // Rerooting at 4 changes the LCA of 0 and 3.

  lcf.update(1, 1, 100);
  lcf.update(2, 4, 100);

  // v=10     v=100     v=100      v=10
  //  0---------1---------2---------3
  //                      |
  //                      +---------4
  //                              v=100
  assert(lcf.query(4, 4) == 100);
  assert(lcf.query(0, 4) == 10);
  assert(lcf.query(3, 4) == 10);
  lcf.cut(1, 2);

  // v=10     v=100     v=100      v=0
  //  0---------1         2---------3
  //                      |
  //                      +---------4
  //                              v=100
  assert(lcf.trees() == 2);
  assert(!lcf.connected(1, 2));
  assert(!lcf.connected(0, 4));
  assert(lcf.connected(2, 3));
  return 0;
}
