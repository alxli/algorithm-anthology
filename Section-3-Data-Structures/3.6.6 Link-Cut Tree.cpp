/*

Maintain a forest of trees with values associated with its nodes, while
supporting both dynamic queries and dynamic updates of all values on any path
between two nodes in a given tree. In addition, support testing of whether two
nodes are connected in the forest, as well as the merging and spliting of trees
by adding or removing specific edges. Link/cut forests divide each of its trees
into vertex-disjoint paths, each represented by a splay tree.

The query operation is defined by an associative join_values() function which
satisfies join_values(x, join_values(y, z)) = join_values(join_values(x, y), z)
for all values x, y, and z in the forest. The default code below assumes a
numerical forest type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case the join_values()
function should be defined to return "a + b".

The update operation is defined by the join_value_with_delta() and join_deltas()
functions, which determines the change made to values. These must satisfy:
- join_deltas(d1, join_deltas(d2, d3)) = join_deltas(join_deltas(d1, d2), d3).
- join_value_with_delta(join_values(v, ...(m times)..., v), d, m)) should be
  equal to join_values(join_value_with_delta(v, d, 1), ...(m times)).
- if a sequence d_1, ..., d_m of deltas is used to update a value v, then
  join_value_with_delta(v, join_deltas(d_1, ..., d_m), 1) should be equivalent
  to m sequential calls to join_value_with_delta(v, d_i, 1) for i = 1..m.
The default code below defines updates that "set" a path's nodes to a new value.
Another possible update operation is "increment", in which case
join_value_with_delta(v, d, len) should be defined to return "v + d*len" and
join_deltas(d1, d2) should be defined to return "d1 + d2".

- link_cut_forest() constructs an empty forest with no trees.
- size() returns the number of nodes in the forest.
- trees() returns the number of trees in the forest.
- make_root(i, v) creates a new tree in the forest consisting of a single node
  labeled with the integer i and value initialized to v.
- is_connected(a, b) returns whether nodes a and b are connected.
- link(a, b) adds an edge between the nodes a and b, both of which must exist
  and not be connected.
- cut(a, b) removes the edge between the nodes a and b, both of which must
  exist and be connected.
- query(a, b) returns the result of join_values() applied to all values on the
  path from the node a to node b.
- update(a, b, d) modifies all the values on the path from node a to node b by
  respectively joining them with d using join_value_with_delta().

Time Complexity:
- O(1) per call to the constructor, size(), and trees().
- O(log n) amortized per call to all other operations, where n is the number of
  nodes.

Space Complexity:
- O(n) for storage of the forest, where n is the number of nodes.
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cstdlib>
#include <map>
#include <stdexcept>

template<class T>
class link_cut_forest {
  static T join_values(const T &a, const T &b) {
    return std::min(a, b);
  }

  static T join_value_with_delta(const T &v, const T &d, int len) {
    return d;
  }

  static T join_deltas(const T &d1, const T &d2) {
    return d2;  // For "set" updates, the more recent delta prevails.
  }

  struct node_t {
    T value, subtree_value, delta;
    int size;
    bool rev, pending;
    node_t *left, *right, *parent;

    node_t(const T &v)
        : value(v), subtree_value(v), size(1), rev(false), pending(false),
          left(NULL), right(NULL), parent(NULL) {}

    inline bool is_root() const {
      return parent == NULL || (parent->left != this && parent->right != this);
    }

    inline T get_subtree_value() const {
      return pending ? join_value_with_delta(subtree_value, delta, size)
                     : subtree_value;
    }

    void push() {
      if (rev) {
        rev = false;
        std::swap(left, right);
        if (left != NULL) {
          left->rev = !left->rev;
        }
        if (right != NULL) {
          right->rev = !right->rev;
        }
      }
      if (pending) {
        value = join_value_with_delta(value, delta, 1);
        subtree_value = join_value_with_delta(subtree_value, delta, size);
        if (left != NULL) {
          left->delta = left->pending ? join_deltas(left->delta, delta) : delta;
          left->pending = true;
        }
        if (right != NULL) {
          right->delta = right->pending ? join_deltas(right->delta, delta)
                                        : delta;
          right->pending = true;
        }
        pending = false;
      }
    }

    void update() {
      size = 1;
      subtree_value = value;
      if (left != NULL) {
        subtree_value = join_values(subtree_value, left->get_subtree_value());
        size += left->size;
      }
      if (right != NULL) {
        subtree_value = join_values(subtree_value, right->get_subtree_value());
        size += right->size;
      }
    }
  };

  int num_trees;
  std::map<int, node_t*> nodes;

  static void connect(node_t *child, node_t *parent, bool is_left) {
    if (child != NULL) {
      child->parent = parent;
    }
    if (is_left) {
      parent->left = child;
    } else {
      parent->right = child;
    }
  }

  static void rotate(node_t *n) {
    node_t *parent = n->parent, *grandparent = parent->parent;
    bool parent_is_root = parent->is_root(), is_left = (n == parent->left);
    connect(is_left ? n->right : n->left, parent, is_left);
    connect(parent, n, !is_left);
    if (parent_is_root) {
      if (n != NULL) {
        n->parent = grandparent;
      }
    } else {
      connect(n, grandparent, parent == grandparent->left);
    }
    parent->update();
  }

  static void splay(node_t *n) {
    while (!n->is_root()) {
      node_t *parent = n->parent, *grandparent = parent->parent;
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

  static node_t* expose(node_t *n) {
    node_t *prev = NULL;
    for (node_t *curr = n; curr != NULL; curr = curr->parent) {
      splay(curr);
      curr->left = prev;
      prev = curr;
    }
    splay(n);
    return prev;
  }

  // Helper variables.
  node_t *u, *v;

  void get_uv(int a, int b) {
    typename std::map<int, node_t*>::iterator it1, it2;
    it1 = nodes.find(a);
    it2 = nodes.find(b);
    if (it1 == nodes.end() || it2 == nodes.end()) {
      throw std::runtime_error("Queried node ID does not exist in forest.");
    }
    u = it1->second;
    v = it2->second;
  }

 public:
  link_cut_forest() : num_trees(0) {}

  ~link_cut_forest() {
    typename std::map<int, node_t*>::iterator it;
    for (it = nodes.begin(); it != nodes.end(); ++it) {
      delete it->second;
    }
  }

  int size() const {
    return nodes.size();
  }

  int trees() const {
    return num_trees;
  }

  void make_root(int i, const T &v = T()) {
    if (nodes.find(i) != nodes.end()) {
      throw std::runtime_error("Cannot make a root with an existing ID.");
    }
    node_t *n = new node_t(v);
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
    return u->parent != NULL;
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
    if (v->right != u || u->left != NULL) {
      throw std::runtime_error("Cannot cut edge that does not exist.");
    }
    v->right->parent = NULL;
    v->right = NULL;
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
    v->delta = v->pending ? join_deltas(v->delta, d) : d;
    v->pending = true;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  link_cut_forest<int> lcf;
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
