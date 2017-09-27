/*

Maintain a map, that is, a collection of key-value pairs such that each possible
key appears at most once in the collection. This implementation requires an
ordering on the set of possible keys defined by the < operator on the key type.
A splay tree is a balanced binary search tree with the additional property that
recently accessed elements are quick to access again.

- splay_tree() constructs an empty map.
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

#include <cstdlib>

template<class K, class V>
class splay_tree {
  struct node_t {
    K key;
    V value;
    node_t *left, *right;

    node_t(const K &k, const V &v)
        : key(k), value(v), left(NULL), right(NULL) {}
  } *root;

  int num_nodes;

  static void rotate_left(node_t *&n) {
    node_t *tmp = n;
    n = n->right;
    tmp->right = n->left;
    n->left = tmp;
  }

  static void rotate_right(node_t *&n) {
    node_t *tmp = n;
    n = n->left;
    tmp->left = n->right;
    n->right = tmp;
  }

  static void splay(node_t *&n, const K &k) {
    if (n == NULL) {
      return;
    }
    if (k < n->key && n->left != NULL) {
      if (k < n->left->key) {
        splay(n->left->left, k);
        rotate_right(n);
      } else if (n->left->key < k) {
        splay(n->left->right, k);
        if (n->left->right != NULL) {
          rotate_left(n->left);
        }
      }
      if (n->left != NULL) {
        rotate_right(n);
      }
    } else if (n->key < k && n->right != NULL) {
      if (k < n->right->key) {
        splay(n->right->left, k);
        if (n->right->left != NULL) {
          rotate_right(n->right);
        }
      } else if (n->right->key < k) {
        splay(n->right->right, k);
        rotate_left(n);
      }
      if (n->right != NULL) {
        rotate_left(n);
      }
    }
  }

  static bool insert(node_t *&n, const K &k, const V &v) {
    if (n == NULL) {
      n = new node_t(k, v);
      return true;
    }
    splay(n, k);
    if (k < n->key) {
      node_t *tmp = new node_t(k, v);
      tmp->left = n->left;
      tmp->right = n;
      n->left = NULL;
      n = tmp;
    } else if (n->key < k) {
      node_t *tmp = new node_t(k, v);
      tmp->left = n;
      tmp->right = n->right;
      n->right = NULL;
      n = tmp;
    } else {
      return false;
    }
    return true;
  }

  static bool erase(node_t *&n, const K &k) {
    if (n == NULL) {
      return false;
    }
    splay(n, k);
    if (k < n->key || n->key < k) {
      return false;
    }
    node_t *tmp = n;
    if (n->left == NULL) {
      n = n->right;
    } else {
      splay(n->left, k);
      n = n->left;
      n->right = tmp->right;
    }
    delete tmp;
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
  splay_tree() : root(NULL), num_nodes(0) {}

  ~splay_tree() {
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
    splay(root, k);
    return (k < root->key || root->key < k) ? NULL : &(root->value);
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
  splay_tree<int, char> t;
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
