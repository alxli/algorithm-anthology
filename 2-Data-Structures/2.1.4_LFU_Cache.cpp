/*

Maintains a least-frequently-used cache with fixed capacity. The cache supports lookups and updates
by key, evicting the entry with the fewest accesses whenever an insertion would exceed capacity, and
breaking ties among equally frequent entries by evicting the least recently used of them.

The difficulty compared to the least-recently-used cache of section 2.1.3 is that frequencies form
an unbounded set of buckets rather than a single ordering, so a naive implementation scans for the
minimum on every eviction. The fix is to bucket the keys by frequency, keeping one list of keys per
frequency in most-recently-used order, and to track the smallest occupied frequency. Every access
moves a key from bucket $f$ to bucket $f + 1$, which is O(1) with an iterator stored per key.

The tracked minimum needs no search either. An insertion always creates a key of frequency $1$, so
the minimum becomes $1$; an access raises exactly one key from the minimum bucket, so the minimum
can only rise by one, and it does so only when that bucket empties. Both updates are O(1), which is
what makes the whole structure constant time.

- `LFUCache<K, V>(capacity)` constructs an empty cache holding at most `capacity` keys, which must
  be positive. The key type `K` must support `operator==` and have `std::hash<K>` defined.
- `size()` returns the number of keys currently stored.
- `freq(key)` returns the number of accesses counted for `key`, or $0$ if it is absent.
- `get(key, &value)` returns whether `key` is present. On success, it copies the associated value
  into `value` and counts an access.
- `put(key, value)` inserts or updates `key`, counts an access, and evicts the least frequently used
  key if the insertion would exceed capacity.

Frequencies grow continually, so a long-lived cache can be held hostage by keys popular only once;
real implementations periodically halve every count, ageing the statistics without reordering them.
This implementation uses `int64_t` counts and asserts before their theoretical overflow.

Time Complexity:
- O(1) expected amortized per call to `get()`, `put()`, and `freq()`, and O(n) in the
  collision-heavy worst case for the hash table.
- O(1) per call to `size()`.

Space Complexity:
- O(n), where $n$ is the cache capacity.

*/

#include <cassert>
#include <cstdint>
#include <list>
#include <unordered_map>

template<typename K, typename V>
class LFUCache {
  struct Entry {
    V value;
    int64_t freq;
    typename std::list<K>::iterator pos;
  };

  int cap;
  int64_t min_freq;
  std::unordered_map<K, Entry> entries;
  std::unordered_map<int64_t, std::list<K>> buckets;  // Each frequency, newest at the front.

  // Moves an existing key from its bucket to the next one up, counting one access.
  void touch(const K &key) {
    Entry &entry = entries[key];
    assert(entry.freq < INT64_MAX);
    std::list<K> &bucket = buckets[entry.freq];
    bucket.erase(entry.pos);
    if (bucket.empty()) {
      buckets.erase(entry.freq);
      if (min_freq == entry.freq) {
        min_freq++;
      }
    }
    entry.freq++;
    buckets[entry.freq].push_front(key);
    entry.pos = buckets[entry.freq].begin();
  }

 public:
  explicit LFUCache(int capacity) : cap(capacity), min_freq(0) { assert(cap > 0); }

  LFUCache(const LFUCache &) = delete;
  LFUCache &operator=(const LFUCache &) = delete;
  int size() const { return static_cast<int>(entries.size()); }

  int64_t freq(const K &key) const {
    auto it = entries.find(key);
    return it == entries.end() ? 0 : it->second.freq;
  }

  bool get(const K &key, V *value) {
    auto it = entries.find(key);
    if (it == entries.end()) {
      return false;
    }
    if (value != nullptr) {
      *value = it->second.value;
    }
    touch(key);
    return true;
  }

  void put(const K &key, const V &value) {
    if (entries.count(key) > 0) {
      entries[key].value = value;
      touch(key);
      return;
    }
    if (size() == cap) {
      // The back of the minimum bucket is the least recently used of the least frequent keys.
      entries.erase(buckets[min_freq].back());
      buckets[min_freq].pop_back();
      if (buckets[min_freq].empty()) {
        buckets.erase(min_freq);
      }
    }
    min_freq = 1;
    buckets[1].push_front(key);
    entries[key] = Entry{value, 1, buckets[1].begin()};
  }
};

/*** Example Usage ***/

#include <string>
using namespace std;

int main() {
  LFUCache<int, string> cache(2);
  string value;
  assert(!cache.get(1, &value));

  cache.put(1, "one");
  cache.put(2, "two");
  assert(cache.size() == 2);
  assert(cache.get(1, &value) && value == "one");
  assert(cache.freq(1) == 2 && cache.freq(2) == 1);

  cache.put(3, "three");  // Key 2 is the least frequently used, so it is evicted.
  assert(!cache.get(2, &value));
  assert(cache.get(3, &value) && value == "three");

  // Keys 1 and 3 now both have frequency 2, so the least recently used of the two goes next.
  assert(cache.freq(1) == 2 && cache.freq(3) == 2);
  cache.put(4, "four");
  assert(!cache.get(1, &value));
  assert(cache.get(3, &value) && value == "three");

  // Updating a stored key counts as an access rather than an insertion.
  cache.put(3, "tres");
  assert(cache.get(3, &value) && value == "tres" && cache.size() == 2);
  return 0;
}
