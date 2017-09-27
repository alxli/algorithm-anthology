/*

Maintain a map, that is, a collection of key-value pairs such that each possible
key appears at most once in the collection. This implementation requires an
ordering on the set of possible keys defined by the < operator on the key type.
An AVL tree is a binary search tree balanced by height, guaranteeing O(log n)
worst-case running time in insertions and deletions by making sure that the
heights of the left and right subtrees at every node differ by at most 1.

- avl_tree() constructs an empty map.
- size() returns the size of the map.
- empty() returns whether the map is empty.
- insert(k, v) adds an entry with key k and value v to the map, returning true
  if an new entry was added or false if the key already exists (in which case
  the map is unchanged and the old value associated with the key is preserved).
- erase(k) removes the entry with key k from the map, returning true if the
  removal was successful or false if the key to be removed was not found.
- find(k) returns a pointer to a const value associated with key k, or NULL if
  the key was not found.
- walk(f) calls the function f(k, v) on each entry of the map, in ascending
  order of keys.

Time Complexity:
- O(1) per call to the constructor, size(), and empty().
- O(log n) per call to insert(), erase(), and find(), where n is the number of
  entries currently in the map.
- O(n) per call to walk().

Space Complexity:
- O(n) for storage of the map elements.
- O(log n) auxiliary stack space for insert(), erase(), and walk().
- O(1) auxiliary for all other operations.

*/

#include <algorithm>
#include <cstdlib>

template<class K, class V>
class avl_tree {
  struct node_t {
    K key;
    V value;
    int height;
    node_t *left, *right;

    node_t(const K &k, const V &v)
        : key(k), value(v), height(1), left(NULL), right(NULL) {}
  } *root;

  int num_nodes;

  static int height(node_t *n) {
    return (n != NULL) ? n->height : 0;
  }

  static void update_height(node_t *n) {
    if (n != NULL) {
      n->height = 1 + std::max(height(n->left), height(n->right));
    }
  }

  static void rotate_left(node_t *&n) {
    node_t *tmp = n;
    n = n->right;
    tmp->right = n->left;
    n->left = tmp;
    update_height(tmp);
    update_height(n);
  }

  static void rotate_right(node_t *&n) {
    node_t *tmp = n;
    n = n->left;
    tmp->left = n->right;
    n->right = tmp;
    update_height(tmp);
    update_height(n);
  }

  static int balance_factor(node_t *n) {
    return (n != NULL) ? (height(n->left) - height(n->right)) : 0;
  }

  static void rebalance(node_t *&n) {
    if (n == NULL) {
      return;
    }
    update_height(n);
    int bf = balance_factor(n);
    if (bf > 1 && balance_factor(n->left) >= 0) {
      rotate_right(n);
    } else if (bf > 1 && balance_factor(n->left) < 0) {
      rotate_left(n->left);
      rotate_right(n);
    } else if (bf < -1 && balance_factor(n->right) <= 0) {
      rotate_left(n);
    } else if (bf < -1 && balance_factor(n->right) > 0) {
      rotate_right(n->right);
      rotate_left(n);
    }
  }

  static bool insert(node_t *&n, const K &k, const V &v) {
    if (n == NULL) {
      n = new node_t(k, v);
      return true;
    }
    if ((k < n->key && insert(n->left, k, v)) ||
        (n->key < k && insert(n->right, k, v))) {
      rebalance(n);
      return true;
    }
    return false;
  }

  static bool erase(node_t *&n, const K &k) {
    if (n == NULL) {
      return false;
    }
    if (!(k < n->key || n->key < k)) {
      if (n->left != NULL && n->right != NULL) {
        node_t *tmp = n->right, *parent = NULL;
        while (tmp->left != NULL) {
          parent = tmp;
          tmp = tmp->left;
        }
        n->key = tmp->key;
        n->value = tmp->value;
        if (parent != NULL) {
          if (!erase(parent->left, parent->left->key)) {
            return false;
          }
        } else if (!erase(n->right, n->right->key)) {
          return false;
        }
      } else {
        node_t *tmp = (n->left != NULL) ? n->left : n->right;
        delete n;
        n = tmp;
      }
      rebalance(n);
      return true;
    }
    if ((k < n->key && erase(n->left, k)) ||
        (n->key < k && erase(n->right, k))) {
      rebalance(n);
      return true;
    }
    return false;
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
  avl_tree() : root(NULL), num_nodes(0) {}

  ~avl_tree() {
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

  const V* find(const K &k) const {
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

  template<class KVFunction>
  void walk(KVFunction f) const {
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
  avl_tree<int, char> t;
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
