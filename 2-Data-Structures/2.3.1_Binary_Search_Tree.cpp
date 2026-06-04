/*

Maintain a map, that is, a collection of key-value pairs such that each possible key appears at most
once in the collection. This implementations requires an ordering on the set of possible keys
defined by `operator <` on the key type. A binary search tree implements this map by inserting and
deleting keys into a binary tree such that every node's left child has a lesser key and every node's
right child has a greater key.

- `BST()` constructs an empty map.
- `size()` returns the size of the map.
- `empty()` returns whether the map is empty.
- `insert(k, v)` adds an entry with key `k` and value `v` to the map, returning `true` if an new
  entry was added or `false` if the key already exists (in which case the map is unchanged and the
  old value associated with the key is preserved).
- `erase(k)` removes the entry with key `k` from the map, returning `true` if the removal was
  successful or `false` if the key to be removed was not found.
- `find(k)` returns a pointer to a const value associated with key `k`, or `nullptr` if the key was not
  found.
- `walk(f)` calls the function `f(k, v)` on each entry of the map, in ascending order of keys.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `empty()`.
- O(n) per call to `insert()`, `erase()`, `find()`, and `walk()`, where $n$ is the number of nodes
  currently in the map.

Space Complexity:
- O(n) for storage of the map elements.
- O(n) auxiliary stack space for `insert()`, `erase()`, and `walk()`.
- O(1) auxiliary for all other operations.

*/

#include <cstddef>

template<class K, class V>
class BST {
  struct Node {
    K key;
    V value;
    Node *left, *right;

    Node(const K &k, const V &v) : key(k), value(v), left(nullptr), right(nullptr) {}
  } *root;

  int num_nodes;

  static bool insert(Node *&n, const K &k, const V &v) {
    if (n == nullptr) {
      n = new Node(k, v);
      return true;
    }
    if (k < n->key) {
      return insert(n->left, k, v);
    } else if (n->key < k) {
      return insert(n->right, k, v);
    }
    return false;
  }

  static bool erase(Node *&n, const K &k) {
    if (n == nullptr) {
      return false;
    }
    if (k < n->key) {
      return erase(n->left, k);
    } else if (n->key < k) {
      return erase(n->right, k);
    }
    if (n->left != nullptr && n->right != nullptr) {
      Node *tmp = n->right, *parent = nullptr;
      while (tmp->left != nullptr) {
        parent = tmp;
        tmp = tmp->left;
      }
      n->key = tmp->key;
      n->value = tmp->value;
      if (parent != nullptr) {
        return erase(parent->left, parent->left->key);
      }
      return erase(n->right, n->right->key);
    }
    Node *tmp = (n->left != nullptr) ? n->left : n->right;
    delete n;
    n = tmp;
    return true;
  }

  template<class KVFunction>
  static void walk(Node *n, KVFunction f) {
    if (n != nullptr) {
      walk(n->left, f);
      f(n->key, n->value);
      walk(n->right, f);
    }
  }

  static void clean_up(Node *n) {
    if (n != nullptr) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  BST() : root(nullptr), num_nodes(0) {}

  ~BST() { clean_up(root); }
  int size() const { return num_nodes; }
  bool empty() const { return root == nullptr; }

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

  const V *find(const K &k) const {
    Node *n = root;
    while (n != nullptr) {
      if (k < n->key) {
        n = n->left;
      } else if (n->key < k) {
        n = n->right;
      } else {
        return &(n->value);
      }
    }
    return nullptr;
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

int main() {
  BST<int, char> t;
  t.insert(2, 'b');
  t.insert(1, 'a');
  t.insert(3, 'c');
  t.insert(5, 'e');
  assert(t.insert(4, 'd'));
  assert(*t.find(4) == 'd');
  assert(!t.insert(4, 'd'));
  auto printch = [](int k, char v) { cout << v; };
  t.walk(printch);
  cout << endl;
  assert(t.erase(1));
  assert(!t.erase(1));
  assert(t.find(1) == nullptr);
  t.walk(printch);
  cout << endl;
  return 0;
}
