/*

Description: A hashmap (std::unordered_map in C++11) is an
alternative to a binary search tree. Hashmaps use more memory than
BSTs, but are usually more efficient. The following implementation
uses the chaining method to handle collisions. You can use the
hash algorithms provided in the example, or define your own.

Time Complexity: insert(), remove(), find(), are O(1) amortized.
rehash() is O(N).

Space Complexity: O(N) on the number of entries.

*/

#include <list>

template<class key_t, class val_t, class Hash> class hashmap {
  struct entry_t {
    key_t key;
    val_t val;
    entry_t(const key_t & k, const val_t & v): key(k), val(v) {}
  };

  std::list<entry_t> * table;
  int table_size, map_size;

  /**
   * This doubles the table size, then rehashes every entry.
   * Rehashing is expensive; it is strongly suggested for the
   * table to be constructed with a large size to avoid rehashing.
   */
  void rehash() {
    std::list<entry_t> * old = table;
    int old_size = table_size;
    table_size = 2*table_size;
    table = new std::list<entry_t>[table_size];
    map_size = 0;
    typename std::list<entry_t>::iterator it;
    for (int i = 0; i < old_size; i++)
      for (it = old[i].begin(); it != old[i].end(); ++it)
        insert(it->key, it->val);
    delete[] old;
  }

 public:
  hashmap(int size = 1024): table_size(size), map_size(0) {
    table = new std::list<entry_t>[table_size];
  }

  ~hashmap() { delete[] table; }
  int size() const { return map_size; }

  void insert(const key_t & key, const val_t & val) {
    if (find(key) != 0) return;
    if (map_size >= table_size) rehash();
    unsigned int i = Hash()(key) % table_size;
    table[i].push_back(entry_t(key, val));
    map_size++;
  }

  void remove(const key_t & key) {
    unsigned int i = Hash()(key) % table_size;
    typename std::list<entry_t>::iterator it = table[i].begin();
    while (it != table[i].end() && it->key != key) ++it;
    if (it == table[i].end()) return;
    table[i].erase(it);
    map_size--;
  }

  val_t * find(const key_t & key) {
    unsigned int i = Hash()(key) % table_size;
    typename std::list<entry_t>::iterator it = table[i].begin();
    while (it != table[i].end() && it->key != key) ++it;
    if (it == table[i].end()) return 0;
    return &(it->val);
  }

  val_t & operator [] (const key_t & key) {
    val_t * ret = find(key);
    if (ret != 0) return *ret;
    insert(key, val_t());
    return *find(key);
  }
};

/*** Examples of Hash Algorithm Definitions ***/

#include <string>

struct class_hash {
  unsigned int operator () (int key) {
    return class_hash()((unsigned int)key);
  }

  unsigned int operator () (long long key) {
    return class_hash()((unsigned long long)key);
  }

  //Knuth's multiplicative method (one-to-one)
  unsigned int operator () (unsigned int key) {
    return key * 2654435761u; //or just return key
  }

  //Jenkins's 64-bit hash
  unsigned int operator () (unsigned long long key) {
    key += ~(key << 32); key ^= (key >> 22);
    key += ~(key << 13); key ^= (key >>  8);
    key +=  (key <<  3); key ^= (key >> 15);
    key += ~(key << 27); key ^= (key >> 31);
    return key;
  }

  //Jenkins's one-at-a-time hash
  unsigned int operator () (const std::string & key) {
    unsigned int hash = 0;
    for (unsigned int i = 0; i < key.size(); i++) {
      hash += ((hash += key[i]) << 10);
      hash ^= (hash >> 6);
    }
    hash ^= ((hash += (hash << 3)) >> 11);
    return hash + (hash << 15);
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  hashmap<string, int, class_hash> M;
  M["foo"] = 1;
  M.insert("bar", 2);
  cout << M["foo"] << M["bar"] << endl; //prints 12
  cout << M["baz"] << M["qux"] << endl; //prints 00
  M.remove("foo");
  cout << M.size() << endl;             //prints 3
  cout << M["foo"] << M["bar"] << endl; //prints 02
  return 0;
}
