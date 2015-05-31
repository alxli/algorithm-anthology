/*

3.4.1 - AVL Tree

Description: Description: A binary search tree (BST) is a node-based
binary tree data structure where the left sub-tree of every node has
keys less than the node's key and the right sub-tree of every node
has keys greater (greater or equal in this implementation) than the
node’s key. A BST may be come degenerate like a linked list resulting
in an O(N) running time per operation.

The AVL tree was the first self-balancing binary search tree to be
invented. In an AVL tree, the heights of the two child subtrees of any
node differ by at most one. if at any time they differ by more than one,
rebalancing is done to restore this property.

Time Complexity: insert(), remove() and find() are O(log(N)) in the
worst case. walk() is O(N).

Space Complexity: O(N) on the number of nodes.

*/

template<class key_t, class val_t> class avl_tree {
  struct node {
    key_t key;
    val_t val;
    node *parent, *L, *R;
    int height;

    node(const key_t &k, const val_t &v): key(k), val(v),
      height(0), parent(0), L(0), R(0) {}

    int updateHeight() {
      if (L != 0 && R != 0) {
        return height = (L->height > R->height ? L->height : R->height) + 1;
      }
      if (L != 0) return height = L->height + 1;
      if (R != 0) return height = R->height + 1;
      return height = 0;
    }

    int getBalance() {
      node *n = this;
      if (n->L != 0 && n->R != 0) return n->L->height - n->R->height;
      if (n->L != 0) return n->L->height + 1;
      if (n->R != 0) return -(n->R->height + 1);
      return 0;
    }

    node* setL(node * n) {
      if (n != 0) n->parent = this;
      L = n; updateHeight(); return L;
    }

    node* setR(node * n) {
      if (n != 0) n->parent = this;
      R = n; updateHeight(); return R;
    }
  } *root;

  int num_nodes;

  void setRoot(node * n) {
    if ((root = n) != 0) root->parent = 0;
  }

  void rotateL(node * n) {
    node *p = n->parent;
    enum { L, R } side;
    if (p != 0) side = (p->L == n) ? L : R;
    node *tmp = n->R; n->setR(tmp->L); tmp->setL(n);
    if (p == 0) setRoot(tmp);
    else if (side == L) p->setL(tmp);
    else if (side == R) p->setR(tmp);
  }

  void rotateR(node * n) {
    node *p = n->parent;
    enum { L, R } side;
    if (p != 0) side = (p->L == n) ? L : R;
    node *tmp = n->L; n->setL(tmp->R); tmp->setR(n);
    if (p == 0) setRoot(tmp);
    else if (side == L) p->setL(tmp);
    else if (side == R) p->setR(tmp); 
  }

  void balance(node * n) {
    int bal = n->getBalance();
    if (bal > 1) {
      if (n->L->getBalance() < 0) rotateL(n->L);
      rotateR(n);
    } else if (bal < -1) {
      if (n->R->getBalance() > 0) rotateR(n->R);
      rotateL(n);
    }
  }

  node* find_node(const key_t & key) {
    for (node* n = root; n != 0; ) {
      if (key < n->key) n = n->L;
      else if (n->key < key) n = n->R;
      else return n;
    }
    return 0;
  }

 template <class Func>
  void internal_walk(node *p, void (*f)(Func), char order) {
    if (p == 0) return;
    if (order == -1) (*f)(p->val);
    internal_walk(p->L, f, order);
    if (order == 0) (*f)(p->val);
    internal_walk(p->R, f, order);
    if (order == 1) (*f)(p->val);
  }

  void clean_up(node * n) {
    if (n == 0) return;
    clean_up(n->L);
    clean_up(n->R);
    delete n;
  }

 public:
  avl_tree(): num_nodes(0) { root = 0; }
  avl_tree(const key_t & k, const val_t & v): num_nodes(1) {
    root = new node(k, v);
  }
  
  ~avl_tree() { clean_up(root); }
  int height() { return root->height; }
  int size() { return num_nodes; }

  avl_tree* subtree(const key_t & k) {
    node * target = find_node(k);
    if (target == 0) return 0;
    avl_tree *subtree = new avl_tree();
    subtree->root = target;
    return subtree;
  }

  val_t* find(const key_t & k) {
    node * n = find_node(k);
    return n == 0 ? 0 : &(n->val);
  }

  template <class Func>
  void walk(void (*f)(Func), char order = 0) {
    internal_walk(root, f, order);
  }

  bool insert(const key_t & k, const val_t & v) {
    if (root == 0) {
      root = new node(k, v);
      num_nodes++;
      return true;
    }
    node *tmp = root, *added;
    while (true) {
      if (k < tmp->key) {
        if ((tmp->L) == 0) {
          added = tmp->setL(new node(k, v));
          break;
        } else tmp = tmp->L;
      } else if (tmp->key < k) {
        if ((tmp->R) == 0) {
          added = tmp->setR(new node(k, v));
          break;
        } else tmp = tmp->R;
      } else return false;
    }
    for (tmp = added; tmp != 0; tmp = tmp->parent) {
      tmp->updateHeight();
      balance(tmp);
    }
    num_nodes++;
    return true;
  }

  bool remove(const key_t & k) {
    if (root == 0) return false;
    node *new_n, *new_p, *tmp, *n = find_node(k), *p;
    if (n == 0) return 0; 
    int bal = n->getBalance();
    enum {L, R} side;
    if ((p = n->parent) != 0) side = (p->L == n) ? L : R;
    if (n->L == 0 && n->R == 0) {
      if (p != 0) {
        side == L ? p->setL(0) : p->setR(0);
        delete n;
        p->updateHeight();
        balance(p);
      } else {
        setRoot(0);
        delete n;
      }
    } else if (n->R == 0) {
      if (p != 0) {
        side == L ? p->setL(n->L) : p->setR(n->L);
        delete n;
        p->updateHeight();
        balance(p);
      } else {
        setRoot(n->L);
        delete n;
      }
    } else if (n->L == 0) {
      if (p != 0) {
        side == L ? p->setL(n->R) : p->setR(n->R);
        delete n;
        p->updateHeight();
        balance(p);
      } else {
        setRoot(n->R);
        delete n;
      }
    } else {
      if (bal > 0) {
        if (n->L->R == 0) {
          tmp = new_n = n->L;
          new_n->setR(n->R);
        } else {
          new_n = n->L->R;
          while (new_n->R != 0) new_n = new_n->R;
          tmp = new_p = new_n->parent;
          new_p->setR(new_n->L);
          new_n->setL(n->L);
          new_n->setR(n->R);
        }
      } else {
        if (n->R->L == 0) {
          tmp = new_n = n->R;
          new_n->setL(n->L);
        } else {
          new_n = n->R->L;
          while (new_n->L != 0) new_n = new_n->L;
          tmp = new_p = new_n->parent;
          new_p->setL(new_n->R);
          new_n->setL(n->L);
          new_n->setR(n->R);
        }
      }
      if (p != 0) {
        if (side == L) p->setL(new_n);
        else p->setR(new_n);
      } else setRoot(new_n);
      delete n;
      balance(tmp);
    }
    num_nodes--;
    return true;
  }
};

/*** Stress Test - Runs in <1 second ***/

#include <cassert>

int main() {
  avl_tree<int, int> T;

  //insert keys in an order that would break a normal BST
  for (int i = 0; i < 1000000; i++) {
    T.insert(i, i + i);
  }
  for (int i = 0; i < 1000000; i++) {
    assert(*T.find(i) == i + i);
  }

  return 0;
}
