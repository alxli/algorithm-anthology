/*

Maintain a dynamically-sized array using a balanced binary search tree while supporting both dynamic
queries and updates of contiguous subarrays via the lazy propagation technique. A treap maintains a
balanced binary tree structure by preserving the heap property on the randomly generated priority
values of nodes, thereby making insertions and deletions run in O(log n) with high probability.

The query operation is defined by an associative aggregate function `combine(a, b)`. The default
code below assumes a numerical array type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case `combine(a, b)` should return $a + b$.

Range updates are defined by `apply_delta(v, d, len)`, which applies an update delta `d` to an
aggregate summary `v` representing `len` array values, and by `compose_deltas(old, d)`, which
combines a pending older delta with a newer delta in that order. These functions do not support
arbitrary combinations: applying a delta to a combined segment must be equivalent to applying it to
each child segment and then combining the results, and composed deltas must be equivalent to
performing their updates sequentially. The default code below defines range assignment. For range
increment, `compose_deltas(old, d)` should return `old + d`; `apply_delta(v, d, len)` should return
`v + d` for range-min/range-max queries, and `v + d * len` for range-sum queries.

This data structure shares every operation of one-dimensional segment trees in this section, with
the additional operations `empty()`, `insert()`, `erase()`, `push_back()`, and `pop_back()`
analogous to those of `std::vector` (here, `insert()` and `erase()` both take an index instead of an
iterator).

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()` and `empty()`.
- O(log n) on average per call to all other operations.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for `size()` and `empty()`.
- O(log n) auxiliary stack space for all other operations.

*/

#include <cstdlib>

template<class T>
class ImplicitTreap {
  static T combine(const T &a, const T &b) { return a < b ? a : b; }
  static T apply_delta(const T &v, const T &d, long long len) { return d; }
  static T compose_deltas(const T &d1, const T &d2) { return d2; }

  struct Node {
    static inline int rand32() { return (rand() & 0x7fff) | ((rand() & 0x7fff) << 15); }

    T value, subtree_value, delta;
    bool pending;
    int size, priority;
    Node *left, *right;

    explicit Node(const T &v)
        : value(v),
          subtree_value(v),
          pending(false),
          size(1),
          priority(rand32()),
          left(nullptr),
          right(nullptr) {}
  } *root;

  static int size(Node *n) { return (n == nullptr) ? 0 : n->size; }

  static void update_value(Node *n) {
    if (n == nullptr) {
      return;
    }
    n->subtree_value = n->value;
    if (n->left != nullptr) {
      n->subtree_value = combine(n->subtree_value, n->left->subtree_value);
    }
    if (n->right != nullptr) {
      n->subtree_value = combine(n->subtree_value, n->right->subtree_value);
    }
    n->size = 1 + size(n->left) + size(n->right);
  }

  static void update_delta(Node *n, const T &d) {
    if (n != nullptr) {
      n->value = apply_delta(n->value, d, 1);
      n->subtree_value = apply_delta(n->subtree_value, d, n->size);
      n->delta = n->pending ? compose_deltas(n->delta, d) : d;
      n->pending = true;
    }
  }

  static void push_delta(Node *n) {
    if (n == nullptr || !n->pending) {
      return;
    }
    if (n->size > 1) {
      update_delta(n->left, n->delta);
      update_delta(n->right, n->delta);
    }
    n->pending = false;
  }

  static void merge(Node *&n, Node *left, Node *right) {
    push_delta(left);
    push_delta(right);
    if (left == nullptr) {
      n = right;
    } else if (right == nullptr) {
      n = left;
    } else if (left->priority < right->priority) {
      merge(left->right, left->right, right);
      n = left;
    } else {
      merge(right->left, left, right->left);
      n = right;
    }
    update_value(n);
  }

  static void split(Node *n, Node *&left, Node *&right, int i) {
    push_delta(n);
    if (n == nullptr) {
      left = right = nullptr;
    } else if (i <= size(n->left)) {
      split(n->left, left, n->left, i);
      right = n;
    } else {
      split(n->right, n->right, right, i - size(n->left) - 1);
      left = n;
    }
    update_value(n);
  }

  static void insert(Node *&n, Node *new_node, int i) {
    push_delta(n);
    if (n == nullptr) {
      n = new_node;
    } else if (new_node->priority < n->priority) {
      split(n, new_node->left, new_node->right, i);
      n = new_node;
    } else if (i <= size(n->left)) {
      insert(n->left, new_node, i);
    } else {
      insert(n->right, new_node, i - size(n->left) - 1);
    }
    update_value(n);
  }

  static void erase(Node *&n, int i) {
    push_delta(n);
    if (i == size(n->left)) {
      Node *left = n->left, *right = n->right;
      delete n;
      merge(n, left, right);
    } else if (i < size(n->left)) {
      erase(n->left, i);
    } else {
      erase(n->right, i - size(n->left) - 1);
    }
    update_value(n);
  }

  static Node *select(Node *n, int i) {
    push_delta(n);
    if (i < size(n->left)) {
      return select(n->left, i);
    }
    if (i > size(n->left)) {
      return select(n->right, i - size(n->left) - 1);
    }
    return n;
  }

  void clean_up(Node *&n) {
    if (n != nullptr) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  explicit ImplicitTreap(int n = 0, const T &v = T()) : root(nullptr) {
    for (int i = 0; i < n; i++) {
      push_back(v);
    }
  }

  template<class It>
  ImplicitTreap(It lo, It hi) : root(nullptr) {
    for (; lo != hi; ++lo) {
      push_back(*lo);
    }
  }

  ~ImplicitTreap() { clean_up(root); }
  int size() const { return size(root); }
  bool empty() const { return root == nullptr; }
  void insert(int i, const T &v) { insert(root, new Node(v), i); }
  void erase(int i) { erase(root, i); }
  void push_back(const T &v) { insert(size(), v); }
  void pop_back() { erase(size() - 1); }
  T at(int i) const { return select(root, i)->value; }

  T query(int lo, int hi) {
    Node *l1, *r1, *l2, *r2, *t;
    split(root, l1, r1, hi + 1);
    split(l1, l2, r2, lo);
    T res = r2->subtree_value;
    merge(t, l2, r2);
    merge(root, t, r1);
    return res;
  }

  void update(int lo, int hi, const T &d) {
    Node *l1, *r1, *l2, *r2, *t;
    split(root, l1, r1, hi + 1);
    split(l1, l2, r2, lo);
    update_delta(r2, d);
    merge(t, l2, r2);
    merge(root, t, r1);
  }

  void update(int i, const T &d) { update(i, i, d); }
};

/*** Example Usage and Output:

Values: 99 -2 1 8 10 11 (min: -2)
Values: 90 -2 1 8 10 11 (min: -2)
Values: 2 2 1 8 10 11 (min: 1)

***/

#include <iostream>
#include <vector>
using namespace std;

void print(ImplicitTreap<int> &t) {
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << " (min: " << t.query(0, t.size() - 1) << ")" << endl;
}

int main() {
  vector<int> arr{99, -2, 1, 8, 10};
  ImplicitTreap<int> t(arr.begin(), arr.end());
  t.push_back(11);
  t.push_back(12);
  t.pop_back();
  print(t);
  t.insert(0, 90);
  t.erase(1);
  print(t);
  t.update(0, 1, 2);
  print(t);
  return 0;
}
