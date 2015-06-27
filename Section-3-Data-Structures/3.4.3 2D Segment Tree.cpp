/*

3.4.3 - 2D Segment Tree

Description: A quadtree is a segment tree but with 4 children
per node, making its running time proportional to the square
root of the number of leaves. However, a 2D segment tree is a
segment tree of segment trees, making its running time
proportional to the log of its size. The following implementation
is a highly optimized implementation with features such as
coordinate compression and path compression.

Time Complexity: O(log(xmax)*log(ymax)) for update(), query(),
and at() operations. size() is O(1).

Space Complexity: Left as an exercise for the reader.

Note: This implementation is 0-based. Valid indices for
all operations are [0..xmax][0..ymax]

*/

#include <limits> /* std::numeric_limits<T>::min() */

template<class T> class segment_tree_2d {
  //these can be set to large values without affecting your memory usage!
  static const int xmax = 1000000000;
  static const int ymax = 1000000000;

  //define the following yourself. merge(x, nullv) must return x for all valid x
  static inline T nullv() { return std::numeric_limits<T>::min(); }
  static inline T merge(const T & a, const T & b) { return a > b ? a : b; }

  struct layer2_node {
    int lo, hi;
    layer2_node *L, *R;
    T value;
    layer2_node(int l, int h) : lo(l), hi(h), L(0), R(0) {}
  };

  struct layer1_node {
    layer1_node *L, *R;
    layer2_node l2;
    layer1_node() : L(0), R(0), l2(0, ymax) {}
  } *root;

  void update2(layer2_node * node, int Q, const T & v) {
    int lo = node->lo, hi = node->hi, mid = (lo + hi)/2;
    if (lo + 1 == hi) {
      node->value = v;
      return;
    }
    layer2_node *& tgt = Q < mid ? node->L : node->R;
    if (tgt == 0) {
      tgt = new layer2_node(Q, Q + 1);
      tgt->value = v;
    } else if (tgt->lo <= Q && Q < tgt->hi) {
      update2(tgt, Q, v);
    } else {
      do {
        (Q < mid ? hi : lo) = mid;
        mid = (lo + hi)/2;
      } while ((Q < mid) == (tgt->lo < mid));
      layer2_node *nnode = new layer2_node(lo, hi);
      (tgt->lo < mid ? nnode->L : nnode->R) = tgt;
      tgt = nnode;
      update2(nnode, Q, v);
    }
    node->value = merge(node->L ? node->L->value : nullv(),
                        node->R ? node->R->value : nullv());
  }

  T query2(layer2_node * nd, int A, int B) {
    if (nd == 0 || B <= nd->lo || nd->hi <= A) return nullv();
    if (A <= nd->lo && nd->hi <= B) return nd->value;
    return merge(query2(nd->L, A, B), query2(nd->R, A, B));
  }

  void update1(layer1_node * node, int lo, int hi, int x, int y, const T & v) {
    if (lo + 1 == hi) update2(&node->l2, y, v);
    else {
      int mid = (lo + hi)/2;
      layer1_node *& nnode = x < mid ? node->L : node->R;
      (x < mid ? hi : lo) = mid;
      if (nnode == 0) nnode = new layer1_node();
      update1(nnode, lo, hi, x, y, v);
      update2(&node->l2, y, merge(
      	node->L ? query2(&node->L->l2, y, y + 1) : nullv(),
        node->R ? query2(&node->R->l2, y, y + 1) : nullv())
      );
    }
  }

  T query1(layer1_node * nd, int lo, int hi, int A1, int B1, int A2, int B2) {
    if (nd == 0 || B1 <= lo || hi <= A1) return nullv();
    if (A1 <= lo && hi <= B1) return query2(&nd->l2, A2, B2);
    int mid = (lo + hi) / 2;
    return merge(query1(nd->L, lo, mid, A1, B1, A2, B2),
                 query1(nd->R, mid, hi, A1, B1, A2, B2));
  }

  void clean_up2(layer2_node * n) {
    if (n == 0) return;
    clean_up2(n->L);
    clean_up2(n->R);
    delete n;
  }

  void clean_up1(layer1_node * n) {
    if (n == 0) return;
    clean_up2(n->l2.L);
    clean_up2(n->l2.R);
    clean_up1(n->L);
    clean_up1(n->R);
    delete n;
  }

 public:
  segment_tree_2d() { root = new layer1_node(); }
  ~segment_tree_2d() { clean_up1(root); }

  void update(int x, int y, const T & v) {
    update1(root, 0, xmax, x, y, v);
  }

  T query(int x1, int y1, int x2, int y2) {
    return query1(root, 0, xmax, x1, x2 + 1, y1, y2 + 1);
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
  segment_tree_2d<int> T;
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      T.update(r, c, arr[r][c]);
  cout << "The maximum value in the rectangle with ";
  cout << "upper left (0,2) and lower right (3,4) is ";
  cout << T.query(0, 2, 3, 4) << ".\n"; //8
  return 0;
}
