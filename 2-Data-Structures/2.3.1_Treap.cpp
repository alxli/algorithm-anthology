/*

Maintain an ordered map, that is, an ordered collection of key-value pairs such that each possible
key appears at most once in the collection. A binary search tree (a.k.a. BST) implements this map by
inserting and deleting keys into a binary tree while upholding the BST property: for each node in
the BST, every node in its left subtree has a lesser key and every node in its right subtree has a
greater key.

A treap is a binary search tree where each node also holds a randomly assigned priority. The tree
satisfies the BST property on keys and the min-heap property on priorities (lower priority value is
closer to root). Since priorities are random, this keeps the tree balanced with high probability,
making insertions and deletions run in O(log n).

This implementation requires an ordering on the key type `K` defined by `operator<`.

- `Treap<K, V>()` constructs an empty map.
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
- `min()`, `max()`, `lower_bound(k)`, `upper_bound(k)`, `prev(k)`, and `next(k)` return the matching
  key-value entry, or `std::nullopt` if no such entry exists. These navigation routines depend only
  on the BST property and can be copied unchanged to the other BST variants later in this chapter.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `empty()`.
- O(log n) on average per call to `insert()`, `erase()`, `find()`, `min()`, `max()`,
  `lower_bound()`, `upper_bound()`, `prev()`, and `next()`, where $n$ is the number of entries
  currently in the map.
- O(n) per call to `entries()`.

Space Complexity:
- O(n) for storage of the map elements.
- O(log n) auxiliary stack space on average for `insert()`, `erase()`, and `entries()`.
- O(1) auxiliary for all other operations.

*/

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

template<class K, class V>
class Treap {
  struct Node {
    static uint32_t rand32() {
      static uint32_t x = 123456789;
      x ^= x << 13;
      x ^= x >> 17;
      x ^= x << 5;
      return x;
    }

    K key;
    V value;
    uint32_t priority;
    Node *left, *right;

    Node(const K &k, const V &v)
        : key(k), value(v), priority(rand32()), left(nullptr), right(nullptr) {}
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

  bool insert(Node *&n, const K &k, const V &v) {
    if (n == nullptr) {
      n = new Node(k, v);
      num_nodes++;
      return true;
    }
    if (k < n->key && insert(n->left, k, v)) {
      if (n->left->priority < n->priority) {
        rotate_right(n);
      }
      return true;
    }
    if (n->key < k && insert(n->right, k, v)) {
      if (n->right->priority < n->priority) {
        rotate_left(n);
      }
      return true;
    }
    return false;
  }

  bool erase(Node *&n, const K &k) {
    if (n == nullptr) {
      return false;
    }
    if (k < n->key) {
      return erase(n->left, k);
    } else if (n->key < k) {
      return erase(n->right, k);
    }
    if (n->left != nullptr && n->right != nullptr) {
      if (n->left->priority < n->right->priority) {
        rotate_right(n);
        return erase(n->right, k);
      }
      rotate_left(n);
      return erase(n->left, k);
    }
    Node *tmp = (n->left != nullptr) ? n->left : n->right;
    delete n;
    n = tmp;
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
  Treap() : root(nullptr), num_nodes(0) {}

  ~Treap() { clean_up(root); }
  Treap(const Treap &) = delete;
  Treap &operator=(const Treap &) = delete;
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
};

/*** Example Usage and Output:

abcde
bcde

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  Treap<int, char> t;
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
