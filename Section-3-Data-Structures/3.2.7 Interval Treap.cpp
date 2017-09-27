/*

Maintain a map from closed, one-dimensional intervals to values while supporting
efficient reporting of any or all entries that intersect with a given query
interval. This implementation uses std::pair to represent intervals, requiring
operators < and == to be defined on the numeric key type. A treap is used to
process the entries, where keys are compared lexicographically as pairs.

- interval_treap() constructs an empty map.
- size() returns the size of the map.
- empty() returns whether the map is empty.
- insert(lo, hi, v) adds an entry with key [lo, hi] and value v to the map,
  returning true if a new interval was added or false if the interval already
  exists (in which case the map is unchanged and the old value associated with
  the key is preserved).
- erase(lo, hi) removes the entry with key [lo, hi] from the map, returning true
  if the removal was successful or false if the interval was not found.
- find_key(lo, hi) returns a pointer to a const std::pair representing the key
  of some interval in the map which intersects with [lo, hi], or NULL if no such
  entry was found.
- find_value(lo, hi) returns a pointer to a const value of some entry in the map
  with a key that intersects with [lo, hi], or NULL if no such entry was found.
- find_all(lo, hi, f) calls the function f(lo, hi, v) on each entry in the map
  that overlaps with [lo, hi], in lexicographically ascending order of intervals.
- walk(f) calls the function f(lo, hi, v) on each interval in the map, in
  lexicographically ascending order of intervals.

Time Complexity:
- O(1) per call to the constructor, size(), and empty().
- O(log n) on average per call to insert(), erase(), and find_any(), where n is
  the number of intervals currently in the set.
- O(log n + m) on average per call to find_all(), where m is the number of
  intersecting intervals that are reported.
- O(n) per call to walk().

Space Complexity:
- O(n) for storage of the map elements.
- O(1) auxiliary for size() and empty().
- O(log n) auxiliary stack space on average for all other operations.

*/

#include <cstdlib>
#include <utility>

template<class K, class V>
class interval_treap {
  typedef std::pair<K, K> interval_t;

  struct node_t {
    static inline int rand32() {
      return (rand() & 0x7fff) | ((rand() & 0x7fff) << 15);
    }

    interval_t interval;
    V value;
    K max;
    int priority;
    node_t *left, *right;

    node_t(const interval_t &i, const V &v)
        : interval(i), value(v), max(i.second), priority(rand32()), left(NULL),
          right(NULL) {}

    void update() {
      max = interval.second;
      if (left != NULL && left->max > max) {
        max = left->max;
      }
      if (right != NULL && right->max > max) {
        max = right->max;
      }
    }
  } *root;

  int num_nodes;

  static void rotate_left(node_t *&n) {
    node_t *tmp = n;
    n = n->right;
    tmp->right = n->left;
    n->left = tmp;
    tmp->update();
  }

  static void rotate_right(node_t *&n) {
    node_t *tmp = n;
    n = n->left;
    tmp->left = n->right;
    n->right = tmp;
    tmp->update();
  }

  static bool insert(node_t *&n, const interval_t &i, const V &v) {
    if (n == NULL) {
      n = new node_t(i, v);
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

  static bool erase(node_t *&n, const interval_t &i) {
    if (n == NULL) {
      return false;
    }
    if (i < n->interval) {
      return erase(n->left, i);
    }
    if (i > n->interval) {
      return erase(n->right, i);
    }
    if (n->left != NULL && n->right != NULL) {
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
    node_t *tmp = (n->left != NULL) ? n->left : n->right;
    delete n;
    n = tmp;
    return true;
  }

  static node_t* find_any(node_t *n, const interval_t &i) {
    if (n == NULL) {
      return NULL;
    }
    if (n->interval.first <= i.second && i.first <= n->interval.second) {
      return n;
    }
    if (n->left != NULL && i.first <= n->left->max) {
      return find_any(n->left, i);
    }
    return find_any(n->right, i);
  }

  template<class KVFunction>
  static void find_all(node_t *n, const interval_t &i, KVFunction f) {
    if (n == NULL || n->max < i.first) {
      return;
    }
    if (n->interval.first <= i.second && i.first <= n->interval.second) {
      f(n->interval.first, n->interval.second, n->value);
    }
    find_all(n->left, i, f);
    find_all(n->right, i, f);
  }

  template<class KVFunction>
  static void walk(node_t *n, KVFunction f) {
    if (n != NULL) {
      walk(n->left, f);
      f(n->interval.first, n->interval.second, n->value);
      walk(n->right, f);
    }
  }

  static void clean_up(node_t *n) {
    if (n != NULL) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  interval_treap() : root(NULL), num_nodes(0) {}

  ~interval_treap() {
    clean_up(root);
  }

  int size() const {
    return num_nodes;
  }

  bool empty() const {
    return root == NULL;
  }

  bool insert(const K &lo, const K &hi, const V &v) {
    if (insert(root, std::make_pair(lo, hi), v)) {
      num_nodes++;
      return true;
    }
    return false;
  }

  bool erase(const K &lo, const K &hi) {
    if (erase(root, std::make_pair(lo, hi))) {
      num_nodes--;
      return true;
    }
    return false;
  }

  const interval_t* find_key(const K &lo, const K &hi) const {
    node_t *n = find_any(root, std::make_pair(lo, hi));
    return (n == NULL) ? NULL : &(n->interval);
  }

  const V* find_value(const K &lo, const K &hi) const {
    node_t *n = find_any(root, std::make_pair(lo, hi));
    return (n == NULL) ? NULL : &(n->value);
  }

  template<class KVFunction>
  void find_all(const K &lo, const K &hi, KVFunction f) const {
    find_all(root, std::make_pair(lo, hi), f);
  }

  template<class KVFunction>
  void walk(KVFunction f) const {
    walk(root, f);
  }
};

/*** Example Usage and Output:

Intervals intersecting [16, 20]: [15, 20] [10, 30] [5, 20] [10, 40]
All intervals: [5, 20] [10, 30] [10, 40] [12, 15] [15, 20]

***/

#include <cassert>
#include <iostream>
using namespace std;

void print(int lo, int hi, char v) {
  cout << " [" << lo << ", " << hi << "]";
}

int main() {
  interval_treap<int, char> t;
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
  assert(*t.find_key(3, 9) == make_pair(5, 20));
  assert(*t.find_value(3, 9) == 'd');
  cout << "Intervals intersecting [16, 20]:";
  t.find_all(16, 20, print);
  cout << "\nAll intervals:";
  t.walk(print);
  cout << endl;
  return 0;
}
