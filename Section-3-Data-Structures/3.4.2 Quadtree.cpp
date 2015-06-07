/*

3.4.2 - Quadtree with Compression

Description: A quadtree can be used to dynamically query values
of rectangles in a 2D array. In a quadtree, every node has exactly
4 children. The following uses dynamically allocated memory to
store the nodes, which allows arbitrarily large indices to exist
without affecting the performance of operations.

Time Complexity: For update(), query() and at(): O(log(N*M)) on
average and O(sqrt(N*M)) in the worst case, where N is the number
of rows and M is the number of columns in the 2D array.

Space Complexity: O(N*M)

Note: This implementation is 0-based. Valid indices for
all operations are [0..XMAX][0..YMAX]

*/

#include <algorithm> /* std::max(), std::min() */
#include <limits>    /* std::numeric_limits<T>::min() */

template<class T> struct quadtree {

  //these can be set to large values without affecting your memory usage!
  static inline const int xmax() { return 1000000000; }
  static inline const int ymax() { return 1000000000; }

  //define the following yourself. merge(x, nullv) must return x for all valid x
  static inline const T nullv() { return std::numeric_limits<T>::min(); }
  static inline const T merge(const T & a, const T & b) { return a > b ? a : b; }

  int X, Y, XL, XH, YL, YH; T V; //temp vals for speed

  struct node_t {
    node_t * child[4];
    int xl, xh, yl, yh;
    T val;

    node_t(int x, int y) {
      xl = xh = x;
      yl = yh = y;
      child[0] = child[1] = child[2] = child[3] = 0;
      val = nullv();
    }
  } *root;

  void update(node_t *& n, int xl, int xh, int yl, int yh) {
    if (X < xl || X > xh || Y < yl || Y > yh) return;
    if (!n) n = new node_t(X, Y);
    if (xl == xh && yl == yh) {
      n->val = V;
      return;
    }
    int xmid = (xl + xh)/2, ymid = (yl + yh)/2;
    update(n->child[0], xl, xmid, yl, ymid);
    update(n->child[1], xmid + 1, xh, yl, ymid);
    update(n->child[2], xl, xmid, ymid + 1, yh);
    update(n->child[3], xmid + 1, xh, ymid + 1, yh);
    for (int i = 0; i < 4; i++) {
      if (!n->child[i] || n->child[i]->val == nullv()) continue;
      n->xl = std::min(n->xl, n->child[i]->xl);
      n->xh = std::max(n->xh, n->child[i]->xh);
      n->yl = std::min(n->yl, n->child[i]->yl);
      n->yh = std::max(n->yh, n->child[i]->yh);
      n->val = merge(n->val, n->child[i]->val);
    }
  }

  void query(node_t * n) {
    if (!n || n->xl > XH || n->xh < XL || n->yh < YL || n->yl > YH ||
        merge(n->val, V) == V)
      return;
    if (n->xl >= XL && n->yl >= YL && n->xh <= XH && n->yh <= YH) {
      V = merge(V, n->val);
      return;
    }
    for (int i = 0; i < 4; i++) query(n->child[i]);
  }

  static void clean_up(node_t * n) {
    if (n == 0) return;
    for (int i = 0; i < 4; i++) clean_up(n->child[i]);
    delete(n);
  }

 public:
  quadtree() { root = 0; }
  ~quadtree() { clean_up(root); }
  T at(int x, int y) { return query(x, y, x, y); }

  void update(int x, int y, const T & v) {
    X = x; Y = y; V = v;
    update(root, 0, xmax() - 1, 0, ymax() - 1);
  }

  T query(int xl, int yl, int xh, int yh) {
    XL = xl; XH = xh;
    YL = yl; YH = yh;
    V = nullv();
    query(root);
    return V;
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int arr[5][5] = {{1, 2, 3, 4, 5},
                   {5, 4, 3, 2, 1},
                   {6, 7, 8, 0, 0},
                   {0, 1, 2, 3, 4},
                   {5, 9, 9, 1, 2}};
  quadtree<int> T;
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      T.update(r, c, arr[r][c]);

  cout << "The maximum value in the rectangle with ";
  cout << "upper left (0,2) and lower right (3,4) is ";
  cout << T.query(0, 2, 3, 4) << ".\n"; //8
  return 0;
}
