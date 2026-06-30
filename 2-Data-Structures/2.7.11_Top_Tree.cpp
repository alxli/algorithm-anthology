/*

Maintain a dynamic forest while supporting both path aggregates and rooted-subtree aggregates. Top
trees represent each tree as a hierarchy of clusters. A path cluster summarizes a contiguous path,
while non-path clusters are raked onto vertices so that changing the preferred path only requires a
logarithmic number of local rotations and recomputations.

This is an advanced alternative to Link-Cut Trees. Link-Cut Trees are usually simpler for dynamic
path queries; Top Trees are useful when the same dynamic forest also needs rooted-subtree queries.

The aggregate operation is defined by an identity element `identity()` and an associative
`combine(a, b)`. The default code below computes sums over vertex and edge values. For minimum
queries, use `std::numeric_limits<T>::max()` as the identity and `std::min(a, b)` as the combine
function. For non-commutative aggregates, store enough information in `T` to support reversing a
path, and update `flip_path()` accordingly.

- `TopTree(n, value)` constructs a forest on vertices $[0, `n`)$ with every vertex value initialized
  to `value`.
- `size()` returns the number of vertices in the forest.
- `edges()` returns the number of edge nodes that have been created.
- `connected(u, v)` returns whether vertices `u` and `v` are in the same tree.
- `link(u, v, value)` adds an edge with value `value` between different trees, returning its edge id
  or $-1$ if the edge would create a cycle.
- `cut(e)` removes edge id `e`. The edge id must currently be present.
- `set_vertex(u, value)` changes vertex `u`'s value to `value`.
- `set_edge(e, value)` changes edge `e`'s value to `value`. The edge id must currently be present.
- `path_query(u, v)` returns the aggregate over all vertex and edge values on the path from `u` to
  `v`. The vertices must be connected.
- `subtree_query(root, u)` reroots the represented tree at `root` and returns the aggregate over the
  rooted subtree of `u`.
- `reroot(u)` makes vertex `u` the root of its represented tree.
- `lca(u, v)` returns the lowest common ancestor of `u` and `v` relative to the current root. The
  vertices must be connected.
- `maybe_lca(u, v)` returns the lowest common ancestor of `u` and `v`, or $-1$ if they are
  disconnected.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `edges()`.
- O(log n) amortized per call to all other operations, where $n$ is the number of vertices and edges
  in the represented tree.

Space Complexity:
- O(n + m) for the vertex and edge nodes, where $m$ is the number of edges ever added.
- O(1) auxiliary for all operations.

*/

#include <array>
#include <cassert>
#include <utility>
#include <vector>

template<typename T>
class TopTree {
  static T identity() { return T(); }
  static T combine(const T &a, const T &b) { return a + b; }

  struct Node {
    Node *p;
    std::array<Node *, 3> c;
    T value, path_value, subtree_value;
    int vertex_id, edge_id;
    bool is_path, is_vert, lazy_flip_path, alive;

    Node(const T &value, int vertex_id, int edge_id)
        : p(nullptr),
          c{nullptr, nullptr, nullptr},
          value(value),
          path_value(value),
          subtree_value(value),
          vertex_id(vertex_id),
          edge_id(edge_id),
          is_path(vertex_id != -1),
          is_vert(vertex_id != -1),
          lazy_flip_path(false),
          alive(vertex_id != -1) {}

    static T path_value_of(Node *n) { return n == nullptr ? identity() : n->path_value; }
    static T subtree_value_of(Node *n) { return n == nullptr ? identity() : n->subtree_value; }

    int dir() const {
      assert(p != nullptr);
      if (this == p->c[0]) {
        return 0;
      } else if (this == p->c[1]) {
        return 1;
      } else if (this == p->c[2]) {
        return 2;
      }
      assert(false);
      return -1;
    }

    Node *&parent_child() const { return p->c[dir()]; }
    bool root_of_auxiliary_tree() const { return p == nullptr || p->is_path != is_path; }

    void flip_path() {
      assert(is_path);
      std::swap(c[0], c[1]);
      lazy_flip_path = !lazy_flip_path;
    }

    void push() {
      if (lazy_flip_path) {
        assert(is_path);
        if (!is_vert) {
          c[0]->flip_path();
          c[1]->flip_path();
        }
        lazy_flip_path = false;
      }
    }

    void push_all() {
      if (p != nullptr) {
        p->push_all();
      }
      push();
    }

    Node *pull_all() {
      Node *cur = this;
      cur->pull();
      while (cur->p != nullptr) {
        cur = cur->p;
        cur->pull();
      }
      return cur;
    }

    void pull() {
      if (is_vert) {
        path_value = value;
        subtree_value = combine(combine(value, subtree_value_of(c[0])), subtree_value_of(c[1]));
      } else if (is_path) {
        path_value = combine(combine(path_value_of(c[0]), value), path_value_of(c[1]));
        subtree_value = combine(combine(subtree_value_of(c[0]), value), subtree_value_of(c[1]));
      } else {
        path_value = identity();
        subtree_value = combine(
            combine(subtree_value_of(c[0]), value),
            combine(subtree_value_of(c[2]), subtree_value_of(c[1]))
        );
      }
    }

    void rotate() {
      assert(!is_vert);
      assert(!root_of_auxiliary_tree());
      Node *parent = p;
      int x = dir();
      assert(x == 0 || x == 1);
      Node *child = c[!x];
      if (parent->p != nullptr) {
        parent->parent_child() = this;
      }
      p = parent->p;
      parent->c[x] = child;
      if (child != nullptr) {
        child->p = parent;
      }
      c[!x] = parent;
      parent->p = this;
      parent->pull();
    }

    void rotate_2(int child_dir) {
      assert(!is_vert);
      assert(!root_of_auxiliary_tree());
      assert(c[child_dir] != nullptr);
      assert(!c[child_dir]->is_vert);
      if (dir() == child_dir) {
        rotate();
        return;
      }
      Node *parent = p;
      int x = dir();
      assert(x == 0 || x == 1);
      assert(child_dir == !x);
      Node *child = c[child_dir]->c[!x];
      if (parent->p != nullptr) {
        parent->parent_child() = this;
      }
      p = parent->p;
      parent->c[x] = child;
      if (child != nullptr) {
        child->p = parent;
      }
      c[child_dir]->c[!x] = parent;
      parent->p = c[child_dir];
      parent->pull();
    }

    void splay_dir(int x) {
      while (!root_of_auxiliary_tree() && dir() == x) {
        if (!p->root_of_auxiliary_tree() && p->dir() == x) {
          p->rotate();
        }
        rotate();
      }
    }

    void splay_2(int child_dir) {
      assert(!is_vert && is_path);
      assert(c[child_dir] != nullptr && !c[child_dir]->is_vert);
      while (!root_of_auxiliary_tree()) {
        if (!p->root_of_auxiliary_tree()) {
          if (p->dir() == dir()) {
            p->rotate();
          } else {
            rotate_2(child_dir);
          }
        }
        rotate_2(child_dir);
      }
    }

    void splay_2() {
      assert(!is_vert && is_path);
      assert(!root_of_auxiliary_tree());
      p->splay_2(dir());
    }

    void splay_vertex() {
      assert(is_vert);
      if (root_of_auxiliary_tree()) {
        return;
      }
      p->splay_dir(dir());
      if (p->root_of_auxiliary_tree()) {
        return;
      }
      assert(p->dir() != dir());
      if (dir() == 1) {
        p->rotate();
      }
      assert(dir() == 0);
      p->splay_2();
      assert(dir() == 0);
      assert(p->dir() == 1);
      assert(p->p->root_of_auxiliary_tree());
    }

    void splay() {
      assert(!is_vert);
      while (!root_of_auxiliary_tree()) {
        if (!p->root_of_auxiliary_tree()) {
          if (p->dir() == dir()) {
            p->rotate();
          } else {
            rotate();
          }
        }
        rotate();
      }
    }

    Node *cut_right() {
      assert(is_vert && is_path);
      splay_vertex();
      if (root_of_auxiliary_tree() || dir() == 1) {
        assert(root_of_auxiliary_tree() || (dir() == 1 && p->root_of_auxiliary_tree()));
        assert(c[0] == nullptr);
        return nullptr;
      }
      Node *parent = p;
      assert(
          parent->root_of_auxiliary_tree() ||
          (parent->dir() == 1 && parent->p->root_of_auxiliary_tree())
      );
      assert(!parent->is_vert);
      assert(parent->is_path);
      assert(parent->c[0] == this);
      assert(parent->c[2] == nullptr);
      if (parent->p != nullptr) {
        parent->parent_child() = this;
      }
      p = parent->p;
      parent->is_path = false;
      parent->c[2] = parent->c[1];
      parent->c[0] = c[0];
      if (c[0] != nullptr) {
        c[0]->p = parent;
      }
      parent->c[1] = c[1];
      if (c[1] != nullptr) {
        c[1]->p = parent;
      }
      c[0] = nullptr;
      c[1] = parent;
      parent->p = this;
      assert(c[2] == nullptr);
      assert(c[0] == nullptr);
      parent->pull();
      return parent;
    }

    Node *splice_non_path() {
      assert(!is_path);
      assert(!is_vert);
      splay();
      assert(p != nullptr && p->is_vert && p->is_path);
      p->cut_right();
      if (!p->is_path) {
        rotate();
      }
      assert(p != nullptr && p->is_vert && p->is_path);
      assert(p->root_of_auxiliary_tree() || (p->dir() == 1 && p->p->root_of_auxiliary_tree()));
      assert(p->c[dir()] == this && p->c[!dir()] == nullptr);
      Node *parent = p;
      if (parent->p != nullptr) {
        parent->parent_child() = this;
      }
      p = parent->p;
      parent->c[0] = c[0];
      if (c[0] != nullptr) {
        c[0]->p = parent;
      }
      parent->c[1] = c[1];
      if (c[1] != nullptr) {
        c[1]->p = parent;
      }
      assert(c[2] != nullptr && c[2]->is_path);
      c[1] = c[2];
      c[0] = parent;
      parent->p = this;
      c[2] = nullptr;
      is_path = true;
      parent->pull();
      return parent;
    }

    Node *splice_all() {
      Node *res = this;
      for (Node *cur = this; cur != nullptr; cur = cur->p) {
        if (!cur->is_path) {
          res = cur->splice_non_path();
        }
        assert(cur->is_path);
      }
      return res;
    }

    Node *expose() {
      assert(is_vert);
      push_all();
      Node *res = splice_all();
      cut_right();
      pull_all();
      return res;
    }

    Node *expose_edge() {
      assert(!is_vert);
      push_all();
      Node *v = is_path ? c[1] : c[2];
      v->push();
      while (!v->is_vert) {
        v = v->c[0];
        v->push();
      }
      Node *res = v->splice_all();
      v->cut_right();
      v->pull_all();
      assert(p == nullptr);
      assert(v == c[1]);
      return res == v ? this : res;
    }

    Node *meld_path_end() {
      assert(p == nullptr);
      Node *rt = this;
      while (true) {
        rt->push();
        if (rt->is_vert) {
          break;
        }
        rt = rt->c[1];
      }
      assert(rt->is_vert);
      rt->splay_vertex();
      if (rt->c[0] != nullptr && rt->c[1] != nullptr) {
        Node *child = rt->c[1];
        while (true) {
          child->push();
          if (child->c[0] == nullptr) {
            break;
          }
          child = child->c[0];
        }
        child->splay();
        assert(child->c[0] == nullptr);
        child->c[0] = rt->c[0];
        child->c[0]->p = child;
        rt->c[0] = nullptr;
        child->pull();
      } else if (rt->c[0] != nullptr) {
        rt->c[1] = rt->c[0];
        rt->c[0] = nullptr;
      }
      assert(rt->c[0] == nullptr);
      return rt->pull_all();
    }

    void make_root() {
      expose();
      Node *rt = this;
      while (rt->p != nullptr) {
        assert(rt->dir() == 1);
        rt = rt->p;
      }
      rt->flip_path();
      rt->meld_path_end();
      expose();
      assert(p == nullptr);
    }
  };

  std::vector<Node *> verts, edge_nodes;

  static void link_nodes(Node *edge, Node *u, Node *v) {
    assert(edge != nullptr && u != nullptr && v != nullptr);
    assert(edge->c[0] == nullptr && edge->c[1] == nullptr && edge->c[2] == nullptr);
    u->expose();
    while (u->p != nullptr) {
      u = u->p;
    }
    v->make_root();
    assert(u->p == nullptr);
    assert(v->p == nullptr);
    edge->is_path = true;
    edge->is_vert = false;
    edge->alive = true;
    edge->c[0] = u;
    u->p = edge;
    edge->c[1] = v;
    v->p = edge;
    edge->pull();
  }

  static std::pair<Node *, Node *> cut_node(Node *edge) {
    assert(edge != nullptr && edge->alive && !edge->is_vert);
    edge->expose_edge();
    assert(edge->p == nullptr);
    assert(edge->is_path);
    Node *l = edge->c[0], *r = edge->c[1];
    assert(l != nullptr && r != nullptr);
    edge->c[0] = edge->c[1] = edge->c[2] = nullptr;
    l->p = r->p = nullptr;
    l = l->meld_path_end();
    edge->is_path = false;
    edge->alive = false;
    edge->pull();
    return {l, r};
  }

  static Node *get_path(Node *u, Node *v) {
    assert(u->is_vert && v->is_vert);
    u->make_root();
    v->expose();
    if (u == v) {
      assert(v->p == nullptr);
      return v;
    }
    assert(v->p->p == nullptr);
    return v->p;
  }

  static Node *get_subtree(Node *root, Node *u) {
    root->make_root();
    u->expose();
    return u;
  }

  static Node *lca_same_component(Node *u, Node *v) {
    u->expose();
    return v->expose();
  }

  static Node *maybe_lca_node(Node *u, Node *v) {
    u->expose();
    Node *up = u->p;
    assert(up == nullptr || up->p == nullptr);
    Node *res = v->expose();
    assert(v->p == nullptr || v->p->p == nullptr);
    if (u != v && up == u->p && (up == nullptr || up->p == nullptr)) {
      return nullptr;
    }
    return res;
  }

 public:
  explicit TopTree(int n, const T &value = T()) {
    verts.reserve(n);
    for (int i = 0; i < n; i++) {
      verts.push_back(new Node(value, i, -1));
    }
  }

  ~TopTree() {
    for (Node *v : verts) {
      delete v;
    }
    for (Node *e : edge_nodes) {
      delete e;
    }
  }

  TopTree(const TopTree &) = delete;
  TopTree &operator=(const TopTree &) = delete;
  int size() const { return static_cast<int>(verts.size()); }
  int edges() const { return static_cast<int>(edge_nodes.size()); }

  bool connected(int u, int v) {
    assert(0 <= u && u < size());
    assert(0 <= v && v < size());
    return u == v || maybe_lca_node(verts[u], verts[v]) != nullptr;
  }

  int link(int u, int v, const T &value = T()) {
    assert(0 <= u && u < size());
    assert(0 <= v && v < size());
    if (connected(u, v)) {
      return -1;
    }
    int id = edges();
    edge_nodes.push_back(new Node(value, -1, id));
    link_nodes(edge_nodes.back(), verts[u], verts[v]);
    return id;
  }

  void cut(int e) {
    assert(0 <= e && e < edges());
    cut_node(edge_nodes[e]);
  }

  void set_vertex(int u, const T &value) {
    assert(0 <= u && u < size());
    verts[u]->expose();
    verts[u]->value = value;
    verts[u]->pull_all();
  }

  void set_edge(int e, const T &value) {
    assert(0 <= e && e < edges());
    assert(edge_nodes[e]->alive);
    edge_nodes[e]->expose_edge();
    edge_nodes[e]->value = value;
    edge_nodes[e]->pull_all();
  }

  T path_query(int u, int v) {
    assert(connected(u, v));
    return get_path(verts[u], verts[v])->path_value;
  }

  T subtree_query(int root, int u) {
    assert(connected(root, u));
    return get_subtree(verts[root], verts[u])->subtree_value;
  }

  void reroot(int u) {
    assert(0 <= u && u < size());
    verts[u]->make_root();
  }

  int lca(int u, int v) {
    assert(connected(u, v));
    return lca_same_component(verts[u], verts[v])->vertex_id;
  }

  int maybe_lca(int u, int v) {
    assert(0 <= u && u < size());
    assert(0 <= v && v < size());
    Node *res = maybe_lca_node(verts[u], verts[v]);
    return res == nullptr ? -1 : res->vertex_id;
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  TopTree<int> tree(5);
  for (int i = 0; i < 5; i++) {
    tree.set_vertex(i, i + 1);
  }
  int e01 = tree.link(0, 1, 10);
  int e12 = tree.link(1, 2, 20);
  int e13 = tree.link(1, 3, 30);
  int e34 = tree.link(3, 4, 40);
  assert(e01 == 0 && e12 == 1 && e13 == 2 && e34 == 3);
  assert(tree.link(0, 4, 100) == -1);

  //       0
  //  v=10 |
  //       1
  // v=20 / \ v=30
  //     2   3
  //         | v=40
  //         4
  assert(tree.path_query(2, 4) == 104);
  assert(tree.subtree_query(0, 3) == 49);
  assert(tree.lca(2, 4) == 1);

  tree.cut(e13);

  // Cut edge 1-3:
  //
  //       0
  //  v=10 |
  //       1       3
  // v=20 /        | v=40
  //     2         4
  assert(!tree.connected(2, 4));
  assert(tree.maybe_lca(2, 4) == -1);
  e13 = tree.link(2, 4, 30);
  assert(e13 == 4);

  // Re-link 2-4 with value 30:
  //
  //       0
  //  v=10 |
  //       1        3
  // v=20 /         | v=40
  //     2----------4
  //         v=30
  assert(tree.path_query(0, 3) == 115);

  tree.set_edge(e34, 4);
  tree.set_vertex(4, 50);

  // Change edge 3-4 to value 4 and vertex 4 to value 50:
  //
  //       0
  //  v=10 |
  //       1        3
  // v=20 /         | v=4
  //     2----------4
  //        v=30   v=50
  assert(tree.subtree_query(2, 4) == 58);
  cout << tree.path_query(0, 3) << endl;
  return 0;
}
