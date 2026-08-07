/*

Maintain a dynamic forest with values on both nodes and edges, supporting connectivity, linking and
cutting edges, rerooting, and path and rooted-subtree aggregate queries. A top tree represents each
tree as a hierarchy of clusters, each with at most two boundary nodes. Path clusters summarize
contiguous paths, while non-path clusters rake side subtrees into a boundary node. Changing the
exposed path requires only a logarithmic number of local rotations and recomputations.

This is an advanced alternative to link/cut trees. Link/cut trees are usually simpler when only path
queries are needed, but top trees handle when the dynamic forest also needs rooted-subtree queries.

The aggregate operation is defined by an associative `combine(a, b)`. The default code below
computes sums over node and edge values. For minimum queries, use `std::min(a, b)`. For
non-commutative aggregates, store enough information in `T` to support reversing a path, and update
`flip_path()` accordingly.

- `TopTree(n, value = T())` constructs a forest on nodes $[0, `n`)$ with every node value
  initialized to `value`.
- `size()` returns the number of nodes in the forest.
- `edges()` returns the number of edge nodes that have been created.
- `connected(u, v)` returns whether nodes `u` and `v` are in the same tree.
- `link(u, v, value = T())` adds an edge with value `value` between different trees, returning its
  edge id or $-1$ if the edge would create a cycle.
- `cut(e)` removes edge id `e`. The edge id must currently be present.
- `set_node(u, value)` changes node `u`'s value to `value`.
- `set_edge(e, value)` changes edge `e`'s value to `value`. The edge id must currently be present.
- `path_query(u, v)` returns the aggregate over all node and edge values on the path from `u` to
  `v`. The nodes must be connected.
- `subtree_query(root, u)` reroots the represented tree at `root` and returns the aggregate over the
  rooted subtree of `u`.
- `reroot(u)` makes node `u` the root of its represented tree.
- `lca(u, v)` returns the lowest common ancestor of `u` and `v` relative to the current root. The
  nodes must be connected.
- `maybe_lca(u, v)` returns the lowest common ancestor of `u` and `v`, or $-1$ if they are
  disconnected.

Time Complexity:
- O(n) per call to the constructor, and O(1) per call to `size()` and `edges()`.
- O(log n) amortized per call to all other operations, where $n$ is the number of nodes and edges in
  the represented tree.

Space Complexity:
- O(n + m) for the internal nodes representing graph nodes and edges, where $m$ is the number of
  edges ever added.
- O(log n) auxiliary stack space for all operations other than `size()` and `edges()`.

*/

#include <array>
#include <cassert>
#include <utility>
#include <vector>

template<typename T>
class TopTree {
  static T combine(const T &a, const T &b) { return a + b; }

  struct Node {
    Node *p;
    std::array<Node *, 3> c;  // c[0] and c[1] are auxiliary children; c[2] is the raked path child.
    T value, path_value, subtree_value;
    int graph_node_id;
    bool is_path, is_graph_node, lazy_flip_path, alive;

    Node(const T &value, int graph_node_id)
        : p(nullptr),
          c{nullptr, nullptr, nullptr},
          value(value),
          path_value(value),
          subtree_value(value),
          graph_node_id(graph_node_id),
          is_path(graph_node_id != -1),
          is_graph_node(graph_node_id != -1),
          lazy_flip_path(false),
          alive(graph_node_id != -1) {}

    int dir() const {
      assert(p != nullptr);
      for (int i = 0; i < 3; i++) {
        if (this == p->c[i]) {
          return i;
        }
      }
      assert(false);
      return -1;
    }

    Node *&parent_child() const { return p->c[dir()]; }
    bool root_of_auxiliary_tree() const { return p == nullptr || p->is_path != is_path; }

    void set_child(int i, Node *child) {
      c[i] = child;
      if (child != nullptr) {
        child->p = this;
      }
    }

    static void replace_in_parent(Node *old, Node *replacement) {
      assert(replacement->p == old);
      if (old->p != nullptr) {
        old->parent_child() = replacement;
      }
      replacement->p = old->p;
      old->p = nullptr;
    }

    void flip_path() {
      assert(is_path);
      std::swap(c[0], c[1]);
      lazy_flip_path = !lazy_flip_path;
    }

    void push() {
      if (lazy_flip_path) {
        assert(is_path);
        if (!is_graph_node) {
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

    void prepend_subtree(Node *n) {
      if (n != nullptr) {
        subtree_value = combine(n->subtree_value, subtree_value);
      }
    }

    void append_subtree(Node *n) {
      if (n != nullptr) {
        subtree_value = combine(subtree_value, n->subtree_value);
      }
    }

    void pull() {
      subtree_value = value;
      if (is_graph_node) {
        path_value = value;
        append_subtree(c[0]);
        append_subtree(c[1]);
      } else if (is_path) {
        path_value = value;
        if (c[0] != nullptr) {
          path_value = combine(c[0]->path_value, path_value);
          prepend_subtree(c[0]);
        }
        if (c[1] != nullptr) {
          path_value = combine(path_value, c[1]->path_value);
          append_subtree(c[1]);
        }
      } else {
        prepend_subtree(c[0]);
        append_subtree(c[2]);
        append_subtree(c[1]);
      }
    }

    void rotate() {
      assert(!is_graph_node);
      assert(!root_of_auxiliary_tree());
      Node *parent = p;
      int x = dir();
      assert(x == 0 || x == 1);
      Node *child = c[!x];
      replace_in_parent(parent, this);
      parent->set_child(x, child);
      set_child(!x, parent);
      parent->pull();
    }

    void rotate2(int child_dir) {
      assert(!is_graph_node);
      assert(!root_of_auxiliary_tree());
      assert(c[child_dir] != nullptr);
      assert(!c[child_dir]->is_graph_node);
      if (dir() == child_dir) {
        rotate();
        return;
      }
      Node *parent = p;
      int x = dir();
      assert(x == 0 || x == 1);
      assert(child_dir == !x);
      Node *middle = c[child_dir];
      Node *child = middle->c[!x];
      replace_in_parent(parent, this);
      parent->set_child(x, child);
      middle->set_child(!x, parent);
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

    void splay2(int child_dir) {
      assert(!is_graph_node && is_path);
      assert(c[child_dir] != nullptr && !c[child_dir]->is_graph_node);
      while (!root_of_auxiliary_tree()) {
        if (!p->root_of_auxiliary_tree()) {
          if (p->dir() == dir()) {
            p->rotate();
          } else {
            rotate2(child_dir);
          }
        }
        rotate2(child_dir);
      }
    }

    void splay2() {
      assert(!is_graph_node && is_path);
      assert(!root_of_auxiliary_tree());
      p->splay2(dir());
    }

    void splay_graph_node() {
      assert(is_graph_node);
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
      p->splay2();
      assert(dir() == 0);
      assert(p->dir() == 1);
      assert(p->p->root_of_auxiliary_tree());
    }

    void splay() {
      assert(!is_graph_node);
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

    void cut_right() {
      assert(is_graph_node && is_path);
      splay_graph_node();
      if (root_of_auxiliary_tree() || dir() == 1) {
        assert(root_of_auxiliary_tree() || (dir() == 1 && p->root_of_auxiliary_tree()));
        assert(c[0] == nullptr);
        return;
      }
      Node *parent = p;
      assert(
          parent->root_of_auxiliary_tree() ||
          (parent->dir() == 1 && parent->p->root_of_auxiliary_tree())
      );
      assert(!parent->is_graph_node);
      assert(parent->is_path);
      assert(parent->c[0] == this);
      assert(parent->c[2] == nullptr);
      replace_in_parent(parent, this);
      parent->is_path = false;
      parent->set_child(2, parent->c[1]);
      parent->set_child(0, c[0]);
      parent->set_child(1, c[1]);
      set_child(0, nullptr);
      set_child(1, parent);
      assert(c[2] == nullptr);
      assert(c[0] == nullptr);
      parent->pull();
    }

    Node *splice_non_path() {
      assert(!is_path);
      assert(!is_graph_node);
      splay();
      assert(p != nullptr && p->is_graph_node && p->is_path);
      p->cut_right();
      if (!p->is_path) {
        rotate();
      }
      assert(p != nullptr && p->is_graph_node && p->is_path);
      assert(p->root_of_auxiliary_tree() || (p->dir() == 1 && p->p->root_of_auxiliary_tree()));
      assert(p->c[dir()] == this && p->c[!dir()] == nullptr);
      Node *parent = p;
      replace_in_parent(parent, this);
      parent->set_child(0, c[0]);
      parent->set_child(1, c[1]);
      assert(c[2] != nullptr && c[2]->is_path);
      set_child(1, c[2]);
      set_child(0, parent);
      set_child(2, nullptr);
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
      assert(is_graph_node);
      push_all();
      Node *res = splice_all();
      cut_right();
      pull_all();
      return res;
    }

    void expose_edge() {
      assert(!is_graph_node);
      push_all();
      Node *v = is_path ? c[1] : c[2];
      v->push();
      while (!v->is_graph_node) {
        v = v->c[0];
        v->push();
      }
      v->splice_all();
      v->cut_right();
      v->pull_all();
      assert(p == nullptr);
      assert(v == c[1]);
    }

    Node *meld_path_end() {
      assert(p == nullptr);
      Node *rt = this;
      while (true) {
        rt->push();
        if (rt->is_graph_node) {
          break;
        }
        rt = rt->c[1];
      }
      assert(rt->is_graph_node);
      rt->splay_graph_node();
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
        child->set_child(0, rt->c[0]);
        rt->set_child(0, nullptr);
        child->pull();
      } else if (rt->c[0] != nullptr) {
        rt->set_child(1, rt->c[0]);
        rt->set_child(0, nullptr);
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

  std::vector<Node *> graph_nodes, edge_nodes;

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
    edge->is_graph_node = false;
    edge->alive = true;
    edge->set_child(0, u);
    edge->set_child(1, v);
    edge->pull();
  }

  static void cut_node(Node *edge) {
    assert(edge != nullptr && edge->alive && !edge->is_graph_node);
    edge->expose_edge();
    assert(edge->p == nullptr);
    assert(edge->is_path);
    Node *l = edge->c[0], *r = edge->c[1];
    assert(l != nullptr && r != nullptr);
    edge->c.fill(nullptr);
    l->p = r->p = nullptr;
    l->meld_path_end();
    edge->is_path = false;
    edge->alive = false;
    edge->pull();
  }

  static Node *get_path(Node *u, Node *v) {
    assert(u->is_graph_node && v->is_graph_node);
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
    assert(n >= 0);
    graph_nodes.reserve(n);
    for (int i = 0; i < n; i++) {
      graph_nodes.push_back(new Node(value, i));
    }
  }

  ~TopTree() {
    for (Node *v : graph_nodes) {
      delete v;
    }
    for (Node *e : edge_nodes) {
      delete e;
    }
  }

  TopTree(const TopTree &) = delete;
  TopTree &operator=(const TopTree &) = delete;
  int size() const { return static_cast<int>(graph_nodes.size()); }
  int edges() const { return static_cast<int>(edge_nodes.size()); }

  bool connected(int u, int v) {
    assert(0 <= u && u < size());
    assert(0 <= v && v < size());
    return u == v || maybe_lca_node(graph_nodes[u], graph_nodes[v]) != nullptr;
  }

  int link(int u, int v, const T &value = T()) {
    assert(0 <= u && u < size());
    assert(0 <= v && v < size());
    if (connected(u, v)) {
      return -1;
    }
    int id = edges();
    edge_nodes.push_back(new Node(value, -1));
    link_nodes(edge_nodes.back(), graph_nodes[u], graph_nodes[v]);
    return id;
  }

  void cut(int e) {
    assert(0 <= e && e < edges());
    cut_node(edge_nodes[e]);
  }

  void set_node(int u, const T &value) {
    assert(0 <= u && u < size());
    graph_nodes[u]->expose();
    graph_nodes[u]->value = value;
    graph_nodes[u]->pull_all();
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
    return get_path(graph_nodes[u], graph_nodes[v])->path_value;
  }

  T subtree_query(int root, int u) {
    assert(connected(root, u));
    return get_subtree(graph_nodes[root], graph_nodes[u])->subtree_value;
  }

  void reroot(int u) {
    assert(0 <= u && u < size());
    graph_nodes[u]->make_root();
  }

  int lca(int u, int v) {
    assert(connected(u, v));
    graph_nodes[u]->expose();
    return graph_nodes[v]->expose()->graph_node_id;
  }

  int maybe_lca(int u, int v) {
    assert(0 <= u && u < size());
    assert(0 <= v && v < size());
    Node *res = maybe_lca_node(graph_nodes[u], graph_nodes[v]);
    return res == nullptr ? -1 : res->graph_node_id;
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  TopTree<int> t(5);
  for (int i = 0; i < 5; i++) {
    t.set_node(i, i + 1);
  }
  int e01 = t.link(0, 1, 10);
  int e12 = t.link(1, 2, 20);
  int e13 = t.link(1, 3, 30);
  int e34 = t.link(3, 4, 40);
  assert(e01 == 0 && e12 == 1 && e13 == 2 && e34 == 3);
  assert(t.link(0, 4, 100) == -1);

  //       0
  //  v=10 |
  //       1
  // v=20 / \ v=30
  //     2   3
  //         | v=40
  //         4
  assert(t.path_query(2, 4) == 104);
  assert(t.subtree_query(0, 3) == 49);
  assert(t.lca(2, 4) == 1);

  // Cut edge 1-3:
  //
  //       0
  //  v=10 |
  //       1       3
  // v=20 /        | v=40
  //     2         4
  t.cut(e13);
  assert(!t.connected(2, 4));
  assert(t.maybe_lca(2, 4) == -1);

  // Re-link 2-4 with value 30:
  //
  //       0
  //  v=10 |
  //       1        3
  // v=20 /         | v=40
  //     2----------4
  //         v=30
  e13 = t.link(2, 4, 30);
  assert(e13 == 4);
  assert(t.path_query(0, 3) == 115);

  // Change edge 3-4 to value 4 and node 4 to value 50:
  //
  //       0
  //  v=10 |
  //       1        3
  // v=20 /         | v=4
  //     2----------4
  //        v=30   v=50
  t.set_edge(e34, 4);
  t.set_node(4, 50);
  assert(t.subtree_query(2, 4) == 58);
  assert(t.path_query(0, 3) == 124);
  return 0;
}
