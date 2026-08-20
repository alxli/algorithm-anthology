/*

The median of a stream can be maintained as values arrive, without ever sorting the stream. This
section keeps the median of a growing stream in two heaps, and the median of a changing collection
in a multiset, the difference between them being whether values ever have to leave again. For the
mean and variance of a stream, see section 1.6.4.

Two heaps hold the values in halves, using linear storage. A max-heap stores the lower half and a
min-heap stores the upper half, balanced so that the lower heap has either the same number of values
as the upper heap or one extra value. Their roots are therefore the middle value or values of the
stream.

- `OnlineMedian<T>()` constructs an empty median tracker for an ordered numeric type `T`.
- `empty()` returns whether the tracker has no values.
- `count()` returns the number of values in the tracker.
- `add(x)` inserts `x` into the tracker.
- `lower_median()` and `upper_median()` return the lower and the upper of the two middle values, or
  the middle value itself for an odd count. The tracker must be nonempty.
- `median()` returns the middle value for an odd count or the arithmetic mean of the two middle
  values for an even count. The tracker must be nonempty.

Since `median()` averages in `double`, an even count over a type whose values exceed the exactly
representable range of `double` should be averaged by the caller from `lower_median()` and
`upper_median()` instead.

Time Complexity:
- O(log n) per call to `add()` and O(1) per query, where $n$ is the number of values.

Space Complexity:
- O(n) storage.

*/

#include <cassert>
#include <functional>
#include <iterator>
#include <queue>
#include <set>
#include <vector>

template<typename T>
class OnlineMedian {
  std::priority_queue<T> lower;
  std::priority_queue<T, std::vector<T>, std::greater<T>> upper;

 public:
  bool empty() const { return lower.empty(); }
  int count() const { return static_cast<int>(lower.size() + upper.size()); }

  void add(const T &x) {
    if (lower.empty() || x <= lower.top()) {
      lower.push(x);
    } else {
      upper.push(x);
    }
    if (lower.size() < upper.size()) {
      lower.push(upper.top());
      upper.pop();
    } else if (lower.size() > upper.size() + 1) {
      upper.push(lower.top());
      lower.pop();
    }
  }

  const T &lower_median() const {
    assert(!empty());
    return lower.top();
  }

  const T &upper_median() const {
    assert(!empty());
    return lower.size() != upper.size() ? lower.top() : upper.top();
  }

  double median() const {
    return static_cast<double>(lower_median()) / 2 + static_cast<double>(upper_median()) / 2;
  }
};

/*

To additionally support arbitrary removals, a multiset (balanced binary search tree) holds the
values in sorted order while an iterator tracks the lower median. Each insertion or erasure shifts
the middle of the multiset by at most one position, so the iterator only ever steps one element in
either direction: whether it moves depends on which side of the median the value lies and on the
parity of the resulting size. Unlike the two heaps above, and unlike the statistics queue of section
1.6.4 that can only drop its oldest value, a multiset erases any value it holds, so removals may
come in any order.

- `DeletableMedian<T>()` constructs an empty median tracker for an ordered numeric type `T`.
- `add(x)` inserts `x` into the median tracker.
- `remove(x)` erases one copy of `x`, which must currently be present.
- `count()` and `empty()` return the number of values and whether the tracker is empty.
- `lower_median()`, `upper_median()`, and `median()` answer as above, over the values currently
  held. The tracker must be nonempty.

A trailing window of width $w$ over an array is one call to `add()` followed by a call to `remove()`
on the value leaving the window, and a two-pointer scan from section 1.2.3 removes by its own
condition instead. Because removals are unordered, the same structure also serves the add and remove
operations of Mo's algorithm in section 2.5.4, which answers median queries over arbitrary ranges
offline. Replacing the parked iterator with an order-statistic tree, such as the `__gnu_pbds` tree
of section 8.6, generalizes further to any rank rather than just the middle.

Time Complexity:
- O(log n) per call to `add()` and `remove()`, and O(1) per query, where $n$ is the number of values
  currently held.

Space Complexity:
- O(n) storage.

*/

template<typename T>
class DeletableMedian {
  std::multiset<T> sorted;
  typename std::multiset<T>::iterator mid;

 public:
  DeletableMedian() = default;
  DeletableMedian(const DeletableMedian &) = delete;
  DeletableMedian &operator=(const DeletableMedian &) = delete;

  bool empty() const { return sorted.empty(); }
  int count() const { return static_cast<int>(sorted.size()); }

  void add(const T &x) {
    if (sorted.empty()) {
      mid = sorted.insert(x);
      return;
    }
    bool below = x < *mid;
    sorted.insert(x);
    if (below) {
      if (sorted.size() % 2 == 0) {
        --mid;
      }
    } else if (sorted.size() % 2 == 1) {
      ++mid;
    }
  }

  void remove(const T &x) {
    assert(!empty());
    if (sorted.size() == 1) {
      assert(x == *mid);
      sorted.clear();  // The iterator is meaningless while empty, and add() resets it.
      return;
    }
    // Erasing the median's own node requires stepping off it first. Any other node holding x
    // lies strictly on one side of the median, so erasing it can never invalidate the iterator.
    auto target = (x == *mid) ? mid : sorted.find(x);
    assert(target != sorted.end());
    if (x <= *mid && sorted.size() % 2 == 0) {
      ++mid;
    } else if (x >= *mid && sorted.size() % 2 == 1) {
      --mid;
    }
    sorted.erase(target);
  }

  const T &lower_median() const {
    assert(!empty());
    return *mid;
  }

  const T &upper_median() const {
    assert(!empty());
    return sorted.size() % 2 == 1 ? *mid : *std::next(mid);
  }

  double median() const {
    return static_cast<double>(lower_median()) / 2 + static_cast<double>(upper_median()) / 2;
  }
};

/*** Example Usage ***/

#include <cmath>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  OnlineMedian<int> medians;
  assert(medians.empty() && medians.count() == 0);
  medians.add(5);
  assert(EQ(medians.median(), 5));
  medians.add(1);
  assert(medians.lower_median() == 1 && medians.upper_median() == 5);
  assert(EQ(medians.median(), 3));
  medians.add(9);
  assert(medians.lower_median() == 5 && medians.upper_median() == 5);
  medians.add(3);
  assert(medians.lower_median() == 3 && medians.upper_median() == 5);
  assert(EQ(medians.median(), 4));

  // A trailing window of width 3 over an array, maintained by the caller.
  vector<int> a{5, 1, 9, 3, 3};
  DeletableMedian<int> window;
  assert(window.empty() && window.count() == 0);
  for (int hi = 0, lo = 0; hi < static_cast<int>(a.size()); hi++) {
    window.add(a[hi]);
    if (window.count() > 3) {
      window.remove(a[lo++]);
    }
  }
  assert(window.count() == 3);  // The window holds 9, 3, and 3.
  assert(window.lower_median() == 3 && EQ(window.median(), 3));

  // Removals need not follow arrival order, and may target the median's own value.
  DeletableMedian<int> tracker;
  for (int x : {1, 2, 3, 4, 5}) {
    tracker.add(x);
  }
  assert(EQ(tracker.median(), 3));
  tracker.remove(3);  // Erasing the median itself steps the iterator off its own node first.
  assert(tracker.lower_median() == 2 && tracker.upper_median() == 4);
  tracker.remove(5);
  assert(EQ(tracker.median(), 2));

  // Duplicates of the median are erased one copy at a time.
  DeletableMedian<int> repeats;
  for (int x : {7, 7, 7}) {
    repeats.add(x);
  }
  repeats.remove(7);
  assert(repeats.count() == 2 && EQ(repeats.median(), 7));
  repeats.remove(7);
  assert(repeats.count() == 1 && repeats.lower_median() == 7);
  return 0;
}
