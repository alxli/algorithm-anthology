/*

3.6.2 - Link/Cut Tree for Dynamic Path Queries and Connectivity

Description: Given an unweighted forest of trees where each node
has an associated value, a link/cut tree can be used to dynamically
query and modify values on the path between pairs of nodes a tree.
This problem can be solved using heavy-light decomposition, which
also supports having values stored on edges rather than the nodes.
However in a link/cut tree, nodes in different trees may be
dynamically linked, edges between nodes in the same tree may be
dynamically split, and connectivity between two nodes (whether they
are in the same tree) may be checked.

Time Complexity: O(log N) amortized for make_root(), link(), cut(),
connected(), modify(), and query(), where N is the number of nodes
in the forest.

Space Complexity: O(N) on the number of nodes in the forest.

*/

#include <algorithm> /* std::max(), std::swap() */
#include <climits>   /* INT_MIN */
#include <map>
#include <stdexcept> /* std::runtime_error() */

template<class T> class linkcut_forest {
  //Modify the following 5 functions to implement your custom
  //operations on the tree. This implements the Add/Max operations.
  //Operations like Add/Sum, Set/Max can also be implemented.
  static inline T modify_op(const T & x, const T & y) {
    return x + y;
  }

  static inline T query_op(const T & x, const T & y) {
    return std::max(x, y);
  }

  static inline T delta_on_segment(const T & delta, int seglen) {
    if (delta == null_delta()) return null_delta();
    //Here you must write a fast equivalent of following slow code:
    //  T result = delta;
    //  for (int i = 1; i < seglen; i++) result = query_op(result, delta);
    //  return result;
    return delta;
  }

  static inline T null_delta() { return 0; }
  static inline T null_value() { return INT_MIN; }

  static inline T join_value_with_delta(const T & v, const T & delta) {
    return delta == null_delta() ? v : modify_op(v, delta);
  }

  static T join_deltas(const T & delta1, const T & delta2) {
    if (delta1 == null_delta()) return delta2;
    if (delta2 == null_delta()) return delta1;
    return modify_op(delta1, delta2);
  }

  struct node_t {
    T value, subtree_value, delta;
    int size;
    bool rev;
    node_t *L, *R, *parent;

    node_t(const T & v) {
      value = subtree_value = v;
      delta = null_delta();
      size = 1;
      rev = false;
      L = R = parent = 0;
    }

    bool is_root() { //is this the root of a splay tree?
      return parent == 0 || (parent->L != this && parent->R != this);
    }

    void push() {
      if (rev) {
        rev = false;
        std::swap(L, R);
        if (L != 0) L->rev = !L->rev;
        if (R != 0) R->rev = !R->rev;
      }
      value = join_value_with_delta(value, delta);
      subtree_value = join_value_with_delta(subtree_value,
                        delta_on_segment(delta, size));
      if (L != 0) L->delta = join_deltas(L->delta, delta);
      if (R != 0) R->delta = join_deltas(R->delta, delta);
      delta = null_delta();
    }

    void update() {
      subtree_value = query_op(query_op(get_subtree_value(L),
                                        join_value_with_delta(value, delta)),
                               get_subtree_value(R));
      size = 1 + get_size(L) + get_size(R);
    }
  };

  static inline int get_size(node_t * n) {
    return n == 0 ? 0 : n->size;
  }

  static inline int get_subtree_value(node_t * n) {
    return n == 0 ? null_value() : join_value_with_delta(n->subtree_value,
                                      delta_on_segment(n->delta, n->size));
  }

  static void connect(node_t * ch, node_t * p, char is_left) {
    if (ch != 0) ch->parent = p;
    if (is_left < 0) return;
    (is_left ? p->L : p->R) = ch;
  }

  /** rotates edge (n, n.parent)
   *         g          g
   *        /          /
   *       p          n
   *      / \  -->   / \
   *     n  p.r    n.l  p
   *    / \            / \
   *  n.l n.r        n.r p.r
   */
  static void rotate(node_t * n) {
    node_t *p = n->parent, *g = p->parent;
    bool is_rootp = p->is_root(), is_left = (n == p->L);
    connect(is_left ? n->R : n->L, p, is_left);
    connect(p, n, !is_left);
    connect(n, g, is_rootp ? -1 : (p == g->L));
    p->update();
  }

  /** brings n to the root, balancing tree
   *
   *  zig-zig case:
   *         g                                  n
   *        / \               p                / \
   *       p  g.r rot(p)    /   \     rot(n) n.l  p
   *      / \      -->    n       g    -->       / \
   *     n  p.r          / \     / \           n.r  g
   *    / \            n.l n.r p.r g.r             / \
   *  n.l n.r                                    p.r g.r
   *
   *  zig-zag case:
   *       g               g
   *      / \             / \               n
   *     p  g.r rot(n)   n  g.r rot(n)    /   \
   *    / \      -->    / \      -->    p       g
   *  p.l  n           p  n.r          / \     / \
   *      / \         / \            p.l n.l n.r g.r
   *    n.l n.r     p.l n.l
   */
  static void splay(node_t * n) {
    while (!n->is_root()) {
      node_t *p = n->parent, *g = p->parent;
      if (!p->is_root()) g->push();
      p->push();
      n->push();
      if (!p->is_root())
        rotate((n == p->L) == (p == g->L) ? p/*zig-zig*/ : n/*zig-zag*/);
      rotate(n);
    }
    n->push();
    n->update();
  }

  //makes node n the root of the virtual tree,
  //and also n becomes the leftmost node in its splay tree
  static node_t * expose(node_t * n) {
    node_t *prev = 0;
    for (node_t *i = n; i != 0; i = i->parent) {
      splay(i);
      i->L = prev;
      prev = i;
    }
    splay(n);
    return prev;
  }

  std::map<int, node_t*> nodes; //use array if ID compression not required
  node_t *u, *v; //temporary

  void get_uv(int a, int b) {
    static typename std::map<int, node_t*>::iterator it1, it2;
    it1 = nodes.find(a);
    it2 = nodes.find(b);
    if (it1 == nodes.end() || it2 == nodes.end())
      throw std::runtime_error("Error: a or b does not exist in forest.");
    u = it1->second;
    v = it2->second;
  }

 public:
  ~linkcut_forest() {
    static typename std::map<int, node_t*>::iterator it;
    for (it = nodes.begin(); it != nodes.end(); ++it)
      delete it->second;
  }

  void make_root(int id, const T & initv) {
    if (nodes.find(id) != nodes.end())
      throw std::runtime_error("Cannot make_root(): ID already exists.");
    node_t * n = new node_t(initv);
    expose(n);
    n->rev = !n->rev;
    nodes[id] = n;
  }

  bool connected(int a, int b) {
    get_uv(a, b);
    if (a == b) return true;
    expose(u);
    expose(v);
    return u->parent != 0;
  }

  void link(int a, int b) {
    if (connected(a, b))
      throw std::runtime_error("Error: a and b are already connected.");
    get_uv(a, b);
    expose(u);
    u->rev = !u->rev;
    u->parent = v;
  }

  void cut(int a, int b) {
    get_uv(a, b);
    expose(u);
    u->rev = !u->rev;
    expose(v);
    if (v->R != u || u->L != 0)
      throw std::runtime_error("Error: edge (a, b) does not exist.");
    v->R->parent = 0;
    v->R = 0;
  }

  T query(int a, int b) {
    if (!connected(a, b))
      throw std::runtime_error("Error: a and b are not connected.");
    get_uv(a, b);
    expose(u);
    u->rev = !u->rev;
    expose(v);
    return get_subtree_value(v);
  }

  void modify(int a, int b, const T & delta) {
    if (!connected(a, b))
      throw std::runtime_error("Error: a and b are not connected.");
    get_uv(a, b);
    expose(u);
    u->rev = !u->rev;
    expose(v);
    v->delta = join_deltas(v->delta, delta);
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  linkcut_forest<int> F;
/*
 v=10      v=40      v=20      v=10
  0---------1---------2---------3
                      \
                       ---------4
                               v=30
*/
  F.make_root(0, 10);
  F.make_root(1, 40);
  F.make_root(2, 20);
  F.make_root(3, 10);
  F.make_root(4, 30);
  F.link(0, 1);
  F.link(1, 2);
  F.link(2, 3);
  F.link(2, 4);
  cout << F.query(1, 4) << "\n"; //40
  F.modify(1, 1, -10);
  F.modify(3, 4, -10);
/*
 v=10      v=30      v=10      v=0
  0---------1---------2---------3
                      \
                       ---------4
                               v=20
*/
  cout << F.query(0, 4) << "\n"; //30
  cout << F.query(3, 4) << "\n"; //20
  F.cut(1, 2);
  cout << F.connected(1, 2) << "\n"; //0
  cout << F.connected(0, 4) << "\n"; //0
  cout << F.connected(2, 3) << "\n"; //1
  return 0;
}
