/*

3.4.1 - Binary Search Tree (Simple)

Description: Description: A binary search tree (BST) is a node-based
binary tree data structure where the left sub-tree of every node has
keys less than the node’s key and the right sub-tree of every node
has keys greater (greater or equal in this implementation) than the
node’s key. A BST may be come degenerate like a linked list resulting
in an O(N) running time per operation. A self-balancing binary search
tree such as a randomized Treap, or a more complicated AVL tree solves
this problem with a worst case of O(log(N)).

Time Complexity: insert(), remove() and find() are O(log(N)) on
average, but O(N) at worst if the tree becomes degenerate. Speed
can be improved by randomizing insertion order. walk() is O(N).
All other functions are O(1).

Space Complexity: O(N) on the number of nodes.

*/

template<class key_t, class val_t> class binary_search_tree {
  struct node_t {
    key_t key;
    val_t val;
    node_t *L, *R;
  } *root;

  int num_nodes;
 
  void internal_insert(node_t *& n, const key_t & k, const val_t & v) {
    if (n == 0) {
      n = new node_t();
      n->key = k;
      n->val = v;
      num_nodes++;
    } else {
      internal_insert(k < n->key ? n->L : n->R, k, v);
    }
  }
  
  /* returns whether the key was successfully removed */
  bool internal_remove(node_t *& n, const key_t & key) {
    if (n == 0) return false;
    if (key < n->key) return internal_remove(n->L, key);
    if (n->key < key) return internal_remove(n->R, key);
    if (n->L == 0) {
      node_t *temp = n->R;
      delete n;
      n = temp;
    } else if (n->R == 0) {
      node_t *temp = n->L;
      delete n;
      n = temp;
    } else {
      node_t *temp = n->R, *parent = 0;
      while (temp->L != 0) {
        parent = temp;
        temp = temp->L;
      }
      n->key = temp->key;
      n->val = temp->val;
      if (parent != 0)
        return internal_remove(parent->L, parent->L->key);
      return internal_remove(n->R, n->R->key);
    }
    num_nodes--;
    return true;
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
  binary_search_tree(): root(0), num_nodes(0) {}
  ~binary_search_tree() { clean_up(root); }
  int size() const { return num_nodes; }
  bool empty() const { return root == 0; }
  
  void insert(const key_t & key, const val_t & val) {
    internal_insert(root, key, val);
  }
  
  bool remove(const key_t & key) {
    return internal_remove(root, key);
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
    return 0; /* key not found */
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

void printch(char c) { cout << c; }

int main() {
  binary_search_tree<int, char> T;
  T.insert(2, 'b');
  T.insert(1, 'a');
  T.insert(3, 'c');
  T.insert(5, 'e');
  T.insert(4, 'x');
  *T.find(4) = 'd';
  cout << "In-order: ";
  T.walk(printch, 0); //abcde
  cout << "\nRemoving node with key 3...";
  cout << (T.remove(3) ? "Success!" : "Failed");
  cout << "\nPre-order: ";
  T.walk(printch, -1); //baed
  cout << "\nPost-order: ";
  T.walk(printch, 1); //adeb
  return 0;
}
