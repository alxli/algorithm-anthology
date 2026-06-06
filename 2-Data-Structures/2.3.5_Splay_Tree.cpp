/*

Maintain a map, that is, a collection of key-value pairs such that each possible key appears at most
once in the collection. This implementation requires an ordering on the set of possible keys defined
by `operator<` on the key type. A splay tree is a balanced binary search tree with the additional
property that recently accessed elements are quick to access again.

- `SplayTree()` constructs an empty map.
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
- O(log n) per call to `insert()`, `erase()`, and `find()`, where $n$ is the number of entries
  currently in the map.
- O(n) per call to `walk()`.

Space Complexity:
- O(n) for storage of the map elements.
- O(log n) auxiliary stack space for `insert()`, `erase()`, and `walk()`.
- O(1) auxiliary for all other operations.

*/

#include <cstddef>

template<class K, class V>
class SplayTree {
  struct Node {
    K key;
    V value;
    Node *left, *right;

    Node(const K &k, const V &v) : key(k), value(v), left(nullptr), right(nullptr) {}
  } *root;

  int num_nodes;

  static void rotate_left(Node *&n) {
    Node *tmp = n;
    n = n->right;
    tmp->right = n->left;
    n->left = tmp;
  }

  static void rotate_right(Node *&n) {
    Node *tmp = n;
    n = n->left;
    tmp->left = n->right;
    n->right = tmp;
  }

  static void splay(Node *&n, const K &k) {
    if (n == nullptr) {
      return;
    }
    if (k < n->key && n->left != nullptr) {
      if (k < n->left->key) {
        splay(n->left->left, k);
        rotate_right(n);
      } else if (n->left->key < k) {
        splay(n->left->right, k);
        if (n->left->right != nullptr) {
          rotate_left(n->left);
        }
      }
      if (n->left != nullptr) {
        rotate_right(n);
      }
    } else if (n->key < k && n->right != nullptr) {
      if (k < n->right->key) {
        splay(n->right->left, k);
        if (n->right->left != nullptr) {
          rotate_right(n->right);
        }
      } else if (n->right->key < k) {
        splay(n->right->right, k);
        rotate_left(n);
      }
      if (n->right != nullptr) {
        rotate_left(n);
      }
    }
  }

  static bool insert(Node *&n, const K &k, const V &v) {
    if (n == nullptr) {
      n = new Node(k, v);
      return true;
    }
    splay(n, k);
    if (k < n->key) {
      Node *tmp = new Node(k, v);
      tmp->left = n->left;
      tmp->right = n;
      n->left = nullptr;
      n = tmp;
    } else if (n->key < k) {
      Node *tmp = new Node(k, v);
      tmp->left = n;
      tmp->right = n->right;
      n->right = nullptr;
      n = tmp;
    } else {
      return false;
    }
    return true;
  }

  static bool erase(Node *&n, const K &k) {
    if (n == nullptr) {
      return false;
    }
    splay(n, k);
    if (k < n->key || n->key < k) {
      return false;
    }
    Node *tmp = n;
    if (n->left == nullptr) {
      n = n->right;
    } else {
      splay(n->left, k);
      n = n->left;
      n->right = tmp->right;
    }
    delete tmp;
    return true;
  }

  template<class Fn>
  static void walk(Node *n, Fn f) {
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
  SplayTree() : root(nullptr), num_nodes(0) {}

  ~SplayTree() { clean_up(root); }
  SplayTree(const SplayTree &) = delete;
  SplayTree &operator=(const SplayTree &) = delete;
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

  const V *find(const K &k) {
    splay(root, k);
    return (k < root->key || root->key < k) ? nullptr : &(root->value);
  }

  template<class Fn>
  void walk(Fn f) const {
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
  SplayTree<int, char> t;
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
