/*

Maintain a mergeable min-priority queue, that is, a collection of elements with support for querying
and extraction of the minimum as well as efficient merging with other instances. This implementation
requires an ordering on the set of possible elements defined by `operator<`. A pairing heap is a
heap-ordered multi-way tree, using a two-pass merge to self-adjust during each deletion. Unlike the
other heaps in this section, it also supports `decrease_key()` and `erase()` via handles returned
by `push()`, the feature pairing heaps are best known for.

- `PairingHeap<T>()` constructs an empty priority queue.
- `PairingHeap<T>(lo, hi)` constructs a priority queue from two ForwardIterators, consisting of
  elements in the range [`lo`, `hi`).
- `size()` returns the size of the priority queue.
- `empty()` returns whether the priority queue is empty.
- `push(v)` inserts the value `v` and returns a handle to the new element. The handle remains valid
  until that element is removed by `pop()` or `erase()`.
- `pop()` removes the minimum element from the priority queue.
- `top()` returns the minimum element in the priority queue.
- `absorb(h)` inserts every value from `h` and sets `h` to the empty priority queue.
- `decrease_key(h, v)` lowers the value of the element at handle `h` to `v`, which must not be
  greater than its current value.
- `erase(h)` removes the element at handle `h` from the priority queue.

In practice, `decrease_key()` runs in near-constant time, making the pairing heap the usual
practical substitute for a Fibonacci heap. When handles are not needed, the lazy-deletion idiom
(push updated entries and skip stale ones on `pop()`, as the Dijkstra section does) is simpler and
often faster.

Time Complexity:
- O(1) per call to the first constructor, `size()`, `empty()`, `top()`, `push()`, and `absorb()`.
- O(log n) amortized per call to `pop()` and `erase()`.
- O(log n) amortized per call to `decrease_key()`, a safe bound to budget for. The true amortized
  cost is sub-logarithmic, though the long-conjectured O(1) bound is provably impossible.
- O(n) per call to the second constructor on the distance between `lo` and `hi`.

Space Complexity:
- O(n) for storage of the priority queue elements.
- O(log n) amortized auxiliary stack space for `pop()`.
- O(1) auxiliary for all other operations.

*/

#include <cstddef>
#include <stdexcept>

template<typename T>
class PairingHeap {
  struct Node {
    T value;
    Node *left, *next, *prev;

    explicit Node(const T &v) : value(v), left(nullptr), next(nullptr), prev(nullptr) {}

    // `prev` points to the previous sibling, or to the parent when this is the first child, so any
    // node can be cut from its child list in O(1).
    void add_child(Node *n) {
      n->prev = this;
      n->next = left;
      if (left != nullptr) {
        left->prev = n;
      }
      left = n;
    }
  } *root;

  int num_nodes;

  static Node *merge(Node *a, Node *b) {
    if (a == nullptr) {
      return b;
    }
    if (b == nullptr) {
      return a;
    }
    if (a->value < b->value) {
      a->add_child(b);
      a->prev = a->next = nullptr;
      return a;
    }
    b->add_child(a);
    b->prev = b->next = nullptr;
    return b;
  }

  // Detaches non-root node `n` from its parent's child list.
  static void cut(Node *n) {
    if (n->prev->left == n) {
      n->prev->left = n->next;
    } else {
      n->prev->next = n->next;
    }
    if (n->next != nullptr) {
      n->next->prev = n->prev;
    }
    n->prev = n->next = nullptr;
  }

  static Node *merge_pairs(Node *n) {
    if (n == nullptr || n->next == nullptr) {
      return n;
    }
    Node *a = n, *b = n->next, *c = n->next->next;
    a->next = b->next = nullptr;
    return merge(merge(a, b), merge_pairs(c));
  }

  static void clean_up(Node *n) {
    if (n != nullptr) {
      clean_up(n->left);
      clean_up(n->next);
      delete n;
    }
  }

 public:
  PairingHeap() : root(nullptr), num_nodes(0) {}

  template<typename It>
  PairingHeap(It lo, It hi) : root(nullptr), num_nodes(0) {
    while (lo != hi) {
      push(*(lo++));
    }
  }

  ~PairingHeap() { clean_up(root); }
  PairingHeap(const PairingHeap &) = delete;
  PairingHeap &operator=(const PairingHeap &) = delete;
  int size() const { return num_nodes; }
  bool empty() const { return root == nullptr; }

  Node *push(const T &v) {
    Node *n = new Node(v);
    root = merge(root, n);
    num_nodes++;
    return n;
  }

  void pop() {
    if (empty()) {
      throw std::runtime_error("Cannot pop from empty heap.");
    }
    Node *tmp = root;
    root = merge_pairs(root->left);
    if (root != nullptr) {
      root->prev = root->next = nullptr;
    }
    delete tmp;
    num_nodes--;
  }

  T top() const {
    if (empty()) {
      throw std::runtime_error("Cannot get top of empty heap.");
    }
    return root->value;
  }

  void absorb(PairingHeap &h) {
    root = merge(root, h.root);
    num_nodes += h.num_nodes;
    h.root = nullptr;
    h.num_nodes = 0;
  }

  void decrease_key(Node *n, const T &v) {
    if (n->value < v) {
      throw std::runtime_error("decrease_key cannot increase the value.");
    }
    n->value = v;
    if (n != root) {
      cut(n);
      root = merge(root, n);
    }
  }

  void erase(Node *n) {
    if (n == root) {
      pop();
      return;
    }
    cut(n);
    root = merge(root, merge_pairs(n->left));
    delete n;
    num_nodes--;
  }
};

/*** Example Usage and Output:

-1
0
5
10
12

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  PairingHeap<int> h, h2;
  h.push(12);
  h.push(10);
  h2.push(5);
  h2.push(-1);
  h2.push(0);
  h.absorb(h2);

  auto handle = h.push(100);
  h.decrease_key(handle, -5);  // 100 -> -5, the new minimum.
  assert(h.top() == -5);
  h.erase(handle);  // Remove it again, restoring the original five elements.
  assert(h.size() == 5);

  while (!h.empty()) {
    cout << h.top() << endl;
    h.pop();
  }
  return 0;
}
