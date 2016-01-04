/*

Description: A binary search tree (BST) is a node-based binary tree data
structure where the left sub-tree of every node has keys less than the
node's key and the right sub-tree of every node has keys greater than the
node's key. A BST may be come degenerate like a linked list resulting in
an O(N) running time per operation. A self-balancing binary search tree
such as a randomized treap prevents the occurence of this known worst case.

Treaps use randomly generated priorities to reduce the height of the
tree. We assume that the rand() function in <cstdlib> is 16-bits, and
call it twice to generate a 32-bit number. For the treap to be
effective, the range of the randomly generated numbers should be
between 0 and around the number of elements in the treap.

Note: The following implementation is used similar to an std::map. In order
to make it behave like an std::set, modify the code to remove the value
associated with each node. In order to make it behave like an std::multiset
or std::multimap, make appropriate changes with key comparisons (e.g.
change (k < n->key) to (k <= n->key) in search conditions).

Time Complexity: insert(), erase(), and find() are O(log(N)) on average
and O(N) in the worst case. Despite the technically O(N) worst case,
such cases are still extremely difficult to trigger, making treaps
very practice in many programming contest applications. walk() is O(N).

Space Complexity: O(N) on the number of nodes.

*/

#include <cstdlib> /* srand(), rand() */
#include <ctime>   /* time() */

template<class key_t, class val_t> class treap {
  struct node_t {
    static inline int rand32() {
      return (rand() & 0x7fff) | ((rand() & 0x7fff) << 15);
    }

    key_t key;
    val_t val;
    int priority;
    node_t *L, *R;

    node_t(const key_t & k, const val_t & v): key(k), val(v), L(0), R(0) {
      priority = rand32();
    }
  } *root;

  int num_nodes;

  static void rotate_l(node_t *& k2) {
    node_t *k1 = k2->R;
    k2->R = k1->L;
    k1->L = k2;
    k2 = k1;
  }

  static void rotate_r(node_t *& k2) {
    node_t *k1 = k2->L;
    k2->L = k1->R;
    k1->R = k2;
    k2 = k1;
  }

  static bool insert(node_t *& n, const key_t & k, const val_t & v) {
    if (n == 0) {
      n = new node_t(k, v);
      return true;
    }
    if (k < n->key && insert(n->L, k, v)) {
      if (n->L->priority < n->priority) rotate_r(n);
      return true;
    } else if (n->key < k && insert(n->R, k, v)) {
      if (n->R->priority < n->priority) rotate_l(n);
      return true;
    }
    return false;
  }

  static bool erase(node_t *& n, const key_t & k) {
    if (n == 0) return false;
    if (k < n->key) return erase(n->L, k);
    if (k > n->key) return erase(n->R, k);
    if (n->L == 0 || n->R == 0) {
      node_t *temp = n;
      n = (n->L != 0) ? n->L : n->R;
      delete temp;
      return true;
    }
    if (n->L->priority < n->R->priority) {
      rotate_r(n);
      return erase(n->R, k);
    }
    rotate_l(n);
    return erase(n->L, k);
  }

  template<class BinaryFunction>
  static void walk(node_t * n, BinaryFunction f) {
    if (n == 0) return;
    walk(n->L, f);
    f(n->key, n->val);
    walk(n->R, f);
  }

  static void clean_up(node_t * n) {
    if (n == 0) return;
    clean_up(n->L);
    clean_up(n->R);
    delete n;
  }

 public:
  treap(): root(0), num_nodes(0) { srand(time(0)); }
  ~treap() { clean_up(root); }
  int size() const { return num_nodes; }
  bool empty() const { return root == 0; }

  bool insert(const key_t & key, const val_t & val) {
    if (insert(root, key, val)) {
      num_nodes++;
      return true;
    }
    return false;
  }

  bool erase(const key_t & key) {
    if (erase(root, key)) {
      num_nodes--;
      return true;
    }
    return false;
  }

  template<class BinaryFunction> void walk(BinaryFunction f) {
    walk(root, f);
  }

  val_t * find(const key_t & key) {
    for (node_t *n = root; n != 0; ) {
      if (n->key == key) return &(n->val);
      n = (key < n->key ? n->L : n->R);
    }
    return 0; //key not found
  }
};

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

void printch(int k, char v) { cout << v; }

int main() {
  treap<int, char> T;
  T.insert(2, 'b');
  T.insert(1, 'a');
  T.insert(3, 'c');
  T.insert(5, 'e');
  T.insert(4, 'x');
  *T.find(4) = 'd';
  cout << "In-order: ";
  T.walk(printch);  //abcde
  cout << "\nRemoving node with key 3...";
  cout << (T.erase(3) ? "Success!" : "Failed");
  cout << "\n";

  //stress test - runs in <0.5 seconds
  //insert keys in an order that would break a normal BST
  treap<int, int> T2;
  for (int i = 0; i < 1000000; i++)
    T2.insert(i, i*1337);
  for (int i = 0; i < 1000000; i++)
    assert(*T2.find(i) == i*1337);
  return 0;
}
