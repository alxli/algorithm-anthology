/*

Maintain an undirected forest dynamically using Euler Tour Trees. Each tree is represented by a
sequence of directed edge occurrences in an implicit treap: an undirected edge $(u, v)$ contributes
two nodes, $(u \to v)$ and $(v \to u)$. Rerooting a represented tree is a cyclic rotation of this
sequence, while `link` concatenates two tours with the new edge occurrences and `cut` removes the
two occurrences of an existing edge, splitting one tour into two.

This version supports dynamic connectivity, `link`, `cut`, rerooting, and listing the vertices in a
connected component. It does not store vertex aggregates. To add component aggregates, a common
extension is to insert one self-loop occurrence per vertex and maintain aggregate data in the treap.

- `EulerTourTree(n)` constructs a forest on vertices numbered [$0$, `n`).
- `connected(u, v)` returns whether `u` and `v` are in the same tree.
- `link(u, v)` adds an edge between different trees and returns false if the edge would create a
  cycle or already exists.
- `cut(u, v)` removes an existing edge and returns false if it is not present.
- `reroot(u)` rotates the Euler tour of `u`'s tree so an occurrence leaving `u` is first.
- `component_vertices(u)` returns the distinct vertices in `u`'s tree.

Time Complexity:
- O(log m) expected per call to `connected`, `link`, `cut`, and `reroot`, where $m$ is the number of
  edges currently in the forest.
- O(k) expected per call to `component_vertices`, where $k$ is the number of directed edge
  occurrences in the component.

Space Complexity:
- O(n + m) for the forest.
- O(log m) auxiliary recursion stack for treap operations.

*/

#include <algorithm>
#include <cassert>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>

class EulerTourTree {
  struct Node {
    int u, v, priority, size;
    Node *left, *right, *parent;

    Node(int u, int v, int priority)
        : u(u), v(v), priority(priority), size(1), left(nullptr), right(nullptr), parent(nullptr) {}
  };

  int n;
  std::mt19937 rng;
  std::vector<std::unordered_map<int, Node *>> adj;
  std::vector<Node *> allocated;

  static int size(Node *t) { return t == nullptr ? 0 : t->size; }

  static void set_parent(Node *t, Node *p) {
    if (t != nullptr) {
      t->parent = p;
    }
  }

  static void update(Node *t) {
    if (t != nullptr) {
      t->size = 1 + size(t->left) + size(t->right);
      set_parent(t->left, t);
      set_parent(t->right, t);
    }
  }

  static Node *root(Node *t) {
    if (t == nullptr) {
      return nullptr;
    }
    while (t->parent != nullptr) {
      t = t->parent;
    }
    return t;
  }

  static int index(Node *t) {
    int res = size(t->left);
    while (t->parent != nullptr) {
      if (t == t->parent->right) {
        res += 1 + size(t->parent->left);
      }
      t = t->parent;
    }
    return res;
  }

  static Node *merge(Node *a, Node *b) {
    if (a == nullptr) {
      return b;
    }
    if (b == nullptr) {
      return a;
    }
    if (a->priority > b->priority) {
      a->right = merge(a->right, b);
      update(a);
      a->parent = nullptr;
      return a;
    }
    b->left = merge(a, b->left);
    update(b);
    b->parent = nullptr;
    return b;
  }

  static std::pair<Node *, Node *> split(Node *t, int left_size) {
    if (t == nullptr) {
      return {nullptr, nullptr};
    }
    if (size(t->left) >= left_size) {
      auto [a, b] = split(t->left, left_size);
      t->left = b;
      update(t);
      set_parent(a, nullptr);
      t->parent = nullptr;
      return {a, t};
    }
    auto [a, b] = split(t->right, left_size - size(t->left) - 1);
    t->right = a;
    update(t);
    set_parent(b, nullptr);
    t->parent = nullptr;
    return {t, b};
  }

  static Node *make_first(Node *x) {
    auto [a, b] = split(root(x), index(x));
    return merge(b, a);
  }

  static Node *remove_first(Node *t) {
    auto [first, rest] = split(t, 1);
    if (first != nullptr) {
      first->left = first->right = first->parent = nullptr;
      first->size = 1;
    }
    return rest;
  }

  Node *new_node(int u, int v) {
    Node *node = new Node(u, v, std::uniform_int_distribution<int>()(rng));
    allocated.push_back(node);
    return node;
  }

  Node *any_occurrence(int u) const { return adj[u].empty() ? nullptr : adj[u].begin()->second; }

  static void collect(Node *t, std::vector<int> &out) {
    if (t == nullptr) {
      return;
    }
    collect(t->left, out);
    out.push_back(t->u);
    collect(t->right, out);
  }

 public:
  explicit EulerTourTree(int n) : n(n), rng(1234567), adj(n) {}

  ~EulerTourTree() {
    for (Node *node : allocated) {
      delete node;
    }
  }

  bool connected(int u, int v) const {
    assert(0 <= u && u < n && 0 <= v && v < n);
    if (u == v) {
      return true;
    }
    Node *a = any_occurrence(u), *b = any_occurrence(v);
    return a != nullptr && b != nullptr && root(a) == root(b);
  }

  void reroot(int u) {
    assert(0 <= u && u < n);
    Node *a = any_occurrence(u);
    if (a != nullptr) {
      make_first(a);
    }
  }

  bool link(int u, int v) {
    assert(0 <= u && u < n && 0 <= v && v < n);
    if (u == v || adj[u].count(v) || connected(u, v)) {
      return false;
    }
    Node *tu = any_occurrence(u), *tv = any_occurrence(v);
    if (tu != nullptr) {
      tu = make_first(tu);
    }
    if (tv != nullptr) {
      tv = make_first(tv);
    }
    Node *uv = new_node(u, v), *vu = new_node(v, u);
    adj[u][v] = uv;
    adj[v][u] = vu;
    merge(merge(tu, uv), merge(tv, vu));
    return true;
  }

  bool cut(int u, int v) {
    assert(0 <= u && u < n && 0 <= v && v < n);
    auto it = adj[u].find(v);
    if (it == adj[u].end()) {
      return false;
    }
    Node *uv = it->second, *vu = adj[v][u];
    make_first(uv);
    auto [left, right] = split(root(uv), index(vu));
    remove_first(left);   // Remove uv.
    remove_first(right);  // Remove vu.
    adj[u].erase(v);
    adj[v].erase(u);
    return true;
  }

  std::vector<int> component_vertices(int u) const {
    assert(0 <= u && u < n);
    Node *a = any_occurrence(u);
    if (a == nullptr) {
      return {u};
    }
    std::vector<int> occurrences, res;
    collect(root(a), occurrences);
    std::set<int> seen(occurrences.begin(), occurrences.end());
    res.assign(seen.begin(), seen.end());
    return res;
  }
};

/*** Example Usage ***/

int main() {
  EulerTourTree ett(6);
  assert(!ett.connected(0, 1));
  assert(ett.connected(0, 0));

  assert(ett.link(0, 1));
  assert(ett.link(1, 2));
  assert(ett.link(3, 4));
  assert(ett.connected(0, 2));
  assert(!ett.connected(0, 4));
  assert(!ett.link(0, 2));  // Would create a cycle.

  auto c = ett.component_vertices(1);
  assert((c == std::vector<int>{0, 1, 2}));

  ett.reroot(2);
  assert(ett.connected(0, 2));
  assert(ett.cut(1, 2));
  assert(!ett.connected(0, 2));
  assert((ett.component_vertices(2) == std::vector<int>{2}));

  assert(ett.link(2, 3));
  assert(ett.connected(0, 4) == false);
  assert(ett.connected(2, 4));
  assert(!ett.cut(0, 5));
  return 0;
}
