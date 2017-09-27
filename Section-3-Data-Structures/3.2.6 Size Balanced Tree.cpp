/*

Maintain a map, that is, a collection of key-value pairs such that each possible
key appears at most once in the collection. In addition, support queries for
keys given their ranks as well as queries for the ranks of given keys. This
implementation requires an ordering on the set of possible keys defined by the
< operator on the key type. A size balanced tree augments each nodes with the
size of its subtree, using it to maintain balance and compute order statistics.

- size_balanced_tree() constructs an empty map.
- size() returns the size of the map.
- empty() returns whether the map is empty.
- insert(k, v) adds an entry with key k and value v to the map, returning true
  if an new entry was added or false if the key already exists (in which case
  the map is unchanged and the old value associated with the key is preserved).
- erase(k) removes the entry with key k from the map, returning true if the
  removal was successful or false if the key to be removed was not found.
- find(k) returns a pointer to a const value associated with key k, or NULL if
  the key was not found.
- select(r) returns a key-value pair of the node with a key of zero-based rank r
  in the map, throwing an exception if the rank is not between 0 and size() - 1.
- rank(k) returns the zero-based rank of key k in the map, throwing an
  exception if the key was not found in the map.
- walk(f) calls the function f(k, v) on each entry of the map, in ascending
  order of keys.

Time Complexity:
- O(1) per call to the constructor, size(), and empty().
- O(log n) per call to insert(), erase(), find(), select(), and rank(), where n
  is the number of entries currently in the map.
- O(n) per call to walk().

Space Complexity:
- O(n) for storage of the map elements.
- O(log n) auxiliary stack space for insert(), erase(), and walk().
- O(1) auxiliary for all other operations.

*/

#include <cstdlib>
#include <stdexcept>
#include <utility>

template<class K, class V>
class size_balanced_tree {
  struct node_t {
    K key;
    V value;
    int size;
    node_t *left, *right;

    node_t(const K &k, const V &v)
        : key(k), value(v), size(1), left(NULL), right(NULL) {}

    inline node_t*& child(int c) {
      return (c == 0) ? left : right;
    }

    void update() {
      size = 1;
      if (left != NULL) {
        size += left->size;
      }
      if (right != NULL) {
        size += right->size;
      }
    }
  } *root;

  static inline int size(node_t *n) {
    return (n == NULL) ? 0 : n->size;
  }

  static void rotate(node_t *&n, int c) {
    node_t *tmp = n->child(c);
    n->child(c) = tmp->child(!c);
    tmp->child(!c) = n;
    n->update();
    tmp->update();
    n = tmp;
  }

  static void maintain(node_t *&n, int c) {
    if (n == NULL || n->child(c) == NULL) {
      return;
    }
    node_t *&tmp = n->child(c);
    if (size(tmp->child(c)) > size(n->child(!c))) {
      rotate(n, c);
    } else if (size(tmp->child(!c)) > size(n->child(!c))) {
      rotate(tmp, !c);
      rotate(n, c);
    } else {
      return;
    }
    maintain(n->left, 0);
    maintain(n->right, 1);
    maintain(n, 0);
    maintain(n, 1);
  }

  static bool insert(node_t *&n, const K &k, const V &v) {
    if (n == NULL) {
      n = new node_t(k, v);
      return true;
    }
    bool result;
    if (k < n->key) {
      result = insert(n->left, k, v);
      maintain(n, 0);
    } else if (n->key < k) {
      result = insert(n->right, k, v);
      maintain(n, 1);
    } else {
      return false;
    }
    n->update();
    return result;
  }

  static bool erase(node_t *&n, const K &k) {
    if (n == NULL) {
      return false;
    }
    bool result;
    int c = (k < n->key);
    if (k < n->key) {
      result = erase(n->left, k);
    } else if (n->key < k) {
      result = erase(n->right, k);
    } else {
      if (n->right == NULL || n->left == NULL) {
        node_t *tmp = n;
        n = (n->right == NULL) ? n->left : n->right;
        delete tmp;
        return true;
      }
      node_t *p = n->right;
      while (p->left != NULL) {
        p = p->left;
      }
      n->key = p->key;
      result = erase(n->right, p->key);
    }
    maintain(n, c);
    n->update();
    return result;
  }

  static std::pair<K, V> select(node_t *n, int r) {
    int rank = size(n->left);
    if (r < rank) {
      return select(n->left, r);
    } else if (r > rank) {
      return select(n->right, r - rank - 1);
    }
    return std::make_pair(n->key, n->value);
  }

  static int rank(node_t *n, const K &k) {
    if (n == NULL) {
      throw std::runtime_error("Cannot rank key that's not in tree.");
    }
    int r = size(n->left);
    if (k < n->key) {
      return rank(n->left, k);
    } else if (n->key < k) {
      return rank(n->right, k) + r + 1;
    }
    return r;
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
  size_balanced_tree() : root(NULL) {}

  ~size_balanced_tree() {
    clean_up(root);
  }

  int size() const {
    return size(root);
  }

  bool empty() const {
    return root == NULL;
  }

  bool insert(const K &k, const V &v) {
    return insert(root, k, v);
  }

  bool erase(const K &k) {
    return erase(root, k);
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

  std::pair<K, V> select(int r) const {
    if (r < 0 || r >= size(root)) {
      throw std::runtime_error("Select rank must be between 0 and size() - 1.");
    }
    return select(root, r);
  }

  int rank(const K &k) const {
    return rank(root, k);
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
  size_balanced_tree<int, char> t;
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
  assert(t.rank(2) == 0);
  assert(t.rank(3) == 1);
  assert(t.rank(5) == 3);
  assert(t.select(0).first == 2);
  assert(t.select(1).first == 3);
  assert(t.select(2).first == 4);
  return 0;
}
