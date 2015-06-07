/*

3.5.5 - Skip List (Probabilistic)

Description: A skip list is an alternative to binary search trees.
Fast search is made possible by maintaining a linked hierarchy of
subsequences, each skipping over fewer elements. Searching starts
in the sparsest subsequence until two consecutive elements have
been found, one smaller and one larger than the element searched for.
Skip lists are generally slower than binary search trees, but can
be easier to implement. The following version uses randomized levels.

Time Complexity: insert(), erase(), count() and find() are O(log(N))
on average, but O(N) in the worst case. walk() is O(N).

Space Complexity: O(N) on the number of elements inserted on average,
but O(N log N) in the worst case.

*/

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>

template<class key_t, class val_t> struct skip_list {

  static const int MAX_LEVEL = 32; //~ log2(max # of keys)
  static const float P = 0.5;

  static int random_level() { //geometric distribution
    int lvl = log((float)rand()/RAND_MAX)/log(1.0 - P);
    return lvl < MAX_LEVEL ? lvl : MAX_LEVEL;
  }

  struct node_t {
    key_t key;
    val_t val;
    node_t **next;

    node_t(int level, const key_t & k, const val_t & v) {
      next = new node_t * [level + 1];
      memset(next, 0, sizeof(node_t*)*(level + 1));
      key = k;
      val = v;
    }

    ~node_t() { delete[] next; }
  } *head, *update[MAX_LEVEL + 1];

  int level, num_nodes;

  skip_list() {
    srand(time(0));
    head = new node_t(MAX_LEVEL, key_t(), val_t());
    level = num_nodes = 0;
  }

  ~skip_list() { delete head; }
  int size() { return num_nodes; }
  bool empty() { return num_nodes == 0; }
  int count(const key_t & k) { return find(k) != 0; }

  void insert(const key_t & k, const val_t & v) {
    node_t * n = head;
    memset(update, 0, sizeof(node_t*)*(MAX_LEVEL + 1));
    for (int i = level; i >= 0; i--) {
      while (n->next[i] && n->next[i]->key < k) n = n->next[i];
      update[i] = n;
    }
    n = n->next[0];
    if (!n || n->key != k) {
      int lvl = random_level();
      if (lvl > level) {
        for (int i = level + 1; i <= lvl; i++) update[i] = head;
        level = lvl;
      }
      n = new node_t(lvl, k, v);
      num_nodes++;
      for (int i = 0; i <= lvl; i++) {
        n->next[i] = update[i]->next[i];
        update[i]->next[i] = n;
      }
    } else if (n && n->key == k && n->val != v) {
      n->val = v;
    }
  }

  void erase(const key_t & k) {
    node_t * n = head;
    memset(update, 0, sizeof(node_t*)*(MAX_LEVEL + 1));
    for (int i = level;i >= 0; i--) {
      while (n->next[i] && n->next[i]->key < k) n = n->next[i];
      update[i] = n;
    }
    n = n->next[0];
    if (n->key == k) {
      for (int i = 0; i <= level; i++) {
        if (update[i]->next[i] != n) break;
        update[i]->next[i] = n->next[i];
      }
      delete n;
      num_nodes--;
      while (level > 0 && !head->next[level]) level--;
    }
  }

  val_t* find(const key_t & k) {
    node_t * n = head;
    for (int i = level; i >= 0; i--)
      while (n->next[i] && n->next[i]->key < k)
        n = n->next[i];
    n = n->next[0];
    if (n && n->key == k) return &(n->val);
    return 0; //not found
  }

  template<class BinaryFunction> void walk(BinaryFunction f) {
    node_t *n = head->next[0];
    while (n) {
      f(n->key, n->val);
      n = n->next[0];
    }
  }
};

/*** Example Usage: Random Tests (runs in <2 seconds) ***/

#include <cassert>
#include <iostream>
#include <map>
using namespace std;

int main() {
  map<int, int> m;
  skip_list<int, int> s;
  for (int i = 0; i < 1000000; i++) {
    int op = rand() % 3;
    int val1 = rand(), val2 = rand();
    if (op == 0) {
      m[val1] = val2;
      s.insert(val1, val2);
    } else if (op == 1) {
      if (!m.count(val1)) continue;
      m.erase(val1);
      s.erase(val1);
    } else if (op == 2) {
      assert(m.count(val1) == s.count(val1));
      if (m.count(val1)) {
        assert(m[val1] == *s.find(val1));
      }
    }
  }
  return 0;
}
