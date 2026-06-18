/*

Maintain a mergeable min-priority queue, that is, a collection of elements with support for querying
and extraction of the minimum as well as efficient merging with other instances. This implementation
requires an ordering on the set of possible elements defined by `operator<`. A skew heap attempts to
maintain balance by unconditionally swapping all nodes in the merge path when merging. Every other
operation reduces to this merge: insertion merges a one-node heap, and extraction of the minimum
merges the root's two subtrees.

- `SkewHeap<T>()` constructs an empty priority queue.
- `SkewHeap<T>(lo, hi)` constructs a priority queue from two ForwardIterators, consisting of
  elements in the range [`lo`, `hi`).
- `size()` returns the size of the priority queue.
- `empty()` returns whether the priority queue is empty.
- `push(v)` inserts the value `v` into the priority queue.
- `pop()` removes the minimum element from the priority queue.
- `top()` returns the minimum element in the priority queue.
- `join(h)` inserts every value from `h` and sets `h` to the empty priority queue.

Time Complexity:
- O(1) per call to the first constructor, `size()`, `empty()`, and `top()`.
- O(log n) amortized auxiliary per call to `push()`, `pop()`, and `join()`, where $n$ is the
  number of elements in the priority queue.
- O(n) per call to the second constructor, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) for storage of the priority queue elements.
- O(log n) amortized auxiliary stack space for `push()`, `pop()`, and `join()`.
- O(1) auxiliary for all other operations.

*/

#include <algorithm>
#include <cstddef>
#include <stdexcept>

template<typename T>
class SkewHeap {
  struct Node {
    T value;
    Node *left, *right;

    explicit Node(const T &v) : value(v), left(nullptr), right(nullptr) {}
  } *root;

  int num_nodes;

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
    std::swap(a->left, a->right);
    a->left = merge(b, a->left);
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
  SkewHeap() : root(nullptr), num_nodes(0) {}

  template<typename It>
  SkewHeap(It lo, It hi) : root(nullptr), num_nodes(0) {
    while (lo != hi) {
      push(*(lo++));
    }
  }

  ~SkewHeap() { clean_up(root); }
  SkewHeap(const SkewHeap &) = delete;
  SkewHeap &operator=(const SkewHeap &) = delete;
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

  void join(SkewHeap &h) {
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
  SkewHeap<int> h, h2;
  h.push(12);
  h.push(10);
  h2.push(5);
  h2.push(-1);
  h2.push(0);
  h.join(h2);
  while (!h.empty()) {
    cout << h.top() << endl;
    h.pop();
  }
  return 0;
}
