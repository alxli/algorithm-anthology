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
- O(log(MAXR) + log(MAXC)) auxiliary stack space for update(), query(), and
  at().

*/

#include <algorithm>
#include <cstdlib>

template<class T>
class segment_tree_2d {
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

    inner_node_t(int lo, int hi, const T &v)
        : value(v), low(lo), high(hi), left(NULL), right(NULL) {}
  };

  struct outer_node_t {
    inner_node_t root;
    int low, high;
    outer_node_t *left, *right;

    outer_node_t(int lo, int hi, const T &v)
        : root(0, MAXC, v), low(lo), high(hi), left(NULL), right(NULL) {}
  } *root;

  T init;

  // Helper variables for query() and update().
  int tgt_r1, tgt_c1, tgt_r2, tgt_c2, width;

  template<class node_t>
  inline T call_query(node_t *n, int area) {
    return (n != NULL) ? query(n) : join_region(init, area);
  }

  T query(inner_node_t *n) {
    int lo = n->low, hi = n->high, mid = lo + (hi - lo)/2;
    if (tgt_c1 <= lo && hi <= tgt_c2) {
      T res = n->value;
      if (tgt_c1 < lo) {
        res = join_values(res, join_region(init, lo - tgt_c1 + 1));
      }
      if (hi < tgt_c2) {
        res = join_values(res, join_region(init, tgt_c2 - hi + 1));
      }
      return res;
    } else if (tgt_c2 <= mid) {
      return call_query(n->left, tgt_c2 - tgt_c1 + 1);
    } else if (mid < tgt_c1) {
      return call_query(n->right, tgt_c2 - tgt_c1 + 1);
    }
    return join_values(
        call_query(n->left, std::min(tgt_c2, mid) - tgt_c1 + 1),
        call_query(n->right, tgt_c2 - std::max(tgt_c1, mid + 1) + 1));
  }

  T query(outer_node_t *n) {
    int lo = n->low, hi = n->high, mid = lo + (hi - lo)/2;
    if (tgt_r1 <= lo && hi <= tgt_r2) {
      T res = query(&(n->root));
      if (tgt_r1 < lo) {
        res = join_values(res, join_region(init, width*(lo - tgt_r1 + 1)));
      }
      if (hi < tgt_r2) {
        res = join_values(res, join_region(init, width*(tgt_r2 - hi + 1)));
      }
      return res;
    } else if (tgt_r2 <= mid) {
      return call_query(n->left, tgt_r2 - tgt_r1 + 1);
    } else if (mid < tgt_r1) {
      return call_query(n->right, tgt_r2 - tgt_r1 + 1);
    }
    return join_values(
        call_query(n->left, width*(std::min(tgt_r2, mid) - tgt_r1 + 1)),
        call_query(n->right, width*(tgt_r2 - std::max(tgt_r1, mid + 1) + 1)));
  }

  void update(inner_node_t *n, int c, const T &d, bool leaf_row) {
    int lo = n->low, hi = n->high, mid = lo + (hi - lo)/2;
    if (lo == hi) {
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
          hi = mid;
        } else {
          lo = mid + 1;
        }
        mid = lo + (hi - lo)/2;
      } while ((c <= mid) == (target->low <= mid));
      inner_node_t *tmp = new inner_node_t(lo, hi, init);
      if (target->low <= mid) {
        tmp->left = target;
      } else {
        tmp->right = target;
      }
      target = tmp;
      update(tmp, c, d, leaf_row);
    }
    T left_value = (n->left != NULL) ? n->left->value
                                     : join_region(init, mid - lo + 1);
    T right_value = (n->right != NULL) ? n->right->value
                                       : join_region(init, hi - mid);
    n->value = join_values(left_value, right_value);
  }

  void update(outer_node_t *n, int r, int c, const T &d) {
    int lo = n->low, hi = n->high, mid = lo + (hi - lo)/2;
    if (lo == hi) {
      update(&(n->root), c, d, true);
      return;
    }
    if (r <= mid) {
      if (n->left == NULL) {
        n->left = new outer_node_t(lo, mid, init);
      }
      update(n->left, r, c, d);
    } else {
      if (n->right == NULL) {
        n->right = new outer_node_t(mid + 1, hi, init);
      }
      update(n->right, r, c, d);
    }
    T value = join_region(init, hi - lo + 1);
    if (n->left != NULL || n->right != NULL) {
      tgt_c1 = tgt_c2 = c;
      T left_value = (n->left != NULL) ? query(&(n->left->root))
                                       : join_region(init, mid - lo + 1);
      T right_value = (n->right != NULL) ? query(&(n->right->root))
                                         : join_region(init, hi - mid);
      value = join_values(left_value, right_value);
    }
    update(&(n->root), c, value, false);
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
  segment_tree_2d(const T &v = T())
      : root(new outer_node_t(0, MAXR, v)), init(v) {}

  ~segment_tree_2d() {
    clean_up(root);
  }

  T at(int r, int c) {
    return query(r, c, r, c);
  }

  T query(int r1, int c1, int r2, int c2) {
    tgt_r1 = r1;
    tgt_c1 = c1;
    tgt_r2 = r2;
    tgt_c2 = c2;
    width = c2 - c1;
    return query(root);
  }

  void update(int r, int c, const T &d) {
    update(root, r, c, d);
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
