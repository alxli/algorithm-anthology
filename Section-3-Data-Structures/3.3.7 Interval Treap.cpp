/*

Maintain a set of closed, one-dimensional intervals while supporting efficient
reporting of any or all intervals which intersect with a given query interval.
This implementation uses std::pair to represent intervals, requiring operators <
and == to be defined on the numeric template type. A treap is used to process
the intervals, where keys are intervals compared lexicographically as pairs.

- interval_treap() constructs an empty set.
- size() returns the size of the set.
- empty() returns whether the set is empty.
- insert(l, h) adds an interval [l, h] to the set, returning true if an new
  interval was added or false if the interval already exists (in which case the
  set is unchanged).
- erase(l, h) removes the interval [l, h] from the set, returning true if the
  removal was successful or false if the interval to be removed was not found.
- find_any(l, h) returns a pointer to a const std::pair representing an interval
  in the set which overlaps with [l, h], or NULL if no overlaps were found.
- find_all(l, h, f) calls the function f(pair<>) on each interval in the set
  that overlaps with [l, h], in lexicographically ascending order of intervals.
- walk(f) calls the function f(pair<>) on each interval in the set, in
  lexicographically ascending order of intervals.

Time Complexity:
- O(1) per call to the constructor, size(), and empty().
- O(log n) on average per call to insert(), erase(), and find_any(), where n is
  the number of intervals currently in the set.
- O(log n + m) on average per call to find_all(), where m is the number of
  intersecting intervals that are reported.
- O(n) per call to walk().

Space Complexity:
- O(n) for storage of the intervals.
- O(1) auxiliary for size() and empty().
- O(log n) auxiliary stack space on average for all other operations.

*/

#include <cstdlib>
#include <utility>

template<class T> class interval_treap {
  typedef std::pair<T, T> interval_t;

  struct node_t {
    static inline int rand32() {
      return (rand() & 0x7fff) | ((rand() & 0x7fff) << 15);
    }

    interval_t interval;
    node_t *left, *right;
    T max;
    int priority;

    node_t(const interval_t &i) {
      this->interval = i;
      max = i.second;
      left = right = NULL;
      priority = rand32();
    }

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

  static void rotate_l(node_t *&n) {
    node_t *tmp = n;
    n = n->right;
    tmp->right = n->left;
    n->left = tmp;
    tmp->update();
  }

  static void rotate_r(node_t *&n) {
    node_t *tmp = n;
    n = n->left;
    tmp->left = n->right;
    n->right = tmp;
    tmp->update();
  }

  static bool insert(node_t *&n, const interval_t &i) {
    if (n == NULL) {
      n = new node_t(i);
      return true;
    }
    if (i < n->interval && insert(n->left, i)) {
      if (n->left->priority < n->priority) {
        rotate_r(n);
      }
      n->update();
      return true;
    }
    if (i > n->interval && insert(n->right, i)) {
      if (n->right->priority < n->priority) {
        rotate_l(n);
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
    } else if (i > n->interval) {
      return erase(n->right, i);
    }
    if (n->left != NULL && n->right != NULL) {
      bool res;
      if (n->left->priority < n->right->priority) {
        rotate_r(n);
        res = erase(n->right, i);
      } else {
        rotate_l(n);
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

  template<class ReportFunction>
  static void find_all(node_t *n, const interval_t &i, ReportFunction f) {
    if (n == NULL || n->max < i.first) {
      return;
    }
    if (n->interval.first <= i.second && i.first <= n->interval.second) {
      f(n->interval);
    }
    find_all(n->left, i, f);
    find_all(n->right, i, f);
  }

  template<class ReportFunction>
  static void walk(node_t *n, ReportFunction f) {
    if (n != NULL) {
      walk(n->left, f);
      f(n->interval);
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
  interval_treap() {
    root = NULL;
    num_nodes = 0;
  }

  ~interval_treap() {
    clean_up(root);
  }

  int size() const {
    return num_nodes;
  }

  bool empty() const {
    return root == NULL;
  }

  bool insert(const T &l, const T &h) {
    if (insert(root, std::make_pair(l, h))) {
      num_nodes++;
      return true;
    }
    return false;
  }

  bool erase(const T &l, const T &h) {
    if (erase(root, std::make_pair(l, h))) {
      num_nodes--;
      return true;
    }
    return false;
  }

  const interval_t* find_any(const T &l, const T &h) {
    node_t *n = find_any(root, std::make_pair(l, h));
    return (n == NULL) ? NULL : &(n->interval);
  }

  template<class ReportFunction>
  void find_all(const T &l, const T &h, ReportFunction f) {
    find_all(root, std::make_pair(l, h), f);
  }

  template<class ReportFunction> void walk(ReportFunction f) {
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

void print(const pair<int, int> &i) {
  cout << " [" << i.first << ", " << i.second << "]";
}

int main() {
  interval_treap<int> t;
  t.insert(15, 20);
  t.insert(10, 30);
  t.insert(17, 19);
  t.insert(5, 20);
  t.insert(12, 15);
  t.insert(10, 40);
  assert(t.size() == 6);
  assert(!t.insert(5, 20));
  t.erase(17, 19);
  assert(t.size() == 5);
  assert(*t.find_any(3, 9) == make_pair(5, 20));
  cout << "Intervals intersecting [16, 20]:";
  t.find_all(16, 20, print);
  cout << "\nAll intervals:";
  t.walk(print);
  cout << "\n";
  return 0;
}
