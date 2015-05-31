/*

3.4.1 - Treap

Description: Description: A binary search tree (BST) is a node-based
binary tree data structure where the left sub-tree of every node has
keys less than the node's key and the right sub-tree of every node
has keys greater (greater or equal in this implementation) than the
node’s key. A BST may be come degenerate like a linked list resulting
in an O(N) running time per operation.

Treaps use randomly generated priorities to reduce the height of the
tree. We assume that the rand() function in <cstdlib> is 16-bits, and
call it twice to generate a 32-bit number. For the treap to be
effective, the range of the randomly generated numbers should be
between 0 and around the number of elements in the treap.

Time Complexity: insert(), remove() and find() are O(log(N)) amortized
in the worst case. walk() is O(N).

Space Complexity: O(N) on the number of nodes.

*/

#include <cstdlib> /* srand(), rand() */
#include <ctime>   /* time() */

template<class key_t, class val_t> class treap {

  //returns a random integer between l and h, inclusive
  static inline int rand_int32(int l, int h) {
    return l + ((rand()&0x7fff) | ((rand()&0x7fff) << 15)) % (h - l + 1);
  }

  struct node_t {
    key_t key;
    val_t val;
    int priority;
    node_t *L, *R;
    
    node_t(const key_t & k, const val_t & v): key(k), val(v), L(0), R(0) {
      priority = rand_int32(0, 1 << 30);
    }
  } *root;

  int num_nodes;

  void rotateL(node_t *& k2) {
    node_t *k1 = k2->R; 
    k2->R = k1->L;
    k1->L = k2;
    k2 = k1;
  }

  void rotateR(node_t *& k2) {
    node_t *k1 = k2->L; 
    k2->L = k1->R;
    k1->R = k2;
    k2 = k1;
  }

  void internal_insert(node_t *& n, const key_t & k, const val_t & v) {
    node_t * p_node = new node_t(k, v);
    if (n == 0) {
      n = p_node;
      num_nodes++;
      return;
    }
    if (k < n->key) {
      internal_insert(n->L, k, v);
      if (n->L->priority < n->priority) rotateR(n);
    } else {
      internal_insert(n->R, k, v);
      if (n->R->priority < n->priority) rotateL(n);
    }
  }

  bool internal_remove(node_t *& n, const key_t & k) {
    if (n == 0) return false;
    if (k < n->key) return internal_remove(n->L, k);
    if (k > n->key) return internal_remove(n->R, k);
    if (n->L == 0 || n->R == 0) {
      node_t *temp = n;
      n = (n->L != 0) ? n->L : n->R;
      delete temp;
      num_nodes--;
      return true;
    }
    if (n->L->priority < n->R->priority) {
      rotateR(n);
      return internal_remove(n->R, k);
    }
    rotateL(n);
    return internal_remove(n->L, k);
  }

  void clean_up(node_t *& n) {
    if (n == 0) return;
    clean_up(n->L);
    clean_up(n->R);
    delete n;
  }

  template<class UnaryFunction>
  void internal_walk(node_t * n, UnaryFunction f, int order) {
    if (n == 0) return;
    if (order < 0) (*f)(n->val);
    if (n->L) internal_walk(n->L, f, order);
    if (order == 0) (*f)(n->val);
    if (n->R) internal_walk(n->R, f, order); 
    if (order > 0) (*f)(n->val);
  }

 public:
  treap(): root(0), num_nodes(0) { srand(time(0)); }
  ~treap() { clean_up(root); }
  int size() const { return num_nodes; }
  bool empty() const { return root == 0; }

  void insert(const key_t & k, const val_t & v) {
    internal_insert(root, k, v);
  }

  bool remove(const key_t & k) {
    return internal_remove(root, k);
  }

  //traverses nodes in either preorder (-1), inorder (0), or postorder (1)
  //for each node, the passed unary function will be called on its value
  //note: inorder is equivalent to visiting the nodes sorted by their keys.
  template<class UnaryFunction> void walk(UnaryFunction f, int order = 0) {
    internal_walk(root, f, order);
  }

  val_t* find(const key_t & key) {
    for (node_t *n = root; n != 0; ) {
      if (n->key == key) return &(n->val);
      n = (key < n->key ? n->L : n->R);
    }
    return 0; //key not found
  }
};

/*** Stress Test - Runs in <3 seconds ***/

#include <cassert>

int main() {
  treap<int, int> T;
  //insert keys in an order that would break a normal BST
  for (int i = 0; i < 1000000; i++) {
    T.insert(i, i + i);
  }
  for (int i = 0; i < 1000000; i++) {
    assert(*T.find(i) == i + i);
  }
  return 0;
}
