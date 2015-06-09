/*

3.3.8 - 1D Interval Tree

Description: An interval tree is structure used to store and
efficiently query intervals. An interval may be dynamically
inserted, and range queries of [lo, hi] may be performed to
have the tree report all intervals that intersect with the
queried interval.

Time Complexity: O(log N) for insert() and O(k) for query(),
where N is the number of intervals in the tree and k is the
number of intervals that will be reported by the query().

Space Complexity: O(N) on the number of intervals in the tree.

*/

#include <utility> /* std:pair */

class interval_tree {
  typedef std::pair<int, int> interval;

  static bool overlap(const interval & a, const interval & b) {
    return a.first <= b.second && b.first <= a.second;
  }

  struct node_t {
    interval i;
    int maxh;
    node_t *L, *R;

    node_t(const interval & i) {
      this->i = i;
      maxh = i.second;
      L = R = 0;
    }
  } *root;

  interval i; //temporary

  void insert(node_t *& n) {
    if (n == 0) { n = new node_t(i); return; }
    int lo = (n->i).first;
    insert(i.first < lo ? n->L : n->R);
    if (n->maxh < i.second) n->maxh = i.second;
  }

  template<class ReportFunction>
  void query(node_t * n, ReportFunction f) {
    if (n == 0 || n->maxh < i.first) return;
    if (overlap(n->i, i)) f(n->i.first, n->i.second);
    query(n->L, f);
    query(n->R, f);
  }

  static void clean_up(node_t * n) {
    if (n == 0) return;
    clean_up(n->L);
    clean_up(n->R);
    delete n;
  }

 public:
  interval_tree() { root = 0; }
  ~interval_tree() { clean_up(root); }

  void insert(int lo, int hi) {
    i = interval(lo, hi);
    insert(root);
  }

  template<class ReportFunction>
  void query(int lo, int hi, ReportFunction f) {
    i = interval(lo, hi);
    query(root, f);
  }
};

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

void print(int lo, int hi) {
  cout << "[" << lo << "," << hi << "] ";
}

int cnt;
void count(int lo, int hi) { cnt++; }

int main() {
  int N = 6;
  int intv[6][2] = {{15, 20}, {10, 30}, {17, 19}, {5, 20}, {12, 15}, {30, 40}};
  interval_tree T;
  for (int i = 0; i < N; i++) {
    T.insert(intv[i][0], intv[i][1]);
  }
  T.query(10, 20, print); cout << "\n"; //[15,20] [10,30] [5,20] [12,15] [17,19]
  T.query(0, 5, print); cout << "\n";   //[5,20]
  T.query(25, 45, print); cout << "\n"; //[10,30] [30,40]
  //check correctness
  for (int l = 0; l <= 50; l++) {
    for (int h = l; h <= 50; h++) {
      cnt = 0;
      T.query(l, h, count);
      int cnt2 = 0;
      for (int i = 0; i < N; i++)
        if (intv[i][0] <= h && l <= intv[i][1])
          cnt2++;
      assert(cnt == cnt2);
    }
  }
  return 0;
}
