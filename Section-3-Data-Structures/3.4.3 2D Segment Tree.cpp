/*

Maintain a two-dimensional array while supporting dynamic queries of rectangular
sub-arrays and dynamic updates of individual indices. This implementation uses
lazy initialization of nodes to conserve memory while supporting large indices.

The query operation is defined by the join_values() and join_region() functions
where join_values(x, join_values(y, z)) = join_values(join_values(x, y), z) for
all values x, y, and z in the array. The join_region(v, area) function must be
defined in conjunction to efficiently return the result of join_values() applied
to a rectangular sub-array of area elements. The default code below assumes a
numerical array type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case join_values(a, b)
should return "a + b" and join_region(v, area) should return "v*area".

The update operation is defined by the join_value_with_delta() function, which
determines the change made to array values. The default code below defines
updates that "set" the chosen array index to a new value. Another possible
update operation is "increment", in which join_value_with_delta(v, d) should be
defined to return "v + d".

- segment_tree_2d(v) constructs a two-dimensional array with rows from 0 to
  MAXR and columns from 0 to MAXC, inclusive. All values are implicitly
  initialized to v.
- at(r, c) returns the value at row r, column c.
- query(r1, c1, r2, c2) returns the result of join_values() applied to every
  value in the rectangular region consisting of rows from r1 to r2, inclusive,
  and columns from c1 to c2, inclusive.
- update(r, c, d) assigns the value v at (r, c) to join_value_with_delta(v, d).

Time Complexity:
- O(1) per call to the constructor.
- O(log(MAXR)*log(MAXC)) per call to at(), update(), and query().

Space Complexity:
- O(n) for storage of the array elements, where n is the number of updated
  entries in the array.
- O(log(MAXR) + log(MAXC)) auxiliary stack space per call to update(), query(),
  and at().

*/

#include <algorithm>
#include <cstdlib>

template<class T> class segment_tree_2d {
  static const int MAXR = 1000000000;
  static const int MAXC = 1000000000;

  static T join_values(const T &a, const T &b) {
    return std::min(a, b);
  }

  static T join_region(const T &v, int area) {
    return v;
  }

  static T join_value_with_delta(const T &v, const T &d) {
    return d;
  }

  struct inner_node_t {
    T value;
    int low, high;
    inner_node_t *left, *right;

    inner_node_t(int l, int h, const T &v)
        : value(v), low(l), high(h), left(NULL), right(NULL) {}
  };

  struct outer_node_t {
    inner_node_t root;
    int low, high;
    outer_node_t *left, *right;

    outer_node_t(int l, int h, const T &v)
        : root(0, MAXC, v), low(l), high(h), left(NULL), right(NULL) {}
  } *root;

  T init;

  void update(inner_node_t *n, int c, const T &d, bool leaf_row) {
    int l = n->low, h = n->high, mid = l + (h - l)/2;
    if (l == h) {
      if (leaf_row) {
        n->value = join_value_with_delta(n->value, d);
      } else {
        n->value = d;
      }
      return;
    }
    inner_node_t *&target = (c <= mid) ? n->left : n->right;
    if (target == NULL) {
      target = new inner_node_t(c, c, init);
    }
    if (target->low <= c && c <= target->high) {
      update(target, c, d, leaf_row);
    } else {
      do {
        if (c <= mid) {
          h = mid;
        } else {
          l = mid + 1;
        }
        mid = l + (h - l)/2;
      } while ((c <= mid) == (target->low <= mid));
      inner_node_t *tmp = new inner_node_t(l, h, init);
      if (target->low <= mid) {
        tmp->left = target;
      } else {
        tmp->right = target;
      }
      target = tmp;
      update(tmp, c, d, leaf_row);
    }
    T left_value = (n->left != NULL) ? n->left->value
                                     : join_region(init, mid - l + 1);
    T right_value = (n->right != NULL) ? n->right->value
                                       : join_region(init, h - mid);
    n->value = join_values(left_value, right_value);
  }

  void update(outer_node_t *n, int r, int c, const T &d) {
    int l = n->low, h = n->high, mid = l + (h - l)/2;
    if (l == h) {
      update(&(n->root), c, d, true);
      return;
    }
    if (r <= mid) {
      if (n->left == NULL) {
        n->left = new outer_node_t(l, mid, init);
      }
      update(n->left, r, c, d);
    } else {
      if (n->right == NULL) {
        n->right = new outer_node_t(mid + 1, h, init);
      }
      update(n->right, r, c, d);
    }
    T value = join_region(init, h - l + 1);
    if (n->left != NULL || n->right != NULL) {
      T left_value = (n->left != NULL) ? query(&(n->left->root), c, c)
                                       : join_region(init, mid - l + 1);
      T right_value = (n->right != NULL) ? query(&(n->right->root), c, c)
                                         : join_region(init, h - mid);
      value = join_values(left_value, right_value);
    }
    update(&(n->root), c, value, false);
  }

  template<class node_t>
  inline T call_query(node_t *n, int l, int h) {
    return (n != NULL) ? query(n, l, h) : join_region(init, h - l + 1);
  }

  T query(inner_node_t *n, int tgt_l, int tgt_h) {
    int l = n->low, h = n->high, mid = l + (h - l)/2;
    if (l == tgt_l && h == tgt_h) {
      return n->value;
    }
    if (tgt_l <= mid && mid < tgt_h) {
      return join_values(
                call_query(n->left, tgt_l, std::min(tgt_h, mid)),
                call_query(n->right, std::max(tgt_l, mid + 1), tgt_h));
    }
    if (tgt_l <= mid) {
      return call_query(n->left, tgt_l, std::min(tgt_h, mid));
    }
    return call_query(n->right, std::max(tgt_l, mid + 1), tgt_h);
  }

  int tgt_c1, tgt_c2;

  T query(outer_node_t *n, int tgt_l, int tgt_h) {
    int l = n->low, h = n->high, mid = l + (h - l)/2;
    if (l == tgt_l && h == tgt_h) {
      return query(&(n->root), tgt_c1, tgt_c2);
    }
    if (tgt_l <= mid && mid < tgt_h) {
      return join_values(
                call_query(n->left, tgt_l, std::min(tgt_h, mid)),
                call_query(n->right, std::max(tgt_l, mid + 1), tgt_h));
    }
    if (tgt_l <= mid) {
      return call_query(n->left, tgt_l, std::min(tgt_h, mid));
    }
    return call_query(n->right, std::max(tgt_l, mid + 1), tgt_h);
  }

  static void clean_up(inner_node_t *n) {
    if (n != NULL) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

  static void clean_up(outer_node_t *n) {
    if (n != NULL) {
      clean_up(n->root.left);
      clean_up(n->root.right);
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  segment_tree_2d(const T &v = T()) {
    root = new outer_node_t(0, MAXR, v);
    init = v;
  }

  ~segment_tree_2d() {
    clean_up(root);
  }

  void update(int r, int c, const T &d) {
    update(root, r, c, d);
  }

  T query(int r1, int c1, int r2, int c2) {
    tgt_c1 = c1;
    tgt_c2 = c2;
    return query(root, r1, r2);
  }

  T at(int r, int c) {
    return query(r, c, r, c);
  }
};

/*** Example Usage and Output:

Values:
7 6 0
5 4 0
0 1 9

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  segment_tree_2d<int> t(0);
  t.update(0, 0, 7);
  t.update(0, 1, 6);
  t.update(1, 0, 5);
  t.update(1, 1, 4);
  t.update(2, 1, 1);
  t.update(2, 2, 9);
  cout << "Values:" << endl;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      cout << t.at(i, j) << " ";
    }
    cout << endl;
  }
  assert(t.query(0, 0, 0, 1) == 6);
  assert(t.query(0, 0, 1, 0) == 5);
  assert(t.query(1, 1, 2, 2) == 0);
  assert(t.query(0, 0, 1000000000, 1000000000) == 0);
  t.update(500000000, 500000000, -100);
  assert(t.query(0, 0, 1000000000, 1000000000) == -100);
  return 0;
}
