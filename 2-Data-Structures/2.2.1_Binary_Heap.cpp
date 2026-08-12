/*

Maintains a priority queue, returning the minimum element by default. The comparator `comp` defines
priority: `comp(a, b)` is true when `a` has higher priority than `b`, so `std::greater<T>` instead
creates a max-priority queue. A binary heap stores a heap-ordered binary tree implicitly in an
array, where the children of the node at index $i$ live at indices $2i + 1$ and $2i + 2$. Insertion
appends the new element and bubbles it up toward the root, while extraction moves the last element
to the root and sifts it down. To customize the ordering, instantiate `BinaryHeap<T, Compare>` and
pass the comparator to either constructor.

- `BinaryHeap<T>()` constructs an empty priority queue.
- `BinaryHeap<T>(lo, hi)` constructs a priority queue from the elements in the half-open iterator
  range $[`lo`, `hi`)$.
- `size()` returns the size of the priority queue.
- `empty()` returns whether the priority queue is empty.
- `push(v)` inserts the value `v` into the priority queue.
- `pop()` removes the highest-priority element from the priority queue.
- `top()` returns the highest-priority element in the priority queue.

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
#include <functional>
#include <utility>
#include <vector>

template<typename T, typename Compare = std::less<T>>
class BinaryHeap {
  std::vector<T> heap;
  Compare comp;

  void sift_down(int i) {
    while (true) {
      int child = i * 2 + 1;
      if (child >= static_cast<int>(heap.size())) {
        break;
      }
      if (child + 1 < static_cast<int>(heap.size()) && comp(heap[child + 1], heap[child])) {
        child++;
      }
      if (comp(heap[child], heap[i])) {
        std::swap(heap[i], heap[child]);
        i = child;
      } else {
        break;
      }
    }
  }

 public:
  explicit BinaryHeap(Compare comp = Compare{}) : comp(std::move(comp)) {}

  template<typename It>
  BinaryHeap(It lo, It hi, Compare comp = Compare{}) : heap(lo, hi), comp(std::move(comp)) {
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
      if (!comp(heap[i], heap[parent])) {
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

  const T &top() const {
    assert(!heap.empty());
    return heap[0];
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  vector<int> a{0, 5, -1, 12};
  BinaryHeap<int> h(a.begin(), a.end());
  assert(h.size() == 4);
  assert(h.top() == -1);
  h.push(10);
  vector<int> popped;
  while (!h.empty()) {
    popped.push_back(h.top());
    h.pop();
  }
  assert((popped == vector<int>{-1, 0, 5, 10, 12}));
  assert(h.empty());

  BinaryHeap<int, greater<int>> max_heap;
  max_heap.push(1);
  max_heap.push(3);
  assert(max_heap.top() == 3);
  return 0;
}
