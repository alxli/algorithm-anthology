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

- quadtree(v) constructs a two-dimensional array with rows from 0 to MAXR and
  columns from 0 to MAXC, inclusive. All values are implicitly initialized to v.
- at(r, c) returns the value at row r, column c.
- query(r1, c1, r2, c2) returns the result of join_values() applied to every
  value in the rectangular region consisting of rows from r1 to r2, inclusive,
  and columns from c1 to c2, inclusive.
- update(r, c, d) assigns the value v at (r, c) to join_value_with_delta(v, d).

Time Complexity:
- O(1) per call to the constructor.
- O(max(MAXR, MAXC)) per call to at(), update(), and query().

Space Complexity:
- O(n) for storage of the array elements, where n is the number of updated
  entries in the array.
- O(sqrt(max(MAXR, MAXC))) auxiliary stack space for update(), query(), and
  at().

*/

#include <algorithm>
#include <cstdlib>

template<class T>
class quadtree {
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

  struct node_t {
    T value;
    node_t *child[4];

    node_t(const T &v) : value(v) {
      for (int i = 0; i < 4; i++) {
        child[i] = NULL;
      }
    }
  };

  node_t *root;
  T init;

  // Helper variables for query().
  int tgt_r1, tgt_c1, tgt_r2, tgt_c2;
  T res;
  bool found;

  void query(node_t *n, int r1, int c1, int r2, int c2) {
    if (tgt_r2 < r1 || tgt_r1 > r2 || tgt_c2 < c1 || tgt_c1 > c2) {
      return;
    }
    if (n == NULL) {
      int rlen = std::min(r2, tgt_r2) - std::max(r1, tgt_r1) + 1;
      int clen = std::min(c2, tgt_c2) - std::max(c1, tgt_c1) + 1;
      T v = join_region(init, rlen*clen);
      res = found ? join_values(res, v) : v;
      found = true;
      return;
    }
    if (tgt_r1 <= r1 && r2 <= tgt_r2 && tgt_c1 <= c1 && c2 <= tgt_c2) {
      res = found ? join_values(res, n->value) : n->value;
      found = true;
      return;
    }
    int rmid = r1 + (r2 - r1)/2, cmid = c1 + (c2 - c1)/2;
    query(n->child[0], r1, c1, rmid, cmid);
    query(n->child[1], rmid + 1, c1, r2, cmid);
    query(n->child[2], r1, cmid + 1, rmid, c2);
    query(n->child[3], rmid + 1, cmid + 1, r2, c2);
  }

  // Helper variables for update().
  int tgt_r, tgt_c;
  T delta;

  void update(node_t *&n, int r1, int c1, int r2, int c2) {
    if (n == NULL) {
      n = new node_t(join_region(init, (r2 - r1 + 1)*(c2 - r1 + 1)));
    }
    if (tgt_r < r1 || tgt_r > r2 || tgt_c < c1 || tgt_c > c2) {
      return;
    }
    if (r1 == r2 && c1 == c2) {
      n->value = join_value_with_delta(n->value, delta);
      return;
    }
    int rmid = r1 + (r2 - r1)/2, cmid = c1 + (c2 - c1)/2;
    update(n->child[0], r1, c1, rmid, cmid);
    update(n->child[1], rmid + 1, c1, r2, cmid);
    update(n->child[2], r1, cmid + 1, rmid, c2);
    update(n->child[3], rmid + 1, cmid + 1, r2, c2);
    bool found = false;
    for (int i = 0; i < 4; i++) {
      n->value = found ? join_values(n->value, n->child[i]->value)
                       : n->child[i]->value;
      found = true;
    }
  }

  static void clean_up(node_t *n) {
    if (n != NULL) {
      for (int i = 0; i < 4; i++) {
        clean_up(n->child[i]);
      }
      delete n;
    }
  }

 public:
  quadtree(const T &v = T()) : root(NULL), init(v) {}

  ~quadtree() {
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
    found = false;
    query(root, 0, 0, MAXR, MAXC);
    return found ? res : join_region(init, (r2 - r1 + 1)*(c2 - c1 + 1));
  }

  void update(int r, int c, const T &d) {
    tgt_r = r;
    tgt_c = c;
    delta = d;
    update(root, 0, 0, MAXR, MAXC);
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
  quadtree<int> t(0);
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
