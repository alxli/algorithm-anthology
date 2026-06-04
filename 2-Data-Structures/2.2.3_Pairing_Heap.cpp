/*

Maintain a mergeable min-priority queue, that is, a collection of elements with support for querying
and extraction of the minimum as well as efficient merging with other instances. This implementation
requires an ordering on the set of possible elements defined by `operator <`. A pairing heap is a
heap-ordered multi-way tree, using a two-pass merge to self-adjust during each deletion.

- `PairingHeap()` constructs an empty priority queue.
- `PairingHeap(lo, hi)` constructs a priority queue from two ForwardIterators, consisting of
  elements in the range `[lo, hi)`.
- `size()` returns the size of the priority queue.
- `empty()` returns whether the priority queue is empty.
- `push(v)` inserts the value `v` into the priority queue.
- `pop()` removes the minimum element from the priority queue.
- `top()` returns the minimum element in the priority queue.
- `absorb(h)` inserts every value from `h` and sets `h` to the empty priority queue.

Time Complexity:
- O(1) per call to the first constructor, `size()`, `empty()`, `top()`, `push()`, and `absorb()`.
- O(log n) amortized per call to `pop()`.
- O(n) per call to the second constructor on the distance between lo and hi.

Space Complexity:
- O(n) for storage of the priority queue elements.
- O(log n) amortized auxiliary stack space for `pop()`.
- O(1) auxiliary for all other operations.

*/

#include <cstddef>
#include <stdexcept>

template<class T>
class PairingHeap {
  struct Node {
    T value;
    Node *left, *next;

    explicit Node(const T &v) : value(v), left(nullptr), next(nullptr) {}

    void add_child(Node *n) {
      if (left == nullptr) {
        left = n;
      } else {
        n->next = left;
        left = n;
      }
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
      return a;
    }
    b->add_child(a);
    return b;
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

  template<class It>
  PairingHeap(It lo, It hi) : root(nullptr), num_nodes(0) {
    while (lo != hi) {
      push(*(lo++));
    }
  }

  ~PairingHeap() { clean_up(root); }
  int size() const { return num_nodes; }
  bool empty() const { return root == nullptr; }

  void push(const T &v) {
    root = merge(root, new Node(v));
    num_nodes++;
  }

  void pop() {
    if (empty()) {
      throw std::runtime_error("Cannot pop from empty heap.");
    }
    Node *tmp = root;
    root = merge_pairs(root->left);
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
    h.root = nullptr;
  }
};

/*** Example Usage and Output:

-1
0
5
10
12

***/

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
  while (!h.empty()) {
    cout << h.top() << endl;
    h.pop();
  }
  return 0;
}
