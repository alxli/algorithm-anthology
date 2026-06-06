/*

Maintain a map, that is, a collection of key-value pairs such that each possible key appears at most
once in the collection. This implementation requires an ordering on the set of possible keys
defined by `operator<` on the key type. A Treap is a binary search tree where each node holds a
randomly assigned priority. The tree satisfies the BST property on keys and the min-heap property
on priorities (lower priority value is closer to root). Since priorities are random, this keeps the
tree balanced with high probability, making insertions and deletions run in O(log n).

- `Treap()` constructs an empty map.
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
- O(log n) on average per call to `insert()`, `erase()`, and `find()`, where $n$ is the number of
  entries currently in the map.
- O(n) per call to `walk()`.

Space Complexity:
- O(n) for storage of the map elements.
- O(log n) auxiliary stack space on average for `insert()`, `erase()`, and `walk()`.
- O(1) auxiliary for all other operations.

*/

#include <cstdint>

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

  static bool insert(Node *&n, const K &k, const V &v) {
    if (n == nullptr) {
      n = new Node(k, v);
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
  Treap() : root(nullptr), num_nodes(0) {}

  ~Treap() { clean_up(root); }
  Treap(const Treap &) = delete;
  Treap &operator=(const Treap &) = delete;
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
  Treap<int, char> t;
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
