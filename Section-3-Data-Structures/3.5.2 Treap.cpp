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

#include <cstdlib>  // srand(), rand()
#include <ctime>  // time()

template<class K, class V> class treap {
  struct node_t {
    K key;
    V value;
    node_t *left, *right;
    int priority;

    node_t(const K &k, const V &v) {
      key = k;
      value = v;
      left = right = NULL;
      priority = (rand() & 0x7fff) | ((rand() & 0x7fff) << 15);
    }
  } *root;

  int num_nodes;

  static void rotate_l(node_t *&k2) {
    node_t *k1 = k2->right;
    k2->right = k1->left;
    k1->left = k2;
    k2 = k1;
  }

  static void rotate_r(node_t *&k2) {
    node_t *k1 = k2->left;
    k2->left = k1->right;
    k1->right = k2;
    k2 = k1;
  }

  static bool insert(node_t *&n, const K &k, const V &v) {
    if (n == NULL) {
      n = new node_t(k, v);
      return true;
    }
    if (k < n->key && insert(n->left, k, v)) {
      if (n->left->priority < n->priority) {
        rotate_r(n);
      }
      return true;
    }
    if (n->key < k && insert(n->right, k, v)) {
      if (n->right->priority < n->priority) {
        rotate_l(n);
      }
      return true;
    }
    return false;
  }

  static bool erase(node_t *&n, const K &k) {
    if (n == NULL) {
      return false;
    }
    if (k < n->key) {
      return erase(n->left, k);
    } else if (n->key < k) {
      return erase(n->right, k);
    }
    if (n->left != NULL && n->right != NULL) {
      if (n->left->priority < n->right->priority) {
        rotate_r(n);
        return erase(n->right, k);
      }
      rotate_l(n);
      return erase(n->left, k);
    }
    node_t *tmp = (n->left != NULL) ? n->left : n->right;
    delete n;
    n = tmp;
    return true;
  }

  template<class KVFunction>
  static void walk(node_t *n, KVFunction f) {
    if (n != NULL) {
      walk(n->left, f);
      f(n->key, n->value);
      walk(n->right, f);
    }
  }

  static void clean_up(node_t *n) {
    if (n != NULL) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  treap() {
    root = NULL;
    num_nodes = 0;
    srand(time(NULL));
  }

  ~treap() {
    clean_up(root);
  }

  int size() const {
    return num_nodes;
  }

  bool empty() const {
    return root == NULL;
  }

  bool insert(const K &k, const V &v) {
    if (insert(root, k, v)) {
      num_nodes++;
      return true;
    }
    return false;
  }

  bool erase(const K &k) {
    if (erase(root, k)) {
      num_nodes--;
      return true;
    }
    return false;
  }

  const V* find(const K &k) {
    node_t *n = root;
    while (n != NULL) {
      if (k < n->key) {
        n = n->left;
      } else if (n->key < k) {
        n = n->right;
      } else {
        return &(n->value);
      }
    }
    return NULL;
  }

  template<class KVFunction> void walk(KVFunction f) {
    walk(root, f);
  }
};

/*** Example Usage and Output:

abcde
bcde

***/

#include <cassert>
#include <iostream>
using namespace std;

void printch(int k, char v) {
  cout << v;
}

int main() {
  treap<int, char> t;
  t.insert(2, 'b');
  t.insert(1, 'a');
  t.insert(3, 'c');
  t.insert(5, 'e');
  assert(t.insert(4, 'd'));
  assert(*t.find(4) == 'd');
  assert(!t.insert(4, 'd'));
  t.walk(printch);
  cout << endl;
  assert(t.erase(1));
  assert(!t.erase(1));
  assert(t.find(1) == NULL);
  t.walk(printch);
  cout << endl;
  return 0;
}

