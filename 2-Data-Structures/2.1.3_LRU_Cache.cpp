/*

Maintains a least-recently-used cache with fixed capacity. The cache supports lookups and updates by
key, evicting the least recently used entry whenever an insertion would exceed capacity.

This implementation combines a doubly linked list (`std::list`) with a map from keys to list
iterators. The front of the list stores the most recently used item. The operation
`items.splice(items.begin(), items, it)` is the standard-library way to move an existing list node
to the front in O(1) time.

To implement the same cache without `std::list`, use the intrusive doubly linked list operations
from section 2.1.2. Store a `Node*` in the map instead of a list iterator, move hits to the front
with `move_to_front(&sentinel, node)`, and evict `sentinel.prev`.

- `LRUCache<K, V>(capacity)` constructs an empty cache holding at most `capacity` keys.
- `get(key, &value)` returns whether `key` is present. If present, it stores the value in `value`
  and marks the key as most recently used.
- `put(key, value)` inserts or updates `key`, marking it as most recently used and evicting the
  least recently used key if needed.
- `size()` returns the number of keys currently stored.

Time Complexity:
- O(1) expected amortized per call to `get(key, &value)` and `put(key, value)`, and O(n) in the
  collision-heavy worst case for the hash table.

Space Complexity:
- O(n), where $n$ is the cache capacity.

*/

#include <cassert>
#include <list>
#include <unordered_map>
#include <utility>

// Replacing std::list with a manual doubly-linked list would look something like:
//   struct DListNode { K key; V value; DListNode *prev, *next; };
//   std::unordered_map<K, DListNode*> where;
//
// On get:
//   move_to_front(&sentinel, where[key]);
//
// On eviction:
//   DListNode *old = sentinel.prev;
//   erase(old);
//   where.erase(old->key).

template<typename K, typename V>
class LRUCache {
  using ListIter = typename std::list<std::pair<K, V>>::iterator;

  int cap;
  std::list<std::pair<K, V>> items;
  std::unordered_map<K, ListIter> where;

 public:
  explicit LRUCache(int capacity) : cap(capacity) { assert(capacity >= 0); }

  int size() const { return static_cast<int>(items.size()); }

  bool get(const K &key, V *value) {
    assert(value != nullptr);
    auto it = where.find(key);
    if (it == where.end()) {
      return false;
    }
    items.splice(items.begin(), items, it->second);
    it->second = items.begin();
    *value = it->second->second;
    return true;
  }

  void put(const K &key, const V &value) {
    auto it = where.find(key);
    if (it != where.end()) {
      it->second->second = value;
      items.splice(items.begin(), items, it->second);
      it->second = items.begin();
      return;
    }
    if (cap == 0) {
      return;
    }
    if (static_cast<int>(items.size()) == cap) {
      where.erase(items.back().first);
      items.pop_back();
    }
    items.push_front({key, value});
    where[key] = items.begin();
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  LRUCache<int, int> cache(2);
  int value = 0;
  cache.put(1, 10);
  cache.put(2, 20);
  assert(cache.get(1, &value) && value == 10);
  cache.put(3, 30);  // Evicts key 2.
  assert(!cache.get(2, &value));
  assert(cache.get(3, &value) && value == 30);
  cache.put(1, 11);
  assert(cache.get(1, &value) && value == 11);

  LRUCache<int, int> disabled(0);
  disabled.put(1, 10);
  assert(disabled.size() == 0);
  return 0;
}
