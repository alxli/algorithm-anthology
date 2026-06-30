/*

Maintain an unordered map: a collection of key-value pairs in which each key appears at most once.
Keys are hashed into table slots, and this implementation resolves collisions using open addressing
with linear probing. Deletions leave tombstones behind so that probe chains remain searchable. It
requires `operator==` on the key type and a hash functor.

Linear probing checks slots $i$, $i + 1$, $i + 2$, ... (modulo the table size) until the desired key
or an empty slot is found. Other probe-step schemes include quadratic probing and double hashing,
but these require additional table-sizing and coprimality conditions to guarantee correct coverage
of the table. They are omitted to keep this implementation focused on core ideas of open addressing.

Compared with the chaining version (2.3.7), open addressing stores entries contiguously, so it is
more cache-friendly and needs no per-entry allocation. The costs: deletions must leave tombstones,
the load factor must stay well below 1, and, unlike chaining or `std::unordered_map`, a pointer from
`find()` or reference from `operator[]` is invalidated as soon as a later insertion rehashes and
relocates the entries.

- `ProbingHashMap<K, V, Hash>()` constructs an empty map.
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
- `std::unordered_map` is the portable standard-library hash map. Unlike this class, it is
  chaining-based rather than open-addressed. Pass a custom hash (see 3.2.1) to harden it against
  adversarial inputs, as its default integer hash is effectively the identity.
- `__gnu_pbds::gp_hash_table` (from GCC's policy-based library) is an open-addressing table that is
  typically several times faster than `std::unordered_map`, but is a non-portable GNU extension.
  Prefer it on GCC-only judges when hashing is the bottleneck; see 8.6 for `HashMap`/`HashSet`
  wrappers with a randomized integer hash.

Time Complexity:
- O(1) per call to the constructor, `size()`, and `empty()`.
- O(1) amortized per call to `insert()`, `erase()`, `find()`, and `operator[]`.
- O(n) per call to `entries()`, where $n$ is the number of entries in the map.

Space Complexity:
- O(n) for storage of the map elements.
- O(1) auxiliary for `insert()` and `operator[]` amortized over a sequence of operations (excluding
  occasional table rehashes).
- O(1) auxiliary for all other operations.

*/

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

template<typename K, typename V, typename Hash>
class ProbingHashMap {
  enum class State { EMPTY, OCCUPIED, DELETED };

  struct HashNode {
    K key;
    V value;
    HashNode(const K &k, const V &v) : key(k), value(v) {}
  };

  std::vector<std::optional<HashNode>> table;
  std::vector<State> state;
  int table_size, num_entries, num_tombstones;

  int bucket(const K &k) const {
    return static_cast<int>(Hash()(k) % static_cast<uint32_t>(table_size));
  }

  void rehash(int new_size) {
    auto old_table = std::move(table);
    auto old_state = std::move(state);
    table_size = new_size;
    table.assign(table_size, std::nullopt);
    state.assign(table_size, State::EMPTY);
    num_entries = 0;
    num_tombstones = 0;
    for (int i = 0; i < static_cast<int>(old_table.size()); i++) {
      if (old_state[i] == State::OCCUPIED) {
        insert(old_table[i]->key, old_table[i]->value);
      }
    }
  }

  void grow_if_needed() {
    // Keep total non-empty slots below 50%. Tombstones count because they lengthen probe chains.
    if (2 * (num_entries + num_tombstones) >= table_size) {
      rehash(2 * table_size);
    }
  }

 public:
  explicit ProbingHashMap(int size = 128)
      : table(size),
        state(size, State::EMPTY),
        table_size(size),
        num_entries(0),
        num_tombstones(0) {}

  int size() const { return num_entries; }
  bool empty() const { return num_entries == 0; }

  bool insert(const K &k, const V &v) {
    grow_if_needed();
    int first_deleted = table_size;
    int i = bucket(k);
    for (int probes = 0; probes < table_size; probes++) {
      if (state[i] == State::OCCUPIED) {
        if (table[i]->key == k) {
          return false;
        }
      } else if (state[i] == State::DELETED) {
        if (first_deleted == table_size) {
          first_deleted = i;
        }
      } else {
        int dest = first_deleted == table_size ? i : first_deleted;
        table[dest].emplace(k, v);
        if (state[dest] == State::DELETED) {
          num_tombstones--;
        }
        state[dest] = State::OCCUPIED;
        num_entries++;
        return true;
      }
      i = (i + 1) % table_size;
    }
    // Should be rare because grow_if_needed keeps slack, but safe.
    rehash(2 * table_size);
    return insert(k, v);
  }

  bool erase(const K &k) {
    int i = bucket(k);
    for (int probes = 0; probes < table_size; probes++) {
      if (state[i] == State::EMPTY) {
        return false;
      }
      if (state[i] == State::OCCUPIED && table[i]->key == k) {
        table[i].reset();
        state[i] = State::DELETED;
        num_entries--;
        num_tombstones++;
        return true;
      }
      i = (i + 1) % table_size;
    }
    return false;
  }

  V *find(const K &k) {
    int i = bucket(k);
    for (int probes = 0; probes < table_size; probes++) {
      if (state[i] == State::EMPTY) {
        return nullptr;
      }
      if (state[i] == State::OCCUPIED && table[i]->key == k) {
        return &table[i]->value;
      }
      i = (i + 1) % table_size;
    }
    return nullptr;
  }

  V &operator[](const K &k) {
    if (V *ptr = find(k); ptr != nullptr) {
      return *ptr;
    }
    grow_if_needed();
    int first_deleted = table_size;
    int i = bucket(k);
    for (int probes = 0; probes < table_size; probes++) {
      if (state[i] == State::DELETED) {
        if (first_deleted == table_size) {
          first_deleted = i;
        }
      } else if (state[i] == State::EMPTY) {
        int dest = first_deleted == table_size ? i : first_deleted;
        table[dest].emplace(k, V());
        if (state[dest] == State::DELETED) {
          num_tombstones--;
        }
        state[dest] = State::OCCUPIED;
        num_entries++;
        return table[dest]->value;
      }
      i = (i + 1) % table_size;
    }
    rehash(2 * table_size);
    return (*this)[k];
  }

  std::vector<std::pair<K, V>> entries() const {
    std::vector<std::pair<K, V>> res;
    res.reserve(num_entries);
    for (int i = 0; i < table_size; i++) {
      if (state[i] == State::OCCUPIED) {
        res.emplace_back(table[i]->key, table[i]->value);
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

  // SplitMix64 mixer (see 3.2.1's mix64).
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
  ProbingHashMap<string, char, Hasher> m;
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
  return 0;
}
