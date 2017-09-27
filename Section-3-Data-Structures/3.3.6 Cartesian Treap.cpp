/*

Maintain a dynamically-sized array using a balanced binary search tree while
supporting both dynamic queries and updates of contiguous subarrays via the lazy
propagation technique. A treap maintains a balanced binary tree structure by
preserving the heap property on the randomly generated priority values of nodes,
thereby making insertions and deletions run in O(log n) with high probability.

The query operation is defined by an associative join_values() function which
satisfies join_values(x, join_values(y, z)) = join_values(join_values(x, y), z)
for all values x, y, and z in the array. The default code below assumes a
numerical array type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case the join_values()
function should be defined to return "a + b".

The update operation is defined by the join_value_with_delta() and join_deltas()
functions, which determines the change made to array values. These must satisfy:
- join_deltas(d1, join_deltas(d2, d3)) = join_deltas(join_deltas(d1, d2), d3).
- join_value_with_delta(join_values(v, ...(m times)..., v), d, m)) should be
  equal to join_values(join_value_with_delta(v, d, 1), ...(m times)).
- if a sequence d_1, ..., d_m of deltas is used to update a value v, then
  join_value_with_delta(v, join_deltas(d_1, ..., d_m), 1) should be equivalent
  to m sequential calls to join_value_with_delta(v, d_i, 1) for i = 1..m.
The default code below defines updates that "set" the chosen array index to a
new value. Another possible update operation is "increment", in which case
join_value_with_delta(v, d, len) should be defined to return "v + d*len" and
join_deltas(d1, d2) should be defined to return "d1 + d2".

This data structure shares every operation of one-dimensional segment trees in
this section, with the additional operations empty(), insert(), erase(),
push_back(), and pop_back() analogous to those of std::vector (here, insert()
and erase() both take an index instead of an iterator).

Time Complexity:
- O(n) per call to both constructors, where n is the size of the array.
- O(1) per call to size() and empty().
- O(log n) on average per call to all other operations.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for size() and empty().
- O(log n) auxiliary stack space for all other operations.

*/

#include <cstdlib>

template<class T>
class cartesian_treap {
  static T join_values(const T &a, const T &b) {
    return a < b ? a : b;
  }

  static T join_value_with_delta(const T &v, const T &d, int len) {
    return d;
  }

  static T join_deltas(const T &d1, const T &d2) {
    return d2;
  }

  struct node_t {
    static inline int rand32() {
      return (rand() & 0x7fff) | ((rand() & 0x7fff) << 15);
    }

    T value, subtree_value, delta;
    bool pending;
    int size, priority;
    node_t *left, *right;

    node_t(const T &v)
        : value(v), subtree_value(v), pending(false), size(1),
          priority(rand32()), left(NULL), right(NULL) {}
  } *root;

  static int size(node_t *n) {
    return (n == NULL) ? 0 : n->size;
  }

  static void update_value(node_t *n) {
    if (n == NULL) {
      return;
    }
    n->subtree_value = n->value;
    if (n->left != NULL) {
      n->subtree_value = join_values(n->subtree_value, n->left->subtree_value);
    }
    if (n->right != NULL) {
      n->subtree_value = join_values(n->subtree_value, n->right->subtree_value);
    }
    n->size = 1 + size(n->left) + size(n->right);
  }

  static void update_delta(node_t *n, const T &d) {
    if (n != NULL) {
      n->delta = n->pending ? join_deltas(n->delta, d) : d;
      n->pending = true;
    }
  }

  static void push_delta(node_t *n) {
    if (n == NULL || !n->pending) {
      return;
    }
    n->value = join_value_with_delta(n->value, n->delta, 1);
    n->subtree_value = join_value_with_delta(n->subtree_value, n->delta,
                                             n->size);
    if (n->size > 1) {
      update_delta(n->left, n->delta);
      update_delta(n->right, n->delta);
    }
    n->pending = false;
  }

  static void merge(node_t *&n, node_t *left, node_t *right) {
    push_delta(left);
    push_delta(right);
    if (left == NULL) {
      n = right;
    } else if (right == NULL) {
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

  static void split(node_t *n, node_t *&left, node_t *&right, int i) {
    push_delta(n);
    if (n == NULL) {
      left = right = NULL;
    } else if (i <= size(n->left)) {
      split(n->left, left, n->left, i);
      right = n;
    } else {
      split(n->right, n->right, right, i - size(n->left) - 1);
      left = n;
    }
    update_value(n);
  }

  static void insert(node_t *&n, node_t *new_node, int i) {
    push_delta(n);
    if (n == NULL) {
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

  static void erase(node_t *&n, int i) {
    push_delta(n);
    if (i == size(n->left)) {
      delete n;
      merge(n, n->left, n->right);
    } else if (i < size(n->left)) {
      erase(n->left, i);
    } else {
      erase(n->right, i - size(n->left) - 1);
    }
    update_value(n);
  }

  static node_t* select(node_t *n, int i) {
    push_delta(n);
    if (i < size(n->left)) {
      return select(n->left, i);
    }
    if (i > size(n->left)) {
      return select(n->right, i - size(n->left) - 1);
    }
    return n;
  }

  void clean_up(node_t *&n) {
    if (n != NULL) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  cartesian_treap(int n = 0, const T &v = T()) : root(NULL) {
    for (int i = 0; i < n; i++) {
      push_back(v);
    }
  }

  template<class It>
  cartesian_treap(It lo, It hi) : root(NULL) {
    for (; lo != hi; ++lo) {
      push_back(*lo);
    }
  }

  ~cartesian_treap() {
    clean_up(root);
  }

  int size() const {
    return size(root);
  }

  bool empty() const {
    return root == NULL;
  }

  void insert(int i, const T &v) {
    insert(root, new node_t(v), i);
  }

  void erase(int i) {
    erase(root, i);
  }

  void push_back(const T &v) {
    insert(size(), v);
  }

  void pop_back() {
    erase(size() - 1);
  }

  T at(int i) const {
    return select(root, i)->value;
  }

  T query(int lo, int hi) {
    node_t *l1, *r1, *l2, *r2, *t;
    split(root, l1, r1, hi + 1);
    split(l1, l2, r2, lo);
    T res = r2->subtree_value;
    merge(t, l2, r2);
    merge(root, t, r1);
    return res;
  }

  void update(int i, const T &d) {
    update(i, i, d);
  }

  void update(int lo, int hi, const T &d) {
    node_t *l1, *r1, *l2, *r2, *t;
    split(root, l1, r1, hi + 1);
    split(l1, l2, r2, lo);
    update_delta(r2, d);
    merge(t, l2, r2);
    merge(root, t, r1);
  }
};

/*** Example Usage and Output:

Values: -99 -2 1 8 10 11 (min: -99)
Values: -90 -2 1 8 10 11 (min: -90)
Values: 2 2 1 8 10 11 (min: 1)

***/

#include <iostream>
using namespace std;

void print(cartesian_treap<int> &t) {
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << " (min: " << t.query(0, t.size() - 1) << ")" << endl;
}

int main() {
  int arr[5] = {99, -2, 1, 8, 10};
  cartesian_treap<int> t(arr, arr + 5);
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
