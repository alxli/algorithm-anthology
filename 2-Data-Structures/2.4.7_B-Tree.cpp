/*

Maintain an ordered map, that is, an ordered collection of key-value pairs such that each possible
key appears at most once in the collection. A B-tree is a balanced search tree whose nodes hold many
entries instead of one. Every node except the root stores between $d - 1$ and $2d - 1$ entries for a
chosen minimum degree $d$, an internal node with $k$ entries has exactly $k + 1$ children, and every
leaf sits at the same depth. The height is O(log_d(n)), and a search examines only that many nodes.

The point is the unit of cost. When reading a node is far more expensive than comparing keys inside
one (as with disk blocks or cache lines), the height is what matters rather than the comparison
count, and packing hundreds of entries per node makes a tree of billions of keys three or four nodes
deep. This is why B-trees hold filesystem and database indexes while BSTs hold in-memory indexes.

Balance is maintained without any rebalancing pass. Descending to insert, the search splits every
full node it meets, so a node always has room for the median that a split of its child pushes up,
and the split cannot cascade back upward. Descending to erase, it does the reverse: before entering
a child with only $d - 1$ entries, it borrows one from an adjacent sibling, or merges the child with
one, so that a deletion never leaves a node underfull. Each pass therefore visits each level once.

- `BTree<K, V, MIN_DEGREE>()` constructs an empty map, where the minimum degree must be at least $2$
  and defaults to $3$.
- `size()` and `empty()` return the number of entries and whether the map is empty.
- `height()` returns the number of levels, which is $0$ for an empty map.
- `find(k)` returns a pointer to a const value associated with key `k`, or `nullptr` if the key was
  not found.
- `insert(k, v)` adds an entry with key `k` and value `v`, returning `true` if a new entry was added
  or `false` if the key already exists (in which case the map is unchanged and the old value
  associated with the key is preserved).
- `erase(k)` removes the entry with key `k`, returning whether it was present.
- `entries()` returns all key-value entries in sorted order.

Only `operator<` is required of `K`. The tree grows in height solely by splitting a full root, and
shrinks solely when the root's last two children merge, which is what keeps every leaf at one depth.

For an in-memory ordered dictionary the treap of section 2.4.1 or the standard `std::map` is
smaller, simpler, and faster; reach for this shape when node reads dominate, or when a problem
statement describes a tree of exactly this kind.

Time Complexity:
- O(d log_d(n)) per call to `insert()`, `erase()`, and `find()`, where $n$ is the number of entries.
  The searches within a node are linear in $d$, which is negligible against the cost of reaching the
  node in the setting the structure is designed for.
- O(1) per call to `size()`, `empty()`, and `height()`.
- O(n) per call to `entries()`.

Space Complexity:
- O(n) for storage of the entries.
- O(log_d(n)) auxiliary stack space per operation, and O(n) for the vector returned by `entries()`.

*/

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

template<typename K, typename V, int MIN_DEGREE = 3>
class BTree {
  static_assert(MIN_DEGREE >= 2, "a B-tree node must hold at least one entry");

  using Entry = std::pair<K, V>;

  struct Node {
    std::vector<Entry> entries;
    std::vector<Node *> children;

    bool leaf() const { return children.empty(); }
    bool full() const { return static_cast<int>(entries.size()) == 2 * MIN_DEGREE - 1; }
    bool minimal() const { return static_cast<int>(entries.size()) < MIN_DEGREE; }
  };

  Node *root;
  int num_nodes, levels;

  // Returns the first position whose key is not less than the target.
  static int lower_position(const Node *n, const K &key) {
    auto by_key = [](const Entry &e, const K &k) { return e.first < k; };
    auto it = std::lower_bound(n->entries.begin(), n->entries.end(), key, by_key);
    return static_cast<int>(it - n->entries.begin());
  }

  // Splits the full child at i, lifting its median entry into the parent, which must not be full.
  static void split_child(Node *parent, int i) {
    Node *left = parent->children[i], *right = new Node();
    right->entries.assign(left->entries.begin() + MIN_DEGREE, left->entries.end());
    if (!left->leaf()) {
      right->children.assign(left->children.begin() + MIN_DEGREE, left->children.end());
      left->children.resize(MIN_DEGREE);
    }
    parent->entries.insert(parent->entries.begin() + i, left->entries[MIN_DEGREE - 1]);
    parent->children.insert(parent->children.begin() + i + 1, right);
    left->entries.resize(MIN_DEGREE - 1);
  }

  static void insert_nonfull(Node *n, const Entry &entry) {
    int i = lower_position(n, entry.first);
    if (n->leaf()) {
      n->entries.insert(n->entries.begin() + i, entry);
      return;
    }
    if (n->children[i]->full()) {
      split_child(n, i);
      if (n->entries[i].first < entry.first) {  // The lifted median now sits before the key.
        i++;
      }
    }
    insert_nonfull(n->children[i], entry);
  }

  // Restores at least MIN_DEGREE entries in child i by borrowing from a sibling or merging with
  // one, then returns the child to descend into, which merging may shift left.
  static Node *ensure_not_minimal(Node *parent, int i) {
    Node *child = parent->children[i];
    if (!child->minimal()) {
      return child;
    }
    Node *left = i > 0 ? parent->children[i - 1] : nullptr;
    Node *right =
        i + 1 < static_cast<int>(parent->children.size()) ? parent->children[i + 1] : nullptr;
    if (left != nullptr && !left->minimal()) {  // Rotate an entry across through the parent.
      child->entries.insert(child->entries.begin(), parent->entries[i - 1]);
      parent->entries[i - 1] = left->entries.back();
      left->entries.pop_back();
      if (!left->leaf()) {
        child->children.insert(child->children.begin(), left->children.back());
        left->children.pop_back();
      }
      return child;
    }
    if (right != nullptr && !right->minimal()) {
      child->entries.push_back(parent->entries[i]);
      parent->entries[i] = right->entries.front();
      right->entries.erase(right->entries.begin());
      if (!right->leaf()) {
        child->children.push_back(right->children.front());
        right->children.erase(right->children.begin());
      }
      return child;
    }
    // Both neighbors are minimal, so fold the separating entry and one sibling into the child.
    if (left != nullptr) {
      left->entries.push_back(parent->entries[i - 1]);
      left->entries.insert(left->entries.end(), child->entries.begin(), child->entries.end());
      left->children.insert(left->children.end(), child->children.begin(), child->children.end());
      parent->entries.erase(parent->entries.begin() + i - 1);
      parent->children.erase(parent->children.begin() + i);
      delete child;
      return left;
    }
    child->entries.push_back(parent->entries[i]);
    child->entries.insert(child->entries.end(), right->entries.begin(), right->entries.end());
    child->children.insert(child->children.end(), right->children.begin(), right->children.end());
    parent->entries.erase(parent->entries.begin() + i);
    parent->children.erase(parent->children.begin() + i + 1);
    delete right;
    return child;
  }

  // The key is taken by value: callers pass a key stored in the tree, and a merge below may erase
  // from the very vector a reference would point into.
  static bool erase(Node *n, K key) {
    int i = lower_position(n, key);
    bool here = i < static_cast<int>(n->entries.size()) && !(key < n->entries[i].first);
    if (n->leaf()) {
      if (!here) {
        return false;
      }
      n->entries.erase(n->entries.begin() + i);
      return true;
    }
    if (!here) {
      return erase(ensure_not_minimal(n, i), key);
    }
    if (!n->children[i]->minimal()) {  // Replace with the predecessor and delete that instead.
      Node *walk = n->children[i];
      while (!walk->leaf()) {
        walk = walk->children.back();
      }
      n->entries[i] = walk->entries.back();
      return erase(n->children[i], n->entries[i].first);
    }
    if (!n->children[i + 1]->minimal()) {  // Or the successor.
      Node *walk = n->children[i + 1];
      while (!walk->leaf()) {
        walk = walk->children.front();
      }
      n->entries[i] = walk->entries.front();
      return erase(n->children[i + 1], n->entries[i].first);
    }
    // Both children are minimal, so fold them and the entry between them into one node, which is
    // where the entry now lives. Borrowing would rotate a different entry down and lose this one.
    Node *left = n->children[i], *right = n->children[i + 1];
    left->entries.push_back(n->entries[i]);
    left->entries.insert(left->entries.end(), right->entries.begin(), right->entries.end());
    left->children.insert(left->children.end(), right->children.begin(), right->children.end());
    n->entries.erase(n->entries.begin() + i);
    n->children.erase(n->children.begin() + i + 1);
    delete right;
    return erase(left, key);
  }

  static void collect(const Node *n, std::vector<Entry> *out) {
    for (int i = 0; i < static_cast<int>(n->entries.size()); i++) {
      if (!n->leaf()) {
        collect(n->children[i], out);
      }
      out->push_back(n->entries[i]);
    }
    if (!n->leaf()) {
      collect(n->children.back(), out);
    }
  }

  static void clean_up(Node *n) {
    if (n != nullptr) {
      for (Node *child : n->children) {
        clean_up(child);
      }
      delete n;
    }
  }

 public:
  BTree() : root(new Node()), num_nodes(0), levels(1) {}

  BTree(const BTree &) = delete;
  BTree &operator=(const BTree &) = delete;
  ~BTree() { clean_up(root); }
  int size() const { return num_nodes; }
  bool empty() const { return num_nodes == 0; }
  int height() const { return num_nodes == 0 ? 0 : levels; }

  const V *find(const K &k) const {
    for (const Node *n = root; n != nullptr;) {
      int i = lower_position(n, k);
      if (i < static_cast<int>(n->entries.size()) && !(k < n->entries[i].first)) {
        return &n->entries[i].second;
      }
      n = n->leaf() ? nullptr : n->children[i];
    }
    return nullptr;
  }

  bool insert(const K &k, const V &v) {
    if (find(k) != nullptr) {
      return false;
    }
    if (root->full()) {  // The only way the tree grows taller.
      Node *grown = new Node();
      grown->children.push_back(root);
      split_child(grown, 0);
      root = grown;
      levels++;
    }
    insert_nonfull(root, Entry(k, v));
    num_nodes++;
    return true;
  }

  bool erase(const K &k) {
    // The descent merges nodes on the way down whether or not the key turns out to be present,
    // so an emptied root must be collapsed even when nothing was removed.
    bool removed = erase(root, k);
    if (root->entries.empty() && !root->leaf()) {  // The only way the tree grows shorter.
      Node *old = root;
      root = root->children[0];
      old->children.clear();
      delete old;
      levels--;
    }
    num_nodes -= removed;
    return removed;
  }

  std::vector<Entry> entries() const {
    std::vector<Entry> res;
    res.reserve(num_nodes);
    collect(root, &res);
    return res;
  }
};

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  BTree<int, int> tree;  // Minimum degree 3, so a node holds 2 to 5 entries.
  assert(tree.empty() && tree.height() == 0 && tree.find(1) == nullptr);

  for (int x : {10, 20, 5, 6, 12, 30, 7, 17}) {
    assert(tree.insert(x, x * x));
  }
  assert(!tree.insert(10, 0));  // Duplicate keys are rejected and the old value preserved.
  assert(tree.size() == 8 && *tree.find(10) == 100 && tree.find(18) == nullptr);

  vector<int> keys;
  for (const auto &entry : tree.entries()) {
    keys.push_back(entry.first);
  }
  assert((keys == vector<int>{5, 6, 7, 10, 12, 17, 20, 30}));

  // Enough entries to split the root, which is the only way the height changes.
  BTree<int, int> deep;
  for (int x = 0; x < 100; x++) {
    deep.insert(x, -x);
  }
  assert(deep.size() == 100 && deep.height() == 4);
  assert(deep.entries().front().first == 0 && deep.entries().back().first == 99);

  // Erasing from an internal node pulls up a neighbouring entry rather than leaving a gap.
  assert(tree.erase(10) && !tree.erase(10));
  assert(tree.size() == 7 && tree.find(10) == nullptr && *tree.find(12) == 144);
  for (int x : {5, 6, 7, 12, 17, 20, 30}) {
    assert(tree.erase(x));
  }
  assert(tree.empty() && tree.entries().empty());

  // Deleting every entry in order collapses the height back to one level.
  for (int x = 0; x < 100; x++) {
    assert(deep.erase(x));
  }
  assert(deep.empty() && deep.height() == 0);
  return 0;
}
