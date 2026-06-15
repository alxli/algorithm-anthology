/*

Maintain an ordered map from closed, one-dimensional intervals to values while supporting efficient
reporting of any or all entries that intersect with a given query interval. This implementation uses
`std::pair` to represent intervals, requiring operators `<` and `==` to be defined on the numeric
key type. A treap is used to process the entries, where keys are compared lexicographically as
pairs. Each node is additionally augmented with the maximum upper endpoint among the intervals in
its subtree, letting intersection queries skip any subtree whose maximum falls below the query
interval's lower bound.

- `IntervalTreap()` constructs an empty map.
- `size()` returns the size of the map.
- `empty()` returns whether the map is empty.
- `insert(lo, hi, v)` adds an entry with key `[lo, hi]` and value `v` to the map, returning `true`
  if a new interval was added or `false` if the interval already exists (in which case the map is
  unchanged and the old value associated with the key is preserved).
- `erase(lo, hi)` removes the entry with key `[lo, hi]` from the map, returning `true` if the
  removal was successful or `false` if the interval was not found.
- `find_key(lo, hi)` returns a pointer to a const `std::pair` representing the key of some interval
  in the map which intersects with `[lo, hi]`, or `nullptr` if no such entry was found.
- `find_value(lo, hi)` returns a pointer to a const value of some entry in the map with a key that
  intersects with `[lo, hi]`, or `nullptr` if no such entry was found.
- `find_all(lo, hi, f)` calls the function `f(lo, hi, v)` on each entry in the map that overlaps
  with `[lo, hi]`, in lexicographically ascending order of intervals.
- `entries()` returns all intervals and their values in lexicographically ascending order of
  intervals.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `empty()`.
- O(log n) on average per call to `insert()`, `erase()`, and `find_any()`, where $n$ is the number
  of intervals currently in the set.
- O(log n + m) on average per call to `find_all()`, where $m$ is the number of intersecting
  intervals that are reported.
- O(n) per call to `entries()`.

Space Complexity:
- O(n) for storage of the map elements.
- O(1) auxiliary for `size()` and `empty()`.
- O(log n) auxiliary stack space on average for all other operations.

*/

#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

template<class K, class V>
class IntervalTreap {
  using Interval = std::pair<K, K>;

  struct Node {
    static uint32_t rand32() {
      static uint32_t x = 123456789;
      x ^= x << 13;
      x ^= x >> 17;
      x ^= x << 5;
      return x;
    }

    Interval interval;
    V value;
    K max;
    uint32_t priority;
    Node *left, *right;

    Node(const Interval &i, const V &v)
        : interval(i), value(v), max(i.second), priority(rand32()), left(nullptr), right(nullptr) {}

    void update() {
      max = interval.second;
      if (left != nullptr && left->max > max) {
        max = left->max;
      }
      if (right != nullptr && right->max > max) {
        max = right->max;
      }
    }
  } *root;

  int num_nodes;

  static void rotate_left(Node *&n) {
    Node *tmp = n;
    n = n->right;
    tmp->right = n->left;
    n->left = tmp;
    tmp->update();
  }

  static void rotate_right(Node *&n) {
    Node *tmp = n;
    n = n->left;
    tmp->left = n->right;
    n->right = tmp;
    tmp->update();
  }

  static bool insert(Node *&n, const Interval &i, const V &v) {
    if (n == nullptr) {
      n = new Node(i, v);
      return true;
    }
    if (i < n->interval && insert(n->left, i, v)) {
      if (n->left->priority < n->priority) {
        rotate_right(n);
      }
      n->update();
      return true;
    }
    if (i > n->interval && insert(n->right, i, v)) {
      if (n->right->priority < n->priority) {
        rotate_left(n);
      }
      n->update();
      return true;
    }
    return false;
  }

  static bool erase(Node *&n, const Interval &i) {
    if (n == nullptr) {
      return false;
    }
    if (i < n->interval) {
      return erase(n->left, i);
    }
    if (i > n->interval) {
      return erase(n->right, i);
    }
    if (n->left != nullptr && n->right != nullptr) {
      bool res;
      if (n->left->priority < n->right->priority) {
        rotate_right(n);
        res = erase(n->right, i);
      } else {
        rotate_left(n);
        res = erase(n->left, i);
      }
      n->update();
      return res;
    }
    Node *tmp = (n->left != nullptr) ? n->left : n->right;
    delete n;
    n = tmp;
    return true;
  }

  static Node *find_any(Node *n, const Interval &i) {
    if (n == nullptr) {
      return nullptr;
    }
    if (n->interval.first <= i.second && i.first <= n->interval.second) {
      return n;
    }
    if (n->left != nullptr && i.first <= n->left->max) {
      return find_any(n->left, i);
    }
    return find_any(n->right, i);
  }

  template<class Fn>
  static void find_all(Node *n, const Interval &i, Fn f) {
    if (n == nullptr || n->max < i.first) {
      return;
    }
    find_all(n->left, i, f);
    if (n->interval.first <= i.second && i.first <= n->interval.second) {
      f(n->interval.first, n->interval.second, n->value);
    }
    if (n->interval.first <= i.second) {
      find_all(n->right, i, f);
    }
  }

  static void collect_entries(Node *n, std::vector<std::tuple<K, K, V>> &res) {
    if (n != nullptr) {
      collect_entries(n->left, res);
      res.push_back({n->interval.first, n->interval.second, n->value});
      collect_entries(n->right, res);
    }
  }

  static void clean_up(Node *n) {
    if (n != nullptr) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  IntervalTreap() : root(nullptr), num_nodes(0) {}

  ~IntervalTreap() { clean_up(root); }
  IntervalTreap(const IntervalTreap &) = delete;
  IntervalTreap &operator=(const IntervalTreap &) = delete;
  int size() const { return num_nodes; }
  bool empty() const { return root == nullptr; }

  bool insert(const K &lo, const K &hi, const V &v) {
    if (insert(root, Interval{lo, hi}, v)) {
      num_nodes++;
      return true;
    }
    return false;
  }

  bool erase(const K &lo, const K &hi) {
    if (erase(root, Interval{lo, hi})) {
      num_nodes--;
      return true;
    }
    return false;
  }

  const Interval *find_key(const K &lo, const K &hi) const {
    Node *n = find_any(root, Interval{lo, hi});
    return (n == nullptr) ? nullptr : &(n->interval);
  }

  const V *find_value(const K &lo, const K &hi) const {
    Node *n = find_any(root, Interval{lo, hi});
    return (n == nullptr) ? nullptr : &(n->value);
  }

  template<class Fn>
  void find_all(const K &lo, const K &hi, Fn f) const {
    find_all(root, Interval{lo, hi}, f);
  }

  std::vector<std::tuple<K, K, V>> entries() const {
    std::vector<std::tuple<K, K, V>> res;
    res.reserve(num_nodes);
    collect_entries(root, res);
    return res;
  }
};

/*** Example Usage and Output:

Intervals intersecting [16, 20]: [5, 20] [10, 30] [10, 40] [15, 20]
All intervals: [5, 20] [10, 30] [10, 40] [12, 15] [15, 20]

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  IntervalTreap<int, char> t;
  t.insert(15, 20, 'a');
  t.insert(10, 30, 'b');
  t.insert(17, 19, 'c');
  t.insert(5, 20, 'd');
  t.insert(12, 15, 'e');
  t.insert(10, 40, 'f');
  assert(t.size() == 6);
  assert(!t.insert(5, 20, 'x'));
  t.erase(17, 19);
  assert(t.size() == 5);
  assert(*t.find_key(3, 9) == (pair<int, int>{5, 20}));
  assert(*t.find_value(3, 9) == 'd');
  cout << "Intervals intersecting [16, 20]:";
  auto print = [](int lo, int hi, char v) { cout << " [" << lo << ", " << hi << "]"; };
  t.find_all(16, 20, print);
  cout << "\nAll intervals:";
  for (const auto &[lo, hi, v] : t.entries()) {
    print(lo, hi, v);
  }
  cout << endl;
  return 0;
}
