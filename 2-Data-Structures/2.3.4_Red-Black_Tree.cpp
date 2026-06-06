/*

Maintain a map, that is, a collection of key-value pairs such that each possible key appears at most
once in the collection. This implementation requires an ordering on the set of possible keys defined
by `operator<` on the key type. A red black tree is a binary search tree balanced by coloring its
nodes red or black, then constraining node colors on any simple path from the root to a leaf.

- `RedBlackTree()` constructs an empty map.
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
- O(log n) auxiliary stack space for `walk()`.
- O(1) auxiliary for all other operations.

*/

#include <algorithm>
#include <cstddef>

template<class K, class V>
class RedBlackTree {
  enum Color { RED, BLACK };
  struct Node {
    K key;
    V value;
    Color color;
    Node *left, *right, *parent;

    Node(const K &k, const V &v, Color c)
        : key(k), value(v), color(c), left(nullptr), right(nullptr), parent(nullptr) {}
  } *root, *LEAF_NIL;

  int num_nodes;

  void rotate_left(Node *n) {
    Node *tmp = n->right;
    if ((n->right = tmp->left) != LEAF_NIL) {
      n->right->parent = n;
    }
    if ((tmp->parent = n->parent) == LEAF_NIL) {
      root = tmp;
    } else if (n->parent->left == n) {
      n->parent->left = tmp;
    } else {
      n->parent->right = tmp;
    }
    tmp->left = n;
    n->parent = tmp;
  }

  void rotate_right(Node *n) {
    Node *tmp = n->left;
    if ((n->left = tmp->right) != LEAF_NIL) {
      n->left->parent = n;
    }
    if ((tmp->parent = n->parent) == LEAF_NIL) {
      root = tmp;
    } else if (n->parent->right == n) {
      n->parent->right = tmp;
    } else {
      n->parent->left = tmp;
    }
    tmp->right = n;
    n->parent = tmp;
  }

  void insert_fix(Node *n) {
    while (n->parent->color == RED) {
      Node *parent = n->parent;
      Node *grandparent = n->parent->parent;
      if (parent == grandparent->left) {
        Node *uncle = grandparent->right;
        if (uncle->color == RED) {
          grandparent->color = RED;
          parent->color = BLACK;
          uncle->color = BLACK;
          n = grandparent;
        } else {
          if (n == parent->right) {
            rotate_left(parent);
            n = parent;
            parent = n->parent;
          }
          rotate_right(grandparent);
          std::swap(parent->color, grandparent->color);
          n = parent;
        }
      } else if (parent == grandparent->right) {
        Node *uncle = grandparent->left;
        if (uncle->color == RED) {
          grandparent->color = RED;
          parent->color = BLACK;
          uncle->color = BLACK;
          n = grandparent;
        } else {
          if (n == parent->left) {
            rotate_right(parent);
            n = parent;
            parent = n->parent;
          }
          rotate_left(grandparent);
          std::swap(parent->color, grandparent->color);
          n = parent;
        }
      }
    }
    root->color = BLACK;
  }

  void replace(Node *n, Node *replacement) {
    if (n->parent == LEAF_NIL) {
      root = replacement;
    } else if (n == n->parent->left) {
      n->parent->left = replacement;
    } else {
      n->parent->right = replacement;
    }
    replacement->parent = n->parent;
  }

  void erase_fix(Node *n) {
    while (n != root && n->color == BLACK) {
      Node *parent = n->parent;
      if (n == parent->left) {
        Node *sibling = parent->right;
        if (sibling->color == RED) {
          sibling->color = BLACK;
          parent->color = RED;
          rotate_left(parent);
          sibling = parent->right;
        }
        if (sibling->left->color == BLACK && sibling->right->color == BLACK) {
          sibling->color = RED;
          n = parent;
        } else {
          if (sibling->right->color == BLACK) {
            sibling->left->color = BLACK;
            sibling->color = RED;
            rotate_right(sibling);
            sibling = parent->right;
          }
          sibling->color = parent->color;
          parent->color = BLACK;
          sibling->right->color = BLACK;
          rotate_left(parent);
          n = root;
        }
      } else {
        Node *sibling = parent->left;
        if (sibling->color == RED) {
          sibling->color = BLACK;
          parent->color = RED;
          rotate_right(parent);
          sibling = parent->left;
        }
        if (sibling->left->color == BLACK && sibling->right->color == BLACK) {
          sibling->color = RED;
          n = parent;
        } else {
          if (sibling->left->color == BLACK) {
            sibling->right->color = BLACK;
            sibling->color = RED;
            rotate_left(sibling);
            sibling = parent->left;
          }
          sibling->color = parent->color;
          parent->color = BLACK;
          sibling->left->color = BLACK;
          rotate_right(parent);
          n = root;
        }
      }
    }
    n->color = BLACK;
  }

  template<class Fn>
  void walk(Node *n, Fn f) const {
    if (n != LEAF_NIL) {
      walk(n->left, f);
      f(n->key, n->value);
      walk(n->right, f);
    }
  }

  void clean_up(Node *n) {
    if (n != LEAF_NIL) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  RedBlackTree() : num_nodes(0) { root = LEAF_NIL = new Node(K(), V(), BLACK); }

  ~RedBlackTree() {
    clean_up(root);
    delete LEAF_NIL;
  }

  RedBlackTree(const RedBlackTree &) = delete;
  RedBlackTree &operator=(const RedBlackTree &) = delete;
  int size() const { return num_nodes; }
  bool empty() const { return num_nodes == 0; }

  bool insert(const K &k, const V &v) {
    Node *curr = root, *prev = LEAF_NIL;
    while (curr != LEAF_NIL) {
      prev = curr;
      if (k < curr->key) {
        curr = curr->left;
      } else if (curr->key < k) {
        curr = curr->right;
      } else {
        return false;
      }
    }
    Node *n = new Node(k, v, RED);
    n->parent = prev;
    if (prev == LEAF_NIL) {
      root = n;
    } else if (k < prev->key) {
      prev->left = n;
    } else {
      prev->right = n;
    }
    n->left = n->right = LEAF_NIL;
    insert_fix(n);
    num_nodes++;
    return true;
  }

  bool erase(const K &k) {
    Node *n = root;
    while (n != LEAF_NIL) {
      if (k < n->key) {
        n = n->left;
      } else if (n->key < k) {
        n = n->right;
      } else {
        break;
      }
    }
    if (n == LEAF_NIL) {
      return false;
    }
    Color color = n->color;
    Node *replacement;
    if (n->left == LEAF_NIL) {
      replacement = n->right;
      replace(n, n->right);
    } else if (n->right == LEAF_NIL) {
      replacement = n->left;
      replace(n, n->left);
    } else {
      Node *tmp = n->right;
      while (tmp->left != LEAF_NIL) {
        tmp = tmp->left;
      }
      color = tmp->color;
      replacement = tmp->right;
      if (tmp->parent == n) {
        replacement->parent = tmp;
      } else {
        replace(tmp, tmp->right);
        tmp->right = n->right;
        tmp->right->parent = tmp;
      }
      replace(n, tmp);
      tmp->left = n->left;
      tmp->left->parent = tmp;
      tmp->color = n->color;
    }
    delete n;
    if (color == BLACK) {
      erase_fix(replacement);
    }
    return true;
  }

  const V *find(const K &k) const {
    Node *n = root;
    while (n != LEAF_NIL) {
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
  RedBlackTree<int, char> t;
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
