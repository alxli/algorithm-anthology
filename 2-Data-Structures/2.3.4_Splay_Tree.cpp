/*

Maintain an ordered map, that is, an ordered collection of key-value pairs such that each possible
key appears at most once in the collection. A splay tree is a balanced binary search tree with the
additional property that recently accessed elements are quick to access again. Every operation
"splays" its target node up to the root through a series of rotations, reshaping the tree so that
frequently accessed keys settle near the top. A single operation may degrade to O(n), but any
sequence of operations averages out to O(log n) amortized each.

This implementation requires an ordering on the key type `K` defined by `operator<`.

- `SplayTree<K, V>()` constructs an empty map.
- `size()` returns the size of the map.
- `empty()` returns whether the map is empty.
- `insert(k, v)` adds an entry with key `k` and value `v` to the map, returning `true` if a new
  entry was added or `false` if the key already exists (in which case the map is unchanged and the
  old value associated with the key is preserved).
- `erase(k)` removes the entry with key `k` from the map, returning `true` if the removal was
  successful or `false` if the key to be removed was not found.
- `find(k)` returns a pointer to a const value associated with key `k`, or `nullptr` if the key was
  not found.
- `entries()` returns all key-value entries in ascending order of keys.

The navigation routines `min()`, `max()`, `lower_bound(k)`, `upper_bound(k)`, `prev(k)`, and
`next(k)` from the treap in 2.3.1 depend only on the BST property and may be copied here unchanged.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `empty()`.
- O(log n) amortized per call to `insert()`, `erase()`, and `find()`, where $n$ is the number of
  entries currently in the map.
- O(n) per call to `entries()`.

Space Complexity:
- O(n) for storage of the map elements.
- O(log n) auxiliary stack space for `insert()`, `erase()`, and `entries()`.
- O(1) auxiliary for all other operations.

*/

#include <cstddef>
#include <utility>
#include <vector>

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

  bool insert(Node *&n, const K &k, const V &v) {
    if (n == nullptr) {
      n = new Node(k, v);
      num_nodes++;
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

  bool erase(Node *&n, const K &k) {
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
    num_nodes--;
    return true;
  }

  static void collect_entries(Node *n, std::vector<std::pair<K, V>> &res) {
    if (n != nullptr) {
      collect_entries(n->left, res);
      res.push_back({n->key, n->value});
      collect_entries(n->right, res);
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
  bool insert(const K &k, const V &v) { return insert(root, k, v); }
  bool erase(const K &k) { return erase(root, k); }

  const V *find(const K &k) {
    splay(root, k);
    if (root == nullptr) {
      return nullptr;
    }
    return (k < root->key || root->key < k) ? nullptr : &(root->value);
  }

  std::vector<std::pair<K, V>> entries() const {
    std::vector<std::pair<K, V>> res;
    res.reserve(num_nodes);
    collect_entries(root, res);
    return res;
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
  for (const auto &[k, v] : t.entries()) {
    cout << v;
  }
  cout << endl;
  assert(t.erase(1));
  assert(!t.erase(1));
  assert(t.find(1) == nullptr);
  for (const auto &[k, v] : t.entries()) {
    cout << v;
  }
  cout << endl;
  return 0;
}
