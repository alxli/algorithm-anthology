/*

Maintain an ordered map, that is, an ordered collection of key-value pairs such that each possible
key appears at most once in the collection. An AVL tree is a binary search tree balanced by height,
guaranteeing O(log n) worst-case running time in insertions and deletions by making sure that the
heights of the left and right subtrees at every node differ by at most $1$. Whenever an insertion or
deletion breaks this invariant, it is repaired with one or two rotations at each affected node along
the search path.

This implementation requires an ordering on the key type `K` defined by `operator<`.

- `AVLTree<K, V>()` constructs an empty map.
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
- O(log n) per call to `insert()`, `erase()`, and `find()`, where $n$ is the number of entries
  currently in the map.
- O(n) per call to `entries()`.

Space Complexity:
- O(n) for storage of the map elements.
- O(log n) auxiliary stack space for `insert()`, `erase()`, and `entries()`.
- O(1) auxiliary for all other operations.

*/

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

template<typename K, typename V>
class AVLTree {
  struct Node {
    K key;
    V value;
    int height;
    Node *left, *right;

    Node(const K &k, const V &v) : key(k), value(v), height(1), left(nullptr), right(nullptr) {}
  } *root;

  int num_nodes;

  static int height(Node *n) { return (n != nullptr) ? n->height : 0; }

  static void update_height(Node *n) {
    if (n != nullptr) {
      n->height = 1 + std::max(height(n->left), height(n->right));
    }
  }

  static void rotate_left(Node *&n) {
    Node *tmp = n;
    n = n->right;
    tmp->right = n->left;
    n->left = tmp;
    update_height(tmp);
    update_height(n);
  }

  static void rotate_right(Node *&n) {
    Node *tmp = n;
    n = n->left;
    tmp->left = n->right;
    n->right = tmp;
    update_height(tmp);
    update_height(n);
  }

  static int balance_factor(Node *n) {
    return (n != nullptr) ? (height(n->left) - height(n->right)) : 0;
  }

  static void rebalance(Node *&n) {
    if (n == nullptr) {
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

  bool insert(Node *&n, const K &k, const V &v) {
    if (n == nullptr) {
      n = new Node(k, v);
      num_nodes++;
      return true;
    }
    if ((k < n->key && insert(n->left, k, v)) || (n->key < k && insert(n->right, k, v))) {
      rebalance(n);
      return true;
    }
    return false;
  }

  bool erase(Node *&n, const K &k) {
    if (n == nullptr) {
      return false;
    }
    if (!(k < n->key || n->key < k)) {
      if (n->left != nullptr && n->right != nullptr) {
        Node *tmp = n->right, *parent = nullptr;
        while (tmp->left != nullptr) {
          parent = tmp;
          tmp = tmp->left;
        }
        n->key = tmp->key;
        n->value = tmp->value;
        if (parent != nullptr) {
          if (!erase(parent->left, parent->left->key)) {
            return false;
          }
        } else if (!erase(n->right, n->right->key)) {
          return false;
        }
      } else {
        Node *tmp = (n->left != nullptr) ? n->left : n->right;
        delete n;
        n = tmp;
        num_nodes--;
      }
      rebalance(n);
      return true;
    }
    if ((k < n->key && erase(n->left, k)) || (n->key < k && erase(n->right, k))) {
      rebalance(n);
      return true;
    }
    return false;
  }

  static void collect_entries(Node *n, std::vector<std::pair<K, V>> &res) {
    if (n != nullptr) {
      collect_entries(n->left, res);
      res.emplace_back(n->key, n->value);
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
  AVLTree() : root(nullptr), num_nodes(0) {}

  ~AVLTree() { clean_up(root); }
  AVLTree(const AVLTree &) = delete;
  AVLTree &operator=(const AVLTree &) = delete;
  int size() const { return num_nodes; }
  bool empty() const { return root == nullptr; }
  bool insert(const K &k, const V &v) { return insert(root, k, v); }
  bool erase(const K &k) { return erase(root, k); }

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
  AVLTree<int, char> t;
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
