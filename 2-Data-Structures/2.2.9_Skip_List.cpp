/*

Maintain a map, that is, a collection of key-value pairs such that each possible
key appears at most once in the collection. This implementation requires both
the < and the == operators to be defined on the key type. A skip list maintains
a linked hierarchy of sorted subsequences with each successive subsequence
skipping over fewer elements than the previous one.

- skip_list() constructs an empty map.
- size() returns the size of the map.
- empty() returns whether the map is empty.
- insert(k, v) adds an entry with key k and value v to the map, returning true
  if an new entry was added or false if the key already exists (in which case
  the map is unchanged and the old value associated with the key is preserved).
- erase(k) removes the entry with key k from the map, returning true if the
  removal was successful or false if the key to be removed was not found.
- find(k) returns a pointer to a const value associated with key k, or NULL if
  the key was not found.
- operator[k] returns a reference to key k's associated value (which may be
  modified), or if necessary, inserts and returns a new entry with the default
  constructed value if key k was not originally found.
- walk(f) calls the function f(k, v) on each entry of the map, in ascending
  order of keys.

Time Complexity:
- O(1) per call to the constructor, size(), and empty().
- O(log n) on average per call to insert(), erase(), find(), and operator[],
  where n is the number of entries currently in the map.
- O(n) per call to walk().

Space Complexity:
- O(n) on average for storage of the map elements.
- O(n) auxiliary heap space for insert() and erase().
- O(1) auxiliary for all other operations.

*/

#include <cmath>
#include <cstdlib>
#include <vector>

template<class K, class V>
class skip_list {
  static const int MAX_LEVELS = 32;  // log2(max possible keys)

  struct node_t {
    K key;
    V value;
    std::vector<node_t*> next;

    node_t(const K &k, const V &v, int levels)
        : key(k), value(v), next(levels, (node_t*)NULL) {}
  } *head;

  int num_nodes;

  static int random_level() {
    static const double p = 0.5;
    int level = 1;
    while (((double)rand() / RAND_MAX) < p && std::abs(level) < MAX_LEVELS) {
      level++;
    }
    return std::abs(level);
  }

  static int node_level(const std::vector<node_t*> &v) {
    int i = 0;
    while (i < (int)v.size() && v[i] != NULL) {
      i++;
    }
    return i + 1;
  }

 public:
  skip_list() : head(new node_t(K(), V(), MAX_LEVELS)), num_nodes(0) {
    for (int i = 0; i < (int)head->next.size(); i++) {
      head->next[i] = NULL;
    }
  }

  ~skip_list() {
    delete head;
  }

  int size() const {
    return num_nodes;
  }

  bool empty() const {
    return num_nodes == 0;
  }

  bool insert(const K &k, const V &v) {
    std::vector<node_t*> update(head->next);
    int curr_level = node_level(update);
    node_t *n = head;
    for (int i = curr_level; i-- > 0; ) {
      while (n->next[i] != NULL && n->next[i]->key < k) {
        n = n->next[i];
      }
      update[i] = n;
    }
    n = n->next[0];
    if (n != NULL && n->key == k) {
      return false;
    }
    int new_level = random_level();
    if (new_level > curr_level) {
      for (int i = curr_level; i < new_level; i++) {
        update[i] = head;
      }
    }
    n = new node_t(k, v, new_level);
    for (int i = 0; i < new_level; i++) {
      n->next[i] = update[i]->next[i];
      update[i]->next[i] = n;
    }
    num_nodes++;
    return true;
  }

  bool erase(const K &k) {
    std::vector<node_t*> update(head->next);
    node_t *n = head;
    for (int i = node_level(update); i-- > 0; ) {
      while (n->next[i] != NULL && n->next[i]->key < k) {
        n = n->next[i];
      }
      update[i] = n;
    }
    n = n->next[0];
    if (n != NULL && n->key == k) {
      for (int i = 0; i < (int)update.size(); i++) {
        if (update[i]->next[i] != n) {
          break;
        }
        update[i]->next[i] = n->next[i];
      }
      delete n;
      num_nodes--;
      return true;
    }
    return false;
  }

  V* find(const K &k) const {
    node_t *n = head;
    for (int i = node_level(n->next); i-- > 0; ) {
      while (n->next[i] != NULL && n->next[i]->key < k) {
        n = n->next[i];
      }
    }
    n = n->next[0];
    return (n != NULL && n->key == k) ? &(n->value) : NULL;
  }

  V& operator[](const K &k) {
    V *ret = find(k);
    if (ret != NULL) {
      return *ret;
    }
    insert(k, V());
    return *find(k);
  }

  template<class KVFunction>
  void walk(KVFunction f) const {
    node_t *n = head->next[0];
    while (n != NULL) {
      f(n->key, n->value);
      n = n->next[0];
    }
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
  skip_list<int, char> l;
  l.insert(2, 'b');
  l.insert(1, 'a');
  l.insert(3, 'c');
  l.insert(5, 'e');
  assert(l.insert(4, 'd'));
  assert(*l.find(4) == 'd');
  assert(!l.insert(4, 'd'));
  l.walk(printch);
  cout << endl;
  assert(l.erase(1));
  assert(!l.erase(1));
  assert(l.find(1) == NULL);
  l.walk(printch);
  cout << endl;
  return 0;
}
