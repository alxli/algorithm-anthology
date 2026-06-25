/*

Maintain a first-in, first-out queue that additionally answers, in O(1) amortized time, the fold of
an associative operation over all elements currently in the queue. This is the "sliding window
aggregation" (SWAG) technique: by pushing new elements and popping old ones as a window advances, it
reports an aggregate of every window in linear total time.

Unlike the monotonic deque used for sliding-window extrema (see 1.2.3), the operation need only
be associative, not order-based, so the same structure handles min, max, sum, gcd, matrix products,
and other monoids. It is sometimes called a "monotonic queue," but that term usually denotes the
monotonic deque of 1.2.3 (which handles only min/max); the two are different structures. The queue
is built from two stacks: a back stack receives pushes while a front stack serves pops, and each
stack stores running folds so the two halves combine in O(1). Whenever the front stack empties, the
back stack is flipped onto it; this costs O(1) amortized per element, since each element is moved
between stacks at most once.

The fold is defined by an associative `combine(a, b)` function. The default code below returns the
"min" of the elements; for "sum", `combine(a, b)` should return `a + b`. The fold is taken in queue
order (front to back), so non-commutative operations such as matrix products are also supported.

- `AggregateQueue<T>()` constructs an empty queue.
- `empty()` returns whether the queue is empty.
- `size()` returns the number of elements in the queue.
- `push(x)` appends `x` to the back of the queue.
- `pop()` removes the element at the front of the queue, which must be non-empty.
- `front()` returns the element at the front of the queue, which must be non-empty.
- `fold()` returns `combine()` applied to all elements in queue order; the queue must be non-empty.

Time Complexity:
- O(1) per call to the constructor, `empty()`, `size()`, `front()`, and `fold()`.
- O(1) amortized per call to `push()` and `pop()`.

Space Complexity:
- O(n) for storage of the queue elements, where $n$ is the number of elements.
- O(1) auxiliary per operation, amortized (a `pop()` may flip the back stack onto the front stack).

*/

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

template<typename T>
class AggregateQueue {
  static T combine(const T &a, const T &b) { return std::min(a, b); }

  // Each stack entry is (value, fold), where fold combines the value with all entries between it
  // and its end of the queue (the front for front_stack, the back for back_stack).
  std::vector<std::pair<T, T>> front_stack, back_stack;

 public:
  bool empty() const { return front_stack.empty() && back_stack.empty(); }
  int size() const { return static_cast<int>(front_stack.size() + back_stack.size()); }

  void push(const T &x) {
    T acc = back_stack.empty() ? x : combine(back_stack.back().second, x);
    back_stack.emplace_back(x, acc);
  }

  void pop() {
    assert(!empty());
    if (front_stack.empty()) {
      // Flip the back stack onto the front stack, reversing order so the oldest element ends up on
      // top, and recompute the front folds in queue order.
      while (!back_stack.empty()) {
        T x = back_stack.back().first;
        back_stack.pop_back();
        T acc = front_stack.empty() ? x : combine(x, front_stack.back().second);
        front_stack.emplace_back(x, acc);
      }
    }
    front_stack.pop_back();
  }

  T front() const {
    assert(!empty());
    return front_stack.empty() ? back_stack.front().first : front_stack.back().first;
  }

  T fold() const {
    assert(!empty());
    if (front_stack.empty()) {
      return back_stack.back().second;
    }
    if (back_stack.empty()) {
      return front_stack.back().second;
    }
    return combine(front_stack.back().second, back_stack.back().second);
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  // Used directly as a FIFO queue with an O(1) min query.
  AggregateQueue<int> q;
  assert(q.empty());
  q.push(5);
  q.push(3);
  q.push(8);
  assert(!q.empty());
  assert(q.front() == 5 && q.size() == 3);
  assert(q.fold() == 3);  // min(5, 3, 8)
  q.pop();                // Removes 5.
  assert(q.fold() == 3);  // min(3, 8)
  q.pop();                // Removes 3.
  assert(q.fold() == 8);
  q.pop();  // Removes 8.
  assert(q.empty());

  // Canonical use: the minimum of every width-3 window, advancing the window by push/pop.
  vector<int> a{4, 2, 5, 1, 3, 6};
  AggregateQueue<int> w;
  vector<int> mins;
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    w.push(a[i]);
    if (w.size() > 3) {
      w.pop();
    }
    if (w.size() == 3) {
      mins.push_back(w.fold());
    }
  }
  assert((mins == vector<int>{2, 1, 1, 1}));  // min of [4,2,5], [2,5,1], [5,1,3], [1,3,6]
  return 0;
}
