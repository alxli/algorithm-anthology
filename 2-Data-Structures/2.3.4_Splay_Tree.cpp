/*

Maintain an ordered map, that is, an ordered collection of key-value pairs such that each possible
key appears at most once in the collection. A splay tree is a balanced binary search tree with the
additional property that recently accessed elements are quick to access again. Every operation
"splays" its target node up to the root through a series of rotations, reshaping the tree so that
frequently accessed keys settle near the top. A single operation may degrade to O(n), but any
sequence of operations averages out to O(log n) amortized each.

The comparator `comp` defines the key ordering: `comp(a, b)` is true when `a` precedes `b`. It
defaults to `std::less<K>`; to customize the ordering, instantiate `SplayTree<K, V, Compare>` and
pass the comparator to the constructor.

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
- `entries()` returns all key-value entries in comparator order.

The comparator-aware navigation routines `min()`, `max()`, `lower_bound(k)`, `upper_bound(k)`,
`prev(k)`, and `next(k)` from the treap in 2.3.1 depend only on the BST property and may be adapted
here as needed.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `empty()`.
- O(log n) amortized per call to `insert()`, `erase()`, and `find()`, where $n$ is the number of
  entries currently in the map.
- O(n) per call to `entries()`.

Space Complexity:
- O(n) for storage of the map elements.
- O(n) auxiliary stack space in the worst case for `insert()`, `erase()`, `find()`, `entries()`, and
  destruction.
- O(n) for the vector returned by `entries()`.
- O(1) auxiliary for `size()` and `empty()`.

*/

#include <functional>
#include <utility>
#include <vector>

template<typename K, typename V, typename Compare = std::less<K>>
class SplayTree {
  struct Node {
    K key;
    V value;
    Node *left, *right;

    Node(const K &k, const V &v) : key(k), value(v), left(nullptr), right(nullptr) {}
  } *root;

  int num_nodes;
  Compare comp;

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

  void splay(Node *&n, const K &k) {
    if (n == nullptr) {
      return;
    }
    if (comp(k, n->key) && n->left != nullptr) {
      if (comp(k, n->left->key)) {
        splay(n->left->left, k);
        rotate_right(n);
      } else if (comp(n->left->key, k)) {
        splay(n->left->right, k);
        if (n->left->right != nullptr) {
          rotate_left(n->left);
        }
      }
      if (n->left != nullptr) {
        rotate_right(n);
      }
    } else if (comp(n->key, k) && n->right != nullptr) {
      if (comp(k, n->right->key)) {
        splay(n->right->left, k);
        if (n->right->left != nullptr) {
          rotate_right(n->right);
        }
      } else if (comp(n->right->key, k)) {
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
    if (comp(k, n->key)) {
      Node *tmp = new Node(k, v);
      tmp->left = n->left;
      tmp->right = n;
      n->left = nullptr;
      n = tmp;
    } else if (comp(n->key, k)) {
      Node *tmp = new Node(k, v);
      tmp->left = n;
      tmp->right = n->right;
      n->right = nullptr;
      n = tmp;
    } else {
      return false;
    }
    num_nodes++;
    return true;
  }

  bool erase(Node *&n, const K &k) {
    if (n == nullptr) {
      return false;
    }
    splay(n, k);
    if (comp(k, n->key) || comp(n->key, k)) {
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
  explicit SplayTree(Compare comp = Compare{})
      : root(nullptr), num_nodes(0), comp(std::move(comp)) {}

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
    return (comp(k, root->key) || comp(root->key, k)) ? nullptr : &(root->value);
  }

  std::vector<std::pair<K, V>> entries() const {
    std::vector<std::pair<K, V>> res;
    res.reserve(num_nodes);
    collect_entries(root, res);
    return res;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  SplayTree<int, char> t;
  assert(t.empty());
  t.insert(2, 'b');
  t.insert(1, 'a');
  t.insert(3, 'c');
  t.insert(5, 'e');
  assert(t.insert(4, 'd'));
  assert(!t.empty() && t.size() == 5);
  assert(*t.find(4) == 'd');
  assert(!t.insert(4, 'd'));
  assert(
      (t.entries() == vector<pair<int, char>>{{1, 'a'}, {2, 'b'}, {3, 'c'}, {4, 'd'}, {5, 'e'}})
  );
  assert(t.erase(1));
  assert(!t.erase(1));
  assert(t.find(1) == nullptr);
  assert(t.size() == 4);
  assert((t.entries() == vector<pair<int, char>>{{2, 'b'}, {3, 'c'}, {4, 'd'}, {5, 'e'}}));

  SplayTree<int, char, greater<int>> descending;
  for (int key : {2, 1, 3}) {
    descending.insert(key, '0' + key);
  }
  assert((descending.entries() == vector<pair<int, char>>{{3, '3'}, {2, '2'}, {1, '1'}}));
  assert(*descending.find(2) == '2');
  assert(descending.erase(2) && descending.find(2) == nullptr);
  return 0;
}
