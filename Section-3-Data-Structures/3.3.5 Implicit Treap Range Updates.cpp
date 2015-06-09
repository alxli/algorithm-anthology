/*

3.3.5 - Implicit Treap with Range Updates

Description: A treap is a self-balancing binary search tree that
uses randomization to maintain a low height. In this version,
it is used emulate the operations of an std::vector with a tradeoff
of increasing the running time of push_back() and at() from O(1) to
O(log N), while decreasing the running time of insert() and erase()
from O(N) to O(log N). Furthermore, this version supports the same
operations as a segment tree with lazy propagation, allowing range
updates and queries to be performed in O(log N).

Time Complexity: Assuming the join functions are O(1):
insert(), push_back(), erase(), at(), modify(), and query()
are O(log N). size() and empty() are O(1). walk() is O(N).

Space Complexity: O(N) on the size of the array.

Note: This implementation is 0-based, meaning that all
indices from 0 to size() - 1, inclusive, are accessible.

*/

#include <climits> /* INT_MIN */
#include <cstdlib> /* srand(), rand() */
#include <ctime>   /* time() */

template<class T> class implicit_treap {
  //Modify the following 5 functions to implement your custom
  //operations on the tree. This implements the Add/Max operations.
  //Operations like Add/Sum, Set/Max can also be implemented.
  static inline T join_values(const T & a, const T & b) {
    return a > b ? a : b;
  }

  static inline T join_deltas(const T & d1, const T & d2) {
    return d1 + d2;
  }

  static inline T join_value_with_delta(const T & v, const T & d, int len) {
    return v + d;
  }

  static inline T null_delta() { return 0; }
  static inline T null_value() { return INT_MIN; }

  struct node_t {
    static inline int rand_int32(int l, int h) { //random number in [l, h]
      return l + ((rand()&0x7fff) | ((rand()&0x7fff) << 15)) % (h - l + 1);
    }

    T value, subtree_value, delta;
    int count, priority;
    node_t *L, *R;

    node_t(const T & val) {
      value = subtree_value = val;
      delta = null_delta();
      count = 1;
      L = R = 0;
      priority = rand_int32(0, 1 << 30);
    }
  } *root;

  static int count(node_t * n) {
    return n ? n->count : 0;
  }

  static T subtree_value(node_t * n) {
    return n ? n->subtree_value : null_value();
  }

  static void update(node_t * n) {
    if (n == 0) return;
    n->subtree_value = join_values(join_values(subtree_value(n->L), n->value),
                                   subtree_value(n->R));
    n->count = 1 + count(n->L) + count(n->R);
  }

  static void apply_delta(node_t * n, const T & delta) {
    if (n == 0) return;
    n->delta = join_deltas(n->delta, delta);
    n->value = join_value_with_delta(n->value, delta, 1);
    n->subtree_value = join_value_with_delta(n->subtree_value, delta, n->count);
  }

  static void push_delta(node_t * n) {
    if (n == 0) return;
    apply_delta(n->L, n->delta);
    apply_delta(n->R, n->delta);
    n->delta = null_delta();
  }

  static void merge(node_t *& n, node_t * L, node_t * R) {
    push_delta(L);
    push_delta(R);
    if (L == 0) n = R;
    else if (R == 0) n = L;
    else if (L->priority < R->priority)
      merge(L->R, L->R, R), n = L;
    else
      merge(R->L, L, R->L), n = R;
    update(n);
  }

  static void split(node_t * n, node_t *& L, node_t *& R, int key) {
    push_delta(n);
    if (n == 0) L = R = 0;
    else if (key <= count(n->L))
      split(n->L, L, n->L, key), R = n;
    else
      split(n->R, n->R, R, key - count(n->L) - 1), L = n;
    update(n);
  }

  static void insert(node_t *& n, node_t * item, int idx) {
    push_delta(n);
    if (n == 0) n = item;
    else if (item->priority < n->priority)
      split(n, item->L, item->R, idx), n = item;
    else if (idx <= count(n->L))
      insert(n->L, item, idx);
    else
      insert(n->R, item, idx - count(n->L) - 1);
    update(n);
  }

  static T get(node_t * n, int idx) {
    push_delta(n);
    if (idx < count(n->L))
      return get(n->L, idx);
    else if (idx > count(n->L))
      return get(n->R, idx - count(n->L) - 1);
    return n->value;
  }

  static void erase(node_t *& n, int idx) {
    push_delta(n);
    if (idx == count(n->L)) {
      delete n;
      merge(n, n->L, n->R);
    } else if (idx < count(n->L)) {
      erase(n->L, idx);
    } else {
      erase(n->R, idx - count(n->L) - 1);
    }
  }

  template<class UnaryFunction>
  void walk(node_t * n, UnaryFunction f) {
    if (n == 0) return;
    push_delta(n);
    if (n->L) walk(n->L, f);
    f(n->value);
    if (n->R) walk(n->R, f);
  }

  void clean_up(node_t *& n) {
    if (n == 0) return;
    clean_up(n->L);
    clean_up(n->R);
    delete n;
  }

 public:
  implicit_treap(): root(0) { srand(time(0)); }
  ~implicit_treap() { clean_up(root); }

  int size() const { return count(root); }
  bool empty() const { return root == 0; }

  //list.insert(list.begin() + idx, val)
  void insert(int idx, const T & val) {
    if (idx < 0 || idx > size()) return;
    node_t * item = new node_t(val);
    insert(root, item, idx);
  }

  void push_back(const T & val) {
    insert(size(), val);
  }

  //list.erase(list.begin() + idx)
  void erase(int idx) {
    if (idx < 0 || idx >= size()) return;
    erase(root, idx);
  }

  T at(int idx) {
    if (root == 0 || idx < 0 || idx >= size())
      return null_value();
    return get(root, idx);
  }

  template<class UnaryFunction> void walk(UnaryFunction f) {
    walk(root, f);
  }

  //for (i = a; i <= b; i++)
  //  list[i] = join_value_with_delta(list[i], delta)
  void modify(int a, int b, const T & delta) {
    if (a < 0 || b < 0 || a >= size() || b >= size() || a > b)
      return;
    node_t *l1, *r1;
    split(root, l1, r1, b + 1);
    node_t *l2, *r2;
    split(l1, l2, r2, a);
    apply_delta(r2, delta);
    node_t *t;
    merge(t, l2, r2);
    merge(root, t, r1);
  }

  //return join_values(list[a..b])
  T query(int a, int b) {
    if (a < 0 || b < 0 || a >= size() || b >= size() || a > b)
      return null_value();
    node_t *l1, *r1;
    split(root, l1, r1, b + 1);
    node_t *l2, *r2;
    split(l1, l2, r2, a);
    int res = subtree_value(r2);
    node_t *t;
    merge(t, l2, r2);
    merge(root, t, r1);
    return res;
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

void print(int x) { cout << x << " "; }

int main() {
  implicit_treap<int> T;
  T.push_back(7);
  T.push_back(8);
  T.push_back(9);
  T.insert(1, 5);
  T.erase(3);
  T.walk(print); cout << "\n";   //7 5 8
  T.modify(0, 2, 2);
  T.walk(print); cout << "\n";   //9 7 10
  cout << T.at(1) << "\n";       //7
  cout << T.query(0, 2) << "\n"; //10
  cout << T.size() << "\n";      //3
  return 0;
}
