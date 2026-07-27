/*

Maintains a mergeable priority queue, returning the minimum element by default. The comparator
`comp` defines priority: `comp(a, b)` is true when `a` has higher priority than `b`, so
`std::greater<T>` instead creates a max-priority queue. A pairing heap is a heap-ordered multi-way
tree, using a two-pass merge to self-adjust during each deletion. Unlike the other heaps in this
section, it also supports `improve_key()` (the usual decrease-key operation for a min-heap) and
`erase()` via handles returned by `push()`, the feature pairing heaps are best known for.

To customize the ordering, instantiate `PairingHeap<T, Compare>` and pass the comparator to either
constructor.

- `PairingHeap<T>()` constructs an empty priority queue.
- `PairingHeap<T>(lo, hi)` constructs a priority queue from the elements in the half-open iterator
  range $[`lo`, `hi`)$.
- `size()` returns the size of the priority queue.
- `empty()` returns whether the priority queue is empty.
- `push(v)` inserts the value `v` and returns a handle to the new element. The handle remains valid
  until that element is removed by `pop()` or `erase()`.
- `pop()` removes the highest-priority element from the priority queue.
- `top()` returns the highest-priority element in the priority queue.
- `absorb(h)` inserts every value from the distinct heap `h` and sets `h` to the empty priority
  queue. Both heaps must use equivalent comparators.
- `improve_key(n, v)` replaces the value at handle `n` with a value `v` of equal or higher priority.
- `erase(n)` removes the element at handle `n` from the priority queue.

In practice, `improve_key()` runs in near-constant time, making the pairing heap the usual practical
substitute for a Fibonacci heap. When handles are not needed, the lazy-deletion idiom (push updated
entries and skip stale ones on `pop()`, as the Dijkstra section does) is simpler and often faster.
On GNU C++ judges, `__gnu_pbds::priority_queue` with `pairing_heap_tag` provides a shorter handled
and meldable heap; see 8.6 for the contest wrapper.

Time Complexity:
- O(1) per call to the first constructor, `size()`, `empty()`, `push()`, `top()`, and `absorb()`.
- O(log n) amortized per call to `pop()` and `erase()`.
- O(log n) amortized per call to `improve_key()`, a safe bound to budget for. The true amortized
  cost is sub-logarithmic, though the long-conjectured O(1) bound is provably impossible.
- O(n) per call to the second constructor on the distance between `lo` and `hi`.

Space Complexity:
- O(n) for storage of the priority queue elements.
- O(n) auxiliary stack space in the worst case for `pop()`, `erase()`, and destruction.
- O(1) auxiliary for all other operations.

*/

#include <cassert>
#include <functional>
#include <utility>

template<typename T, typename Compare = std::less<T>>
class PairingHeap {
  struct Node {
    T value;
    Node *left, *next, *prev;

    explicit Node(const T &v) : value(v), left(nullptr), next(nullptr), prev(nullptr) {}

    // prev points to the previous sibling, or to the parent when this is the first child, so any
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
  Compare comp;

  Node *merge(Node *a, Node *b) {
    if (a == nullptr) {
      return b;
    }
    if (b == nullptr) {
      return a;
    }
    if (comp(a->value, b->value)) {
      a->add_child(b);
      a->prev = a->next = nullptr;
      return a;
    }
    b->add_child(a);
    b->prev = b->next = nullptr;
    return b;
  }

  // Detaches non-root node n from its parent's child list.
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

  Node *merge_pairs(Node *n) {
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
  explicit PairingHeap(Compare comp = Compare())
      : root(nullptr), num_nodes(0), comp(std::move(comp)) {}

  template<typename It>
  PairingHeap(It lo, It hi, Compare comp = Compare())
      : root(nullptr), num_nodes(0), comp(std::move(comp)) {
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
    assert(!empty());
    Node *tmp = root;
    root = merge_pairs(root->left);
    if (root != nullptr) {
      root->prev = root->next = nullptr;
    }
    delete tmp;
    num_nodes--;
  }

  const T &top() const {
    assert(!empty());
    return root->value;
  }

  void absorb(PairingHeap &h) {
    assert(this != &h);
    root = merge(root, h.root);
    num_nodes += h.num_nodes;
    h.root = nullptr;
    h.num_nodes = 0;
  }

  void improve_key(Node *n, const T &v) {
    assert(!comp(n->value, v));
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

/*** Example Usage ***/

#include <vector>
using namespace std;

int main() {
  PairingHeap<int> h, h2;
  assert(h.empty());
  h.push(12);
  h.push(10);
  h2.push(5);
  h2.push(-1);
  h2.push(0);
  h.absorb(h2);
  assert(h.size() == 5);
  assert(h2.empty());

  auto handle = h.push(100);
  h.improve_key(handle, -5);  // 100 -> -5, the new minimum.
  assert(h.top() == -5);
  h.erase(handle);  // Remove it again, restoring the original five elements.
  assert(h.size() == 5);

  vector<int> popped;
  while (!h.empty()) {
    popped.push_back(h.top());
    h.pop();
  }
  assert((popped == vector<int>{-1, 0, 5, 10, 12}));

  PairingHeap<int, greater<int>> max_heap;
  auto max_handle = max_heap.push(1);
  max_heap.improve_key(max_handle, 3);
  assert(max_heap.top() == 3);
  return 0;
}
