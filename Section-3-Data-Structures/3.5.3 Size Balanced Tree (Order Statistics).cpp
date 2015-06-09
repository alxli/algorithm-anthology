/*

3.5.3 - Size Balanced Tree with Order Statistics

Description: A binary search tree (BST) is a node-based binary tree data
structure where the left sub-tree of every node has keys less than the
node's key and the right sub-tree of every node has keys greater than the
node's key. A BST may be come degenerate like a linked list resulting in
an O(N) running time per operation. A self-balancing binary search tree
such as a randomized treap prevents the occurence of this known worst case.

The size balanced tree is a data structure first published in 2007 by
Chinese student Chen Qifeng. The tree is rebalanced by examining the sizes
of each node's subtrees. It is popular amongst Chinese OI competitors due
to its speed, simplicity to implement, and ability to double up as an
ordered statistics tree if necessary.
For more info, see: http://wcipeg.com/wiki/Size_Balanced_Tree

An ordered statistics tree is a BST that supports additional operations:
- Select(i): find the i-th smallest element stored in the tree
- Rank(x):   find the rank of element x in the tree,
             i.e. its index in the sorted list of elements of the tree
For more info, see: http://en.wikipedia.org/wiki/Order_statistic_tree

Note: The following implementation is used similar to an std::map. In order
to make it behave like an std::set, modify the code to remove the value
associated with each node. Making a size balanced tree behave like an
std::multiset or std::multimap is a more complex issue. Refer to the
articles above and determine the correct way to preserve the binary search
tree property with maintain() if equivalent keys are allowed.

Time Complexity: insert(), erase(), find(), select() and rank() are
O(log N) on the number of elements in the tree. walk() is O(N).

Space Complexity: O(N) on the number of nodes in the tree.

*/

#include <stdexcept> /* std::runtime_error */
#include <utility>   /* pair */

template<class key_t, class val_t> class size_balanced_tree {
  struct node_t {
    key_t key;
    val_t val;
    int size;
    node_t * c[2];

    node_t(const key_t & k, const val_t & v) {
      key = k, val = v;
      size = 1;
      c[0] = c[1] = 0;
    }

    void update() {
      size = 1;
      if (c[0]) size += c[0]->size;
      if (c[1]) size += c[1]->size;
    }
  } *root;

  static inline int size(node_t * n) {
    return n ? n->size : 0;
  }

  static void rotate(node_t *& n, bool d) {
    node_t * p = n->c[d];
    n->c[d] = p->c[!d];
    p->c[!d] = n;
    n->update();
    p->update();
    n = p;
  }

  static void maintain(node_t *& n, bool d) {
    if (n == 0 || n->c[d] == 0) return;
    node_t *& p = n->c[d];
    if (size(p->c[d]) > size(n->c[!d])) {
      rotate(n, d);
    } else if (size(p->c[!d]) > size(n->c[!d])) {
      rotate(p, !d);
      rotate(n, d);
    } else return;
    maintain(n->c[0], 0);
    maintain(n->c[1], 1);
    maintain(n, 0);
    maintain(n, 1);
  }

  static void insert(node_t *& n, const key_t & k, const val_t & v) {
    if (n == 0) {
      n = new node_t(k, v);
      return;
    }
    if (k < n->key) {
      insert(n->c[0], k, v);
      maintain(n, 0);
    } else if (n->key < k) {
      insert(n->c[1], k, v);
      maintain(n, 1);
    } else return;
    n->update();
  }

  static void erase(node_t *& n, const key_t & k) {
    if (n == 0) return;
    bool d = k < n->key;
    if (k < n->key) {
      erase(n->c[0], k);
    } else if (n->key < k) {
      erase(n->c[1], k);
    } else {
      if (n->c[1] == 0 || n->c[0] == 0) {
        delete n;
        n = n->c[1] == 0 ? n->c[0] : n->c[1];
        return;
      }
      node_t * p = n->c[1];
      while (p->c[0] != 0) p = p->c[0];
      n->key = p->key;
      erase(n->c[1], p->key);
    }
    maintain(n, d);
    n->update();
  }

  template<class BinaryFunction>
  static void walk(node_t * n, BinaryFunction f) {
    if (n == 0) return;
    walk(n->c[0], f);
    f(n->key, n->val);
    walk(n->c[1], f);
  }

  static std::pair<key_t, val_t> select(node_t *& n, int k) {
    int r = size(n->c[0]);
    if (k < r) return select(n->c[0], k);
    if (k > r) return select(n->c[1], k - r - 1);
    return std::make_pair(n->key, n->val);
  }

  static int rank(node_t * n, const key_t & k) {
    if (n == 0)
      throw std::runtime_error("Cannot rank key not in tree.");
    int r = size(n->c[0]);
    if (k < n->key) return rank(n->c[0], k);
    if (n->key < k) return rank(n->c[1], k) + r + 1;
    return r;
  }

  static void clean_up(node_t * n) {
    if (n == 0) return;
    clean_up(n->c[0]);
    clean_up(n->c[1]);
    delete n;
  }

 public:
  size_balanced_tree() : root(0) {}
  ~size_balanced_tree() { clean_up(root); }
  int size() { return size(root); }
  bool empty() const { return root == 0; }

  void insert(const key_t & key, const val_t & val) {
    insert(root, key, val);
  }

  void erase(const key_t & key) {
    erase(root, key);
  }

  template<class BinaryFunction> void walk(BinaryFunction f) {
    walk(root, f);
  }

  val_t* find(const key_t & key) {
    for (node_t *n = root; n != 0; ) {
      if (n->key == key) return &(n->val);
      n = (key < n->key ? n->c[0] : n->c[1]);
    }
    return 0; //key not found
  }

  std::pair<key_t, val_t> select(int k) {
    if (k >= size(root))
      throw std::runtime_error("k must be smaller size of tree.");
    return select(root, k);
  }

  int rank(const key_t & key) {
    return rank(root, key);
  }
};

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

void printch(int k, char v) { cout << v; }

int main() {
  size_balanced_tree<int, char> T;
  T.insert(2, 'b');
  T.insert(1, 'a');
  T.insert(3, 'c');
  T.insert(5, 'e');
  T.insert(4, 'x');
  *T.find(4) = 'd';
  cout << "In-order: ";
  T.walk(printch);                  //abcde
  T.erase(3);
  cout << "\nRank of 2: " << T.rank(2); //1
  cout << "\nRank of 5: " << T.rank(5); //3
  cout << "\nValue of 3rd smallest key: ";
  cout << T.select(2).second;           //d
  cout << "\n";

  //stress test - runs in <1 second
  //insert keys in an order that would break a normal BST
  size_balanced_tree<int, int> T2;
  for (int i = 0; i < 1000000; i++)
    T2.insert(i, i*1337);
  for (int i = 0; i < 1000000; i++)
    assert(*T2.find(i) == i*1337);
  return 0;
}
