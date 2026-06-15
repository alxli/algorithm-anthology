/*

Maintain an ordered map, that is, an ordered collection of key-value pairs such that each possible
key appears at most once in the collection. This implementation requires an ordering on the set of
possible keys defined by `operator<` on the key type. A binary search tree (a.k.a. BST) implements
this map by inserting and deleting keys into a binary tree while upholding the BST property: for
each node in the BST, every node in its left subtree has a lesser key and every node in its right
subtree has a greater key.

- `BST()` constructs an empty map.
- `size()` returns the size of the map.
- `empty()` returns whether the map is empty.
- `insert(k, v)` adds an entry with key `k` and value `v` to the map, returning `true` if a new
  entry was added or `false` if the key already exists (in which case the map is unchanged and the
  old value associated with the key is preserved).
- `erase(k)` removes the entry with key `k` from the map, returning `true` if the removal was
  successful or `false` if the key to be removed was not found.
- `find(k)` returns a pointer to a const value associated with key `k`, or `nullptr` if the key was
  not found.
- `min()`, `max()`, `lower_bound(k)`, `upper_bound(k)`, `prev(k)`, and `next(k)` return the matching
  key-value entry, or `std::nullopt` if no such entry exists.
- `entries()` returns all key-value entries in ascending order of keys.

The explicit navigation routines above depend only on the BST property and can be copied unchanged
to the other BST variants later in this chapter.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `empty()`.
- O(h) per call to `insert()`, `erase()`, `find()`, `min()`, `max()`, `lower_bound()`,
  `upper_bound()`, `prev()`, and `next()`, where $h$ is the height of the tree. This is O(log n)
  when the tree is balanced, but O(n) in the worst case for this unbalanced implementation.
- O(n) per call to `entries()`, where $n$ is the number of nodes currently in the map.

Space Complexity:
- O(n) for storage of the map elements.
- O(n) auxiliary stack space for `insert()`, `erase()`, and `entries()`.
- O(1) auxiliary for all other operations.

*/

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

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

  static void collect_entries(Node *n, std::vector<std::pair<K, V>> &res) {
    if (n != nullptr) {
      collect_entries(n->left, res);
      res.push_back({n->key, n->value});
      collect_entries(n->right, res);
    }
  }

  static std::optional<std::pair<K, V>> entry(Node *n) {
    if (n == nullptr) {
      return std::nullopt;
    }
    return std::pair<K, V>{n->key, n->value};
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
  BST(const BST &) = delete;
  BST &operator=(const BST &) = delete;
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

  std::optional<std::pair<K, V>> min() const {
    Node *n = root;
    if (n == nullptr) {
      return std::nullopt;
    }
    while (n->left != nullptr) {
      n = n->left;
    }
    return std::pair<K, V>{n->key, n->value};
  }

  std::optional<std::pair<K, V>> max() const {
    Node *n = root;
    if (n == nullptr) {
      return std::nullopt;
    }
    while (n->right != nullptr) {
      n = n->right;
    }
    return std::pair<K, V>{n->key, n->value};
  }

  std::optional<std::pair<K, V>> lower_bound(const K &k) const {
    Node *n = root, *best = nullptr;
    while (n != nullptr) {
      if (!(n->key < k)) {
        best = n;
        n = n->left;
      } else {
        n = n->right;
      }
    }
    return entry(best);
  }

  std::optional<std::pair<K, V>> upper_bound(const K &k) const {
    Node *n = root, *best = nullptr;
    while (n != nullptr) {
      if (k < n->key) {
        best = n;
        n = n->left;
      } else {
        n = n->right;
      }
    }
    return entry(best);
  }

  std::optional<std::pair<K, V>> prev(const K &k) const {
    Node *n = root, *best = nullptr;
    while (n != nullptr) {
      if (n->key < k) {
        best = n;
        n = n->right;
      } else {
        n = n->left;
      }
    }
    return entry(best);
  }

  std::optional<std::pair<K, V>> next(const K &k) const { return upper_bound(k); }

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
  BST<int, char> t;
  t.insert(2, 'b');
  t.insert(1, 'a');
  t.insert(3, 'c');
  t.insert(5, 'e');
  assert(t.insert(4, 'd'));
  assert(*t.find(4) == 'd');
  assert(!t.insert(4, 'd'));
  assert(t.min()->first == 1 && t.min()->second == 'a');
  assert(t.max()->first == 5 && t.max()->second == 'e');
  assert(t.lower_bound(4)->first == 4);
  assert(t.upper_bound(4)->first == 5);
  assert(t.prev(4)->first == 3);
  assert(t.next(4)->first == 5);
  assert(!t.prev(1));
  assert(!t.next(5));
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
