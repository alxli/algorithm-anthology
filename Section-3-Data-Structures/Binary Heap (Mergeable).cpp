
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

template<class T> class mergeable_heap {
  struct node_t {
    T value;
    node_t *left, *right;

    node_t(const T &v) {
      value = v;
      left = right = NULL;
    }
  } *root;

  int num_nodes;

  static node_t* merge(node_t *a, node_t *b) {
    if (a == NULL) {
      return b;
    }
    if (b == NULL) {
      return a;
    }
    if (a->value > b->value) {
      std::swap(a, b);
    }
    if (rand() % 2 == 1) {
      std::swap(a->left, a->right);
    }
    a->left = merge(a->left, b);
    return a;
  }

  static void clean_up(node_t *n) {
    if (n != NULL) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  mergeable_heap() {
    root = NULL;
    num_nodes = 0;
  }

  template<class It> mergeable_heap(It lo, It hi) {
    root = NULL;
    num_nodes = 0;
    while (lo != hi) {
      push(*(lo++));
    }
  }

  ~mergeable_heap() {
    clean_up(root);
  }

  int size() {
    return num_nodes;
  }

  bool empty() {
    return root == NULL;
  }

  void push(const T &v) {
    root = merge(root, new node_t(v));
    num_nodes++;
  }

  void pop() {
    if (empty()) {
      throw std::runtime_error("Cannot pop from empty heap.");
    }
    node_t *tmp = root;
    root = merge(root->left, root->right);
    delete tmp;
    num_nodes--;
  }

  T top() {
    if (empty()) {
      throw std::runtime_error("Cannot get top of empty heap.");
    }
    return root->value;
  }

  void merge(const mergeable_heap &h) {
    root = merge(root, h.root);
  }
};

/*** Example Usage and Output:

-1
0
5
10
12

***/

#include <iostream>
using namespace std;

int main() {
  mergeable_heap<int> h, h2;
  h.push(12);
  h.push(10);
  h2.push(5);
  h2.push(-1);
  h2.push(0);
  h.merge(h2);
  while (!h.empty()) {
    cout << h.top() << endl;
    h.pop();
  }
  return 0;
}
