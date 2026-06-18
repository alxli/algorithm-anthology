/*

Maintain a mergeable min-priority queue, that is, a collection of elements with support for querying
and extraction of the minimum as well as efficient merging with other instances. This implementation
requires an ordering on the set of possible elements defined by `operator<`. A leftist heap stores
at each node a rank (the length of the shortest path to a missing descendant) and keeps every left
child's rank at least that of its right child, so the right spine has length O(log n). Merging walks
only this right spine, swapping children where needed to restore the invariant. Every other
operation reduces to this merge: insertion merges a one-node heap, and extraction of the minimum
merges the root's two subtrees. Unlike the self-adjusting skew heap, the explicit rank bound makes
every operation O(log n) in the worst case rather than amortized.

- `LeftistHeap<T>()` constructs an empty priority queue.
- `LeftistHeap<T>(lo, hi)` constructs a priority queue from two ForwardIterators, consisting of
  elements in the range [`lo`, `hi`).
- `size()` returns the size of the priority queue.
- `empty()` returns whether the priority queue is empty.
- `push(v)` inserts the value `v` into the priority queue.
- `pop()` removes the minimum element from the priority queue.
- `top()` returns the minimum element in the priority queue.
- `absorb(h)` inserts every value from `h` and sets `h` to the empty priority queue.

Time Complexity:
- O(1) per call to the first constructor, `size()`, `empty()`, and `top()`.
- O(log n) per call to `push()`, `pop()`, and `absorb()`, where $n$ is the number of elements in the
  priority queue.
- O(n) per call to the second constructor, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) for storage of the priority queue elements.
- O(log n) auxiliary stack space for `push()`, `pop()`, and `absorb()`.
- O(1) auxiliary for all other operations.

*/

#include <algorithm>
#include <cstddef>
#include <stdexcept>

template<typename T>
class LeftistHeap {
  struct Node {
    T value;
    int rank;
    Node *left, *right;

    explicit Node(const T &v) : value(v), rank(1), left(nullptr), right(nullptr) {}
  } *root;

  int num_nodes;

  static int rank(Node *n) { return (n == nullptr) ? 0 : n->rank; }

  static Node *merge(Node *a, Node *b) {
    if (a == nullptr) {
      return b;
    }
    if (b == nullptr) {
      return a;
    }
    if (b->value < a->value) {
      std::swap(a, b);
    }
    a->right = merge(a->right, b);
    if (rank(a->left) < rank(a->right)) {
      std::swap(a->left, a->right);
    }
    a->rank = rank(a->right) + 1;
    return a;
  }

  static void clean_up(Node *n) {
    if (n != nullptr) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  LeftistHeap() : root(nullptr), num_nodes(0) {}

  template<typename It>
  LeftistHeap(It lo, It hi) : root(nullptr), num_nodes(0) {
    while (lo != hi) {
      push(*(lo++));
    }
  }

  ~LeftistHeap() { clean_up(root); }
  LeftistHeap(const LeftistHeap &) = delete;
  LeftistHeap &operator=(const LeftistHeap &) = delete;
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
    root = merge(root->left, root->right);
    delete tmp;
    num_nodes--;
  }

  T top() const {
    if (empty()) {
      throw std::runtime_error("Cannot get top of empty heap.");
    }
    return root->value;
  }

  void absorb(LeftistHeap &h) {
    root = merge(root, h.root);
    num_nodes += h.num_nodes;
    h.root = nullptr;
    h.num_nodes = 0;
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
  LeftistHeap<int> h, h2;
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
