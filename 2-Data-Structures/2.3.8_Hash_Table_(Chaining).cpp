/*

Maintain an unordered map: a collection of key-value pairs in which each key appears at most once.
Keys are hashed into buckets, and this implementation resolves collisions by chaining the entries in
each bucket into a linked list. It requires `operator==` on the key type and a hash functor.

Compared with the open-addressing version (2.3.9), chaining keeps each entry at a stable address: a
rehash never moves existing nodes, so pointers from `find()` and references from `operator[]` stay
valid, and load factors above 1 are tolerated gracefully. The costs are a separate allocation per
entry and cache-unfriendly pointer chasing during traversal.

- `ChainingHashMap()` constructs an empty map.
- `size()` returns the size of the map.
- `empty()` returns whether the map is empty.
- `insert(k, v)` adds an entry with key `k` and value `v` to the map, returning `true` if a new
  entry was added or `false` if the key already exists (in which case the map is unchanged and the
  old value associated with the key is preserved).
- `erase(k)` removes the entry with key `k` from the map, returning `true` if the removal was
  successful or `false` if the key to be removed was not found.
- `find(k)` returns a pointer to the value associated with key `k`, or `nullptr` if the key was not
  found.
- `operator[k]` returns a reference to key `k`'s associated value (which may be modified), or if
  necessary, inserts and returns a new entry with the default constructed value if key `k` was not
  originally found.
- `walk(f)` calls the function `f(k, v)` on each entry of the map, in no guaranteed order.

This is an educational implementation; in practice prefer one of the standard options:
- `std::unordered_map` is the portable standard-library hash map. It is also chaining-based, so this
  class is essentially a transparent version of it. Pass a custom hash (see 3.2.1) to harden it
  against adversarial inputs, as its default integer hash is effectively the identity.
- `__gnu_pbds::gp_hash_table` (from GCC's policy-based library) is an open-addressing table that is
  typically several times faster than `std::unordered_map`, but is a non-portable GNU extension.
  Prefer it on GCC-only judges when hashing is the bottleneck.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `empty()`.
- O(1) amortized per call to `insert()`, `erase()`, `find()`, and `operator[]`.
- O(n) per call to `walk()`, where $n$ is the number of entries in the map.

Space Complexity:
- O(n) for storage of the map elements.
- O(n) auxiliary heap space for `insert()`.
- O(1) auxiliary for all other operations.

*/

#include <cstdint>
#include <list>
#include <vector>

template<class K, class V, class Hash>
class ChainingHashMap {
  struct HashNode {
    K key;
    V value;
    HashNode(const K &k, const V &v) : key(k), value(v) {}
  };

  std::vector<std::list<HashNode>> table;
  int table_size, num_entries;

  void grow_and_rehash() {
    auto old = std::move(table);
    table_size = 2 * table_size;
    table.assign(table_size, std::list<HashNode>());
    num_entries = 0;
    for (auto &bucket : old) {
      for (auto &entry : bucket) {
        insert(entry.key, entry.value);
      }
    }
  }

 public:
  explicit ChainingHashMap(int size = 128) : table(size), table_size(size), num_entries(0) {}

  int size() const { return num_entries; }
  bool empty() const { return num_entries == 0; }

  bool insert(const K &k, const V &v) {
    if (find(k) != nullptr) {
      return false;
    }
    if (num_entries >= table_size) {
      grow_and_rehash();
    }
    uint32_t i = Hash()(k) % table_size;
    table[i].emplace_back(k, v);
    num_entries++;
    return true;
  }

  bool erase(const K &k) {
    uint32_t i = Hash()(k) % table_size;
    auto it = table[i].begin();
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

  V *find(const K &k) {
    uint32_t i = Hash()(k) % table_size;
    auto it = table[i].begin();
    while (it != table[i].end() && !(it->key == k)) {
      ++it;
    }
    if (it == table[i].end()) {
      return nullptr;
    }
    return &(it->value);
  }

  V &operator[](const K &k) {
    if (V *ptr = find(k); ptr != nullptr) {
      return *ptr;
    }
    if (num_entries >= table_size) {
      grow_and_rehash();
    }
    uint32_t i = Hash()(k) % table_size;
    table[i].emplace_back(k, V());
    num_entries++;
    return table[i].back().value;
  }

  template<class Fn>
  void walk(Fn f) const {
    for (int i = 0; i < table_size; i++) {
      for (const auto &entry : table[i]) {
        f(entry.key, entry.value);
      }
    }
  }
};

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
#include <chrono>
#include <string>
using namespace std;

// Example key hashers. For more hash algorithms and overloads, see 3.2.1. To defend against
// adversarially crafted collisions in open-hacking environments, a random seed is added to input
// keys before mixing (as 3.2.1's IntHasher does).
struct Hasher {
  inline static const uint64_t RAND_SEED =
      std::chrono::steady_clock::now().time_since_epoch().count();

  // Signed -> unsigned delegates.
  uint32_t operator()(int k) { return Hasher()(static_cast<uint32_t>(k)); }
  uint32_t operator()(int64_t k) { return Hasher()(static_cast<uint64_t>(k)); }

  // Knuth's multiplicative method. Fast, but affine: an additive RAND_SEED only shifts all buckets
  // uniformly and won't stop crafted collisions (unlike the non-linear hashers below). To harden
  // it, randomize the odd multiplier and take the high bits instead.
  uint32_t operator()(uint32_t k) {
    return k * 2654435761u;  // Or just return k.
  }

  // SplitMix64 finalizer (see 3.2.1's mix64).
  uint32_t operator()(uint64_t k) {
    k += RAND_SEED;
    k = (k ^ (k >> 30)) * 0xbf58476d1ce4e5b9ULL;
    k = (k ^ (k >> 27)) * 0x94d049bb133111ebULL;
    return static_cast<uint32_t>(k ^ (k >> 31));
  }

  // Jenkins's one-at-a-time hash.
  uint32_t operator()(const std::string &k) {
    uint32_t hash = RAND_SEED;
    for (char c : k) {
      hash += ((hash + static_cast<unsigned char>(c)) << 10);
      hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    return hash + (hash << 15);
  }
};

int main() {
  ChainingHashMap<string, char, Hasher> m;
  m["foo"] = 'a';
  m.insert("bar", 'b');
  assert(m["foo"] == 'a');
  assert(m["bar"] == 'b');
  assert(m["baz"] == '\0');
  m["baz"] = 'c';

  string vals;
  m.walk([&](const string &k, char c) { vals += c; });
  sort(vals.begin(), vals.end());
  assert(vals == "abc");
  assert(m.erase("foo"));
  assert(m.size() == 2);
  assert(m["foo"] == '\0');
  assert(m.size() == 3);
  return 0;
}
