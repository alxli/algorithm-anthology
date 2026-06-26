/*

Maintain a min-priority queue, that is, a collection of elements with support for querying and
extraction of the minimum. This implementation requires an ordering on the set of possible elements
defined by `operator<`. A binary min-heap implements a priority queue by inserting and deleting
nodes into a binary tree such that the parent of any node is always less than its children. The tree
is stored implicitly in an array, where the children of the node at index $i$ live at indices
$2i + 1$ and $2i + 2$. Insertion appends the new element and bubbles it up toward the root, while
extraction moves the last element to the root and sifts it down.

- `BinaryHeap<T>()` constructs an empty priority queue.
- `BinaryHeap<T>(lo, hi)` constructs a priority queue from two ForwardIterators, consisting of
  elements in the range $[`lo`, `hi`)$.
- `size()` returns the size of the priority queue.
- `empty()` returns whether the priority queue is empty.
- `push(v)` inserts the value `v` into the priority queue.
- `pop()` removes the minimum element from the priority queue.
- `top()` returns the minimum element in the priority queue.

Time Complexity:
- O(1) per call to the first constructor, `size()`, `empty()`, and `top()`.
- O(log n) per call to `push()` and `pop()`, where $n$ is the number of elements in the priority
  queue.
- O(n) per call to the second constructor on the distance between `lo` and `hi` (bottom-up heapify).

Space Complexity:
- O(n) for storage of the priority queue elements.
- O(1) auxiliary for all operations.

*/

#include <cassert>
#include <utility>
#include <vector>

template<typename T>
class BinaryHeap {
  std::vector<T> heap;

  void sift_down(int i) {
    while (true) {
      int child = 2 * i + 1;
      if (child >= static_cast<int>(heap.size())) {
        break;
      }
      if (child + 1 < static_cast<int>(heap.size()) && heap[child + 1] < heap[child]) {
        child++;
      }
      if (heap[child] < heap[i]) {
        std::swap(heap[i], heap[child]);
        i = child;
      } else {
        break;
      }
    }
  }

 public:
  BinaryHeap() {}

  template<typename It>
  BinaryHeap(It lo, It hi) : heap(lo, hi) {
    for (int i = static_cast<int>(heap.size()) / 2 - 1; i >= 0; i--) {
      sift_down(i);
    }
  }

  int size() const { return static_cast<int>(heap.size()); }
  bool empty() const { return heap.empty(); }

  void push(const T &v) {
    heap.push_back(v);
    int i = static_cast<int>(heap.size()) - 1;
    while (i > 0) {
      int parent = (i - 1) / 2;
      if (!(heap[i] < heap[parent])) {
        break;
      }
      std::swap(heap[i], heap[parent]);
      i = parent;
    }
  }

  void pop() {
    assert(!heap.empty());
    heap[0] = heap.back();
    heap.pop_back();
    if (!heap.empty()) {
      sift_down(0);
    }
  }

  T top() const {
    assert(!heap.empty());
    return heap[0];
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
  vector<int> a{0, 5, -1, 12};
  BinaryHeap<int> h(a.begin(), a.end());
  assert(h.size() == 4);
  assert(h.top() == -1);
  h.push(10);
  vector<int> popped;
  while (!h.empty()) {
    cout << h.top() << endl;
    popped.push_back(h.top());
    h.pop();
  }
  assert((popped == vector<int>{-1, 0, 5, 10, 12}));
  assert(h.empty());
  return 0;
}
