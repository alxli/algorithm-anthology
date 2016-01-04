/*

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

template<class T> class quadtree {
  //these can be set to large values without affecting your memory usage!
  static const int xmax = 1000000000;
  static const int ymax = 1000000000;

  //define the following yourself. merge(x, nullv) must return x for all valid x
  static inline T nullv() { return std::numeric_limits<T>::min(); }
  static inline T merge(const T & a, const T & b) { return a > b ? a : b; }

  int X, Y, X1, X2, Y1, Y2;  T V; //temp vals for speed

  struct node_t {
    node_t * child[4];
    int x1, x2, y1, y2;
    T value;

    node_t(int x, int y) {
      x1 = x2 = x;
      y1 = y2 = y;
      child[0] = child[1] = child[2] = child[3] = 0;
      value = nullv();
    }
  } *root;

  void update(node_t *& n, int x1, int x2, int y1, int y2) {
    if (X < x1 || X > x2 || Y < y1 || Y > y2) return;
    if (n == 0) n = new node_t(X, Y);
    if (x1 == x2 && y1 == y2) {
      n->value = V;
      return;
    }
    int xmid = (x1 + x2)/2, ymid = (y1 + y2)/2;
    update(n->child[0], x1, xmid, y1, ymid);
    update(n->child[1], xmid + 1, x2, y1, ymid);
    update(n->child[2], x1, xmid, ymid + 1, y2);
    update(n->child[3], xmid + 1, x2, ymid + 1, y2);
    for (int i = 0; i < 4; i++) {
      if (!n->child[i] || n->child[i]->value == nullv()) continue;
      n->x1 = std::min(n->x1, n->child[i]->x1);
      n->x2 = std::max(n->x2, n->child[i]->x2);
      n->y1 = std::min(n->y1, n->child[i]->y1);
      n->y2 = std::max(n->y2, n->child[i]->y2);
      n->value = merge(n->value, n->child[i]->value);
    }
  }

  void query(node_t * n) {
    if (n == 0 || n->x1 > X2 || n->x2 < X1 || n->y2 < Y1 || n->y1 > Y2 ||
        merge(n->value, V) == V)
      return;
    if (n->x1 >= X1 && n->y1 >= Y1 && n->x2 <= X2 && n->y2 <= Y2) {
      V = merge(V, n->value);
      return;
    }
    for (int i = 0; i < 4; i++) query(n->child[i]);
  }

  static void clean_up(node_t * n) {
    if (n == 0) return;
    for (int i = 0; i < 4; i++) clean_up(n->child[i]);
    delete n;
  }

 public:
  quadtree() { root = 0; }
  ~quadtree() { clean_up(root); }

  void update(int x, int y, const T & v) {
    X = x;
    Y = y;
    V = v;
    update(root, 0, xmax - 1, 0, ymax - 1);
  }

  T query(int x1, int y1, int x2, int y2) {
    X1 = x1;
    X2 = x2;
    Y1 = y1;
    Y2 = y2;
    V = nullv();
    query(root);
    return V;
  }

  T at(int x, int y) {
    return query(x, y, x, y);
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
