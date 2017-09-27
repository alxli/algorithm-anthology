/*

Maintain a map, that is, a collection of key-value pairs such that each possible
key appears at most once in the collection. This implementation requires an
ordering on the set of possible keys defined by the < operator on the key type.
A red black tree is a binary search tree balanced by coloring its nodes red or
black, then constraining node colors on any simple path from the root to a leaf.

- red_black_tree() constructs an empty map.
- size() returns the size of the map.
- empty() returns whether the map is empty.
- insert(k, v) adds an entry with key k and value v to the map, returning true
  if an new entry was added or false if the key already exists (in which case
  the map is unchanged and the old value associated with the key is preserved).
- erase(k) removes the entry with key k from the map, returning true if the
  removal was successful or false if the key to be removed was not found.
- find(k) returns a pointer to a const value associated with key k, or NULL if
  the key was not found.
- walk(f) calls the function f(k, v) on each entry of the map, in ascending
  order of keys.

Time Complexity:
- O(1) per call to the constructor, size(), and empty().
- O(log n) per call to insert(), erase(), and find(), where n is the number of
  entries currently in the map.
- O(n) per call to walk().

Space Complexity:
- O(n) for storage of the map elements.
- O(log n) auxiliary stack space for walk().
- O(1) auxiliary for all other operations.

*/

#include <algorithm>
#include <cstdlib>

template<class K, class V>
class red_black_tree {
  enum color_t { RED, BLACK };
  struct node_t {
    K key;
    V value;
    color_t color;
    node_t *left, *right, *parent;

    node_t(const K &k, const V &v, color_t c)
        : key(k), value(v), color(c), left(NULL), right(NULL), parent(NULL) {}
  } *root, *LEAF_NIL;

  int num_nodes;

  void rotate_left(node_t *n) {
    node_t *tmp = n->right;
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

  void rotate_right(node_t *n) {
    node_t *tmp = n->left;
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

  void insert_fix(node_t *n) {
    while (n->parent->color == RED) {
      node_t *parent = n->parent;
      node_t *grandparent = n->parent->parent;
      if (parent == grandparent->left) {
        node_t *uncle = grandparent->right;
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
        node_t *uncle = grandparent->left;
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

  void replace(node_t *n, node_t *replacement) {
    if (n->parent == LEAF_NIL) {
      root = replacement;
    } else if (n == n->parent->left) {
      n->parent->left = replacement;
    } else {
      n->parent->right = replacement;
    }
    replacement->parent = n->parent;
  }

  void erase_fix(node_t *n) {
    while (n != root && n->color == BLACK) {
      node_t *parent = n->parent;
      if (n == parent->left) {
        node_t *sibling = parent->right;
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
        node_t *sibling = parent->left;
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

  template<class KVFunction>
  void walk(node_t *n, KVFunction f) const {
    if (n != LEAF_NIL) {
      walk(n->left, f);
      f(n->key, n->value);
      walk(n->right, f);
    }
  }

  void clean_up(node_t *n) {
    if (n != LEAF_NIL) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  red_black_tree() : num_nodes(0) {
    root = LEAF_NIL = new node_t(K(), V(), BLACK);
  }

  ~red_black_tree() {
    clean_up(root);
    delete LEAF_NIL;
  }

  int size() const {
    return num_nodes;
  }

  bool empty() const {
    return num_nodes == 0;
  }

  bool insert(const K &k, const V &v) {
    node_t *curr = root, *prev = LEAF_NIL;
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
    node_t *n = new node_t(k, v, RED);
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
    node_t *n = root;
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
    color_t color = n->color;
    node_t *replacement;
    if (n->left == LEAF_NIL) {
      replacement = n->right;
      replace(n, n->right);
    } else if (n->right == LEAF_NIL) {
      replacement = n->left;
      replace(n, n->left);
    } else {
      node_t *tmp = n->right;
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

  const V* find(const K &k) const {
    node_t *n = root;
    while (n != LEAF_NIL) {
      if (k < n->key) {
        n = n->left;
      } else if (n->key < k) {
        n = n->right;
      } else {
        return &(n->value);
      }
    }
    return NULL;
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

void printch(int k, char v) {
  cout << v;
}

int main() {
  red_black_tree<int, char> t;
  t.insert(2, 'b');
  t.insert(1, 'a');
  t.insert(3, 'c');
  t.insert(5, 'e');
  assert(t.insert(4, 'd'));
  assert(*t.find(4) == 'd');
  assert(!t.insert(4, 'd'));
  t.walk(printch);
  cout << endl;
  assert(t.erase(1));
  assert(!t.erase(1));
  assert(t.find(1) == NULL);
  t.walk(printch);
  cout << endl;
  return 0;
}
