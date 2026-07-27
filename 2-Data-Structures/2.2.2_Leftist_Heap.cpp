/*

Maintains a mergeable priority queue, returning the minimum element by default. The comparator
`comp` defines priority: `comp(a, b)` is true when `a` has higher priority than `b`, so
`std::greater<T>` instead creates a max-priority queue. A leftist heap stores at each node a rank
(the length of the shortest path to a missing descendant) and keeps every left child's rank at least
that of its right child, so the right spine has length O(log n). Merging walks only this right
spine, swapping children where needed to restore the invariant. Every other operation reduces to
this merge: insertion merges a one-node heap, and extraction merges the root's two subtrees. Unlike
the self-adjusting skew heap, the explicit rank bound makes every operation O(log n) in the worst
case rather than amortized. To customize the ordering, instantiate `LeftistHeap<T, Compare>` and
pass the comparator to either constructor.

- `LeftistHeap<T>()` constructs an empty priority queue.
- `LeftistHeap<T>(lo, hi)` constructs a priority queue from the elements in the half-open iterator
  range $[`lo`, `hi`)$.
- `size()` returns the size of the priority queue.
- `empty()` returns whether the priority queue is empty.
- `push(v)` inserts the value `v` into the priority queue.
- `pop()` removes the highest-priority element from the priority queue.
- `top()` returns the highest-priority element in the priority queue.
- `absorb(h)` inserts every value from the distinct heap `h` and sets `h` to the empty priority
  queue. Both heaps must use equivalent comparators.

Time Complexity:
- O(1) per call to the first constructor, `size()`, `empty()`, and `top()`.
- O(log n) per call to `push()`, `pop()`, and `absorb()`, where $n$ is the number of elements in the
  priority queue.
- O(n log n) per call to the second constructor, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) for storage of the priority queue elements.
- O(log n) auxiliary stack space for `push()`, `pop()`, and `absorb()`.
- O(n) auxiliary stack space in the worst case for destruction.
- O(1) auxiliary for all other operations.

*/

#include <cassert>
#include <functional>
#include <utility>

template<typename T, typename Compare = std::less<T>>
class LeftistHeap {
  struct Node {
    T value;
    int rank;
    Node *left, *right;

    explicit Node(const T &v) : value(v), rank(1), left(nullptr), right(nullptr) {}
  } *root;

  int num_nodes;
  Compare comp;

  static int rank(Node *n) { return (n == nullptr) ? 0 : n->rank; }

  Node *merge(Node *a, Node *b) {
    if (a == nullptr) {
      return b;
    }
    if (b == nullptr) {
      return a;
    }
    if (comp(b->value, a->value)) {
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
  explicit LeftistHeap(Compare comp = Compare())
      : root(nullptr), num_nodes(0), comp(std::move(comp)) {}

  template<typename It>
  LeftistHeap(It lo, It hi, Compare comp = Compare())
      : root(nullptr), num_nodes(0), comp(std::move(comp)) {
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
    assert(!empty());
    Node *tmp = root;
    root = merge(root->left, root->right);
    delete tmp;
    num_nodes--;
  }

  const T &top() const {
    assert(!empty());
    return root->value;
  }

  void absorb(LeftistHeap &h) {
    assert(this != &h);
    root = merge(root, h.root);
    num_nodes += h.num_nodes;
    h.root = nullptr;
    h.num_nodes = 0;
  }
};

/*** Example Usage ***/

#include <vector>
using namespace std;

int main() {
  LeftistHeap<int> h, h2;
  assert(h.empty());
  h.push(12);
  h.push(10);
  h2.push(5);
  h2.push(-1);
  h2.push(0);
  h.absorb(h2);
  assert(h.size() == 5);
  assert(h2.empty());
  vector<int> popped;
  while (!h.empty()) {
    popped.push_back(h.top());
    h.pop();
  }
  assert((popped == vector<int>{-1, 0, 5, 10, 12}));

  LeftistHeap<int, greater<int>> max_heap;
  max_heap.push(1);
  max_heap.push(3);
  assert(max_heap.top() == 3);
  return 0;
}
