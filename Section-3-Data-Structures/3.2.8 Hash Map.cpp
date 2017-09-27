/*

Maintain a map, that is, a collection of key-value pairs such that each possible
key appears at most once in the collection. This implementation requires the ==
operator to be defined on the key type. A hash map implements a map by hashing
keys into buckets using a hash function. This implementation resolves collisions
by chaining entries hashed to the same bucket into a linked list.

- hash_map() constructs an empty map.
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
- walk(f) calls the function f(k, v) on each entry of the map, in no guaranteed
  order.

Time Complexity:
- O(1) per call to the constructor, size(), and empty().
- O(1) amortized per call to insert(), erase(), find(), and operator[].
- O(n) per call to walk(), where n is the number of entries in the map.

Space Complexity:
- O(n) for storage of the map elements.
- O(n) auxiliary heap space for insert().
- O(1) auxiliary for all other operations.

*/

#include <cstdlib>
#include <list>

template<class K, class V, class Hash>
class hash_map {
  struct entry_t {
    K key;
    V value;

    entry_t(const K &k, const V &v) : key(k), value(v) {}
  };

  std::list<entry_t> *table;
  int table_size, num_entries;

  void double_capacity_and_rehash() {
    std::list<entry_t> *old = table;
    int old_size = table_size;
    table_size = 2*table_size;
    table = new std::list<entry_t>[table_size];
    num_entries = 0;
    typename std::list<entry_t>::iterator it;
    for (int i = 0; i < old_size; i++) {
      for (it = old[i].begin(); it != old[i].end(); ++it) {
        insert(it->key, it->value);
      }
    }
    delete[] old;
  }

 public:
  hash_map(int size = 128) : table_size(size), num_entries(0) {
    table = new std::list<entry_t>[table_size];
  }

  ~hash_map() {
    delete[] table;
  }

  int size() const {
    return num_entries;
  }

  bool empty() const {
    return num_entries == 0;
  }

  bool insert(const K &k, const V &v) {
    if (find(k) != NULL) {
      return false;
    }
    if (num_entries >= table_size) {
      double_capacity_and_rehash();
    }
    unsigned int i = Hash()(k) % table_size;
    table[i].push_back(entry_t(k, v));
    num_entries++;
    return true;
  }

  bool erase(const K &k) {
    unsigned int i = Hash()(k) % table_size;
    typename std::list<entry_t>::iterator it = table[i].begin();
    while (it != table[i].end() && !(it->key == k)) {
      ++it;
    }
    if (it == table[i].end()) {
      return false;
    }
    table[i].erase(it);
    num_entries--;
    return true;
  }

  V* find(const K &k) const {
    unsigned int i = Hash()(k) % table_size;
    typename std::list<entry_t>::iterator it = table[i].begin();
    while (it != table[i].end() && !(it->key == k)) {
      ++it;
    }
    if (it == table[i].end()) {
      return NULL;
    }
    return &(it->value);
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
    for (int i = 0; i < table_size; i++) {
      typename std::list<entry_t>::iterator it;
      for (it = table[i].begin(); it != table[i].end(); ++it) {
        f(it->key, it->value);
      }
    }
  }
};

/*** Example Usage and Output:

cab

***/

#include <cassert>
#include <iostream>
using namespace std;

struct class_hash {
  unsigned int operator()(int k) {
    return class_hash()((unsigned int)k);
  }

  unsigned int operator()(long long k) {
    return class_hash()((unsigned long long)k);
  }

  // Knuth's one-to-one multiplicative method.
  unsigned int operator()(unsigned int k) {
    return k * 2654435761u;  // Or just return k.
  }

  // Jenkins's 64-bit hash.
  unsigned int operator()(unsigned long long k) {
    k += ~(k << 32);
    k ^=  (k >> 22);
    k += ~(k << 13);
    k ^=  (k >> 8);
    k +=  (k << 3);
    k ^=  (k >> 15);
    k += ~(k << 27);
    k ^=  (k >> 31);
    return k;
  }

  // Jenkins's one-at-a-time hash.
  unsigned int operator()(const std::string &k) {
    unsigned int hash = 0;
    for (unsigned int i = 0; i < k.size(); i++) {
      hash += ((hash + k[i]) << 10);
      hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    return hash + (hash << 15);
  }
};

void printch(const string &k, char v) {
  cout << v;
}

int main() {
  hash_map<string, char, class_hash> m;
  m["foo"] = 'a';
  m.insert("bar", 'b');
  assert(m["foo"] == 'a');
  assert(m["bar"] == 'b');
  assert(m["baz"] == '\0');
  m["baz"] = 'c';
  m.walk(printch);
  cout << endl;
  assert(m.erase("foo"));
  assert(m.size() == 2);
  assert(m["foo"] == '\0');
  assert(m.size() == 3);
  return 0;
}
