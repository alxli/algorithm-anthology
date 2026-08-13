/*

Maintain an unordered map: a collection of key-value pairs in which each key appears at most once.
Keys are hashed into buckets, and this implementation resolves collisions by chaining the entries in
each bucket into a linked list. The hash and key-equality policies follow the conventions of
`std::unordered_map`.

Compared with the open-addressing version (2.3.8), chaining keeps each entry at a stable address: a
rehash never moves existing nodes, so pointers from `find()` and references from `operator[]` stay
valid, and load factors above 1 are tolerated gracefully. The costs are a separate allocation per
entry and cache-unfriendly pointer chasing during traversal.

- `ChainingHashMap<K, V>(buckets = 128)` constructs an empty map with the positive number of buckets
  given by `buckets`, using `std::hash<K>` and `std::equal_to<K>`.
- `ChainingHashMap<K, V, Hash, KeyEqual>(buckets, hash, equal)` instead stores the supplied hash and
  equality policies.
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
- `entries()` returns all key-value entries in no guaranteed order.

This is an educational implementation; in practice prefer one of the standard options:
- `std::unordered_map` is the portable standard-library hash map. It is also chaining-based, so this
  class is essentially a transparent version of it. Pass a custom hash (see 3.2.1) to harden it
  against adversarial inputs, as its default integer hash is effectively the identity.
- `__gnu_pbds::gp_hash_table` (from GCC's policy-based library) is an open-addressing table that is
  typically several times faster than `std::unordered_map`, but is a non-portable GNU extension.
  Prefer it on GCC-only judges when hashing is the bottleneck; see 8.6 for `HashMap`/`HashSet`
  wrappers with a randomized integer hash.

Time Complexity:
- O(b) per call to the constructor, where $b$ is the initial number of buckets.
- O(1) per call to `size()` and `empty()`.
- O(1) expected amortized per call to `insert()`, `erase()`, `find()`, and `operator[]`, and O(n) in
  the collision-heavy worst case.
- O(n + b) per call to `entries()`, where $n$ is the number of entries and $b$ is the number of
  buckets.

Space Complexity:
- O(n + b) for storage of the map elements and buckets.
- O(n + b) auxiliary during a rehash.
- O(n) for the vector returned by `entries()`.
- O(1) auxiliary for all other operations.

*/

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <list>
#include <utility>
#include <vector>

template<typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
class ChainingHashMap {
  struct HashNode {
    K key;
    V value;
    HashNode(const K &k, const V &v) : key(k), value(v) {}
  };

  std::vector<std::list<HashNode>> table;
  int table_size, num_entries;
  Hash hash;
  KeyEqual equal;

  int bucket(const K &k) const {
    return static_cast<int>(hash(k) % static_cast<std::size_t>(table_size));
  }

  void grow_and_rehash() {
    auto old = std::move(table);
    table_size = 2 * table_size;
    table.assign(table_size, std::list<HashNode>{});
    for (auto &chain : old) {
      while (!chain.empty()) {
        auto it = chain.begin();
        int i = bucket(it->key);
        table[i].splice(table[i].end(), chain, it);
      }
    }
  }

 public:
  explicit ChainingHashMap(int buckets = 128, Hash hash = Hash{}, KeyEqual equal = KeyEqual{})
      : table_size(buckets), num_entries(0), hash(std::move(hash)), equal(std::move(equal)) {
    assert(buckets > 0);
    table.resize(buckets);
  }

  int size() const { return num_entries; }
  bool empty() const { return num_entries == 0; }

  bool insert(const K &k, const V &v) {
    if (find(k) != nullptr) {
      return false;
    }
    if (num_entries >= table_size) {
      grow_and_rehash();
    }
    int i = bucket(k);
    table[i].emplace_back(k, v);
    num_entries++;
    return true;
  }

  bool erase(const K &k) {
    int i = bucket(k);
    auto it = table[i].begin();
    while (it != table[i].end() && !equal(it->key, k)) {
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
    int i = bucket(k);
    auto it = table[i].begin();
    while (it != table[i].end() && !equal(it->key, k)) {
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
    int i = bucket(k);
    table[i].emplace_back(k, V{});
    num_entries++;
    return table[i].back().value;
  }

  std::vector<std::pair<K, V>> entries() const {
    std::vector<std::pair<K, V>> res;
    res.reserve(num_entries);
    for (int i = 0; i < table_size; i++) {
      for (const auto &[k, v] : table[i]) {
        res.emplace_back(k, v);
      }
    }
    return res;
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
  inline static const uint64_t RAND_SEED = chrono::steady_clock::now().time_since_epoch().count();

  // Signed -> unsigned delegates.
  uint32_t operator()(int k) const { return Hasher{}(static_cast<uint32_t>(k)); }
  uint32_t operator()(int64_t k) const { return Hasher{}(static_cast<uint64_t>(k)); }

  // Knuth's multiplicative method. Fast, but affine: an additive RAND_SEED only shifts all buckets
  // uniformly and won't stop crafted collisions (unlike the non-linear hashers below). To harden
  // it, randomize the odd multiplier and take the high bits instead.
  uint32_t operator()(uint32_t k) const {
    return k * 2654435761U;  // Or just return k.
  }

  // SplitMix64 mixer (see 3.2.1's mix64).
  uint32_t operator()(uint64_t k) const {
    k += RAND_SEED;
    k = (k ^ (k >> 30)) * 0xbf58476d1ce4e5b9ULL;
    k = (k ^ (k >> 27)) * 0x94d049bb133111ebULL;
    return static_cast<uint32_t>(k ^ (k >> 31));
  }

  // Jenkins's one-at-a-time hash.
  uint32_t operator()(const string &k) const {
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

struct AbsHash {
  int salt;
  explicit AbsHash(int salt = 0) : salt(salt) {}
  size_t operator()(int x) const { return (x < 0 ? -static_cast<int64_t>(x) : x) + salt; }
};

struct AbsEqual {
  bool operator()(int a, int b) const { return AbsHash{}(a) == AbsHash{}(b); }
};

int main() {
  ChainingHashMap<string, char, Hasher> m;
  assert(m.empty());
  m["foo"] = 'a';
  assert(m.insert("bar", 'b'));
  assert(!m.insert("bar", 'z'));
  assert(!m.empty() && m.size() == 2);
  assert(m["foo"] == 'a');
  assert(m["bar"] == 'b');
  assert(m.find("foo") != nullptr && *m.find("foo") == 'a');
  assert(m.find("qux") == nullptr);
  assert(m["baz"] == '\0');
  m["baz"] = 'c';

  string vals;
  for (const auto &[k, v] : m.entries()) {
    vals += v;
  }
  sort(vals.begin(), vals.end());
  assert(vals == "abc");
  assert(m.erase("foo"));
  assert(m.size() == 2);
  assert(m["foo"] == '\0');
  assert(m.size() == 3);

  ChainingHashMap<string, char, Hasher> stable(1);
  stable["x"] = 'x';
  char *ptr = stable.find("x");
  stable["y"] = 'y';  // Rehashes without relocating the existing list node.
  assert(ptr == stable.find("x") && *ptr == 'x');

  ChainingHashMap<string, int> defaults;
  defaults["answer"] = 42;
  assert(*defaults.find("answer") == 42);

  ChainingHashMap<int, char, AbsHash, AbsEqual> absolute(128, AbsHash(7), AbsEqual{});
  assert(absolute.insert(-2, 'x'));
  assert(!absolute.insert(2, 'y') && *absolute.find(2) == 'x');
  return 0;
}
