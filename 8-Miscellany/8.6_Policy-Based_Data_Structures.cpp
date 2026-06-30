/*

GNU policy-based data structures provide non-standard associative containers and priority queues,
including order-statistic trees and fast hash tables. They are not part of the C++ standard library,
but they are available on many GNU C++ contest judges.

- `HashMap<K, V>` and `HashSet<K>` behave like faster, non-standard alternatives to
  `std::unordered_map<K, V>` and `std::unordered_set<K>`, using `gp_hash_table`. The default
  `SplitMix64Hasher` is for integer-like keys; pass a custom hash for other key types. It randomizes
  the hash seed to avoid weak adversarial integer hashes.
- `OrderedSet<T>` and `OrderedMap<K, V>` behave like `std::set<T>` and `std::map<K, V>` but also
  support `find_by_order(k)` and `order_of_key(x)`.
- `OrderedMultiset<T>` behaves like `std::multiset<T>` by storing duplicates as (value, unique ID)
  pairs.
- `OrderedMultimap<K, V>` behaves like `std::multimap<K, V>` by storing duplicate keys as (key,
  unique ID) pairs.
- `PairingHeap<T>` is a meldable priority queue; `push()` returns an iterator that can be passed to
  `modify()` or `erase()`.
- `BinaryHeap<T>`, `BinomialHeap<T>`, and `RcBinomialHeap<T>` are alternative GNU priority queue
  tags with the same interface.

The order-statistic operations use GNU PBDS conventions: `find_by_order(k)` is 0-indexed and
`order_of_key(x)` returns the number of keys strictly less than `x`, even if `x` is absent. Raw
PBDS ordered trees return iterators from `find_by_order(k)`; the multiset and multimap wrappers
below unwrap that iterator into a value. The priority queue `join()` operation melds another heap
into the current one. Since this depends on `__gnu_pbds`, keep a standard library fallback in mind
when portability matters.

Time Complexity:
- `HashMap`/`HashSet`: same expected bounds as `std::unordered_map`/`std::unordered_set` (expected
  O(1), worst-case O(n)).
- `OrderedSet`/`OrderedMap`: same as `std::set`/`std::map`, with O(log n) for `find_by_order` and
  `order_of_key`.
- `OrderedMultiset`/`OrderedMultimap`: same as `std::multiset`/`std::multimap`, with O(log n) for
  `find_by_order` and `order_of_key`.
- O(1) amortized per call to `push()` and `join()` for `PairingHeap`.
- O(log n) amortized per call to `pop()`, `modify()`, and `erase()` for `PairingHeap`.

Space Complexity:
- O(n) for all containers.

*/

#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/priority_queue.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <functional>
#include <utility>

struct SplitMix64Hasher {
  // SplitMix64 mixer.
  static uint64_t mix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
  }

  std::size_t operator()(uint64_t x) const {
    static const uint64_t FIXED_RANDOM =
        std::chrono::steady_clock::now().time_since_epoch().count();
    return mix64(x + FIXED_RANDOM);
  }
};

template<typename K, typename Hash = SplitMix64Hasher>
using HashSet = __gnu_pbds::gp_hash_table<K, __gnu_pbds::null_type, Hash>;

template<typename K, typename V, typename Hash = SplitMix64Hasher>
using HashMap = __gnu_pbds::gp_hash_table<K, V, Hash>;

template<typename K, typename V, typename Compare = std::less<K>>
using OrderedMap = __gnu_pbds::tree<
    K, V, Compare, __gnu_pbds::rb_tree_tag, __gnu_pbds::tree_order_statistics_node_update>;

template<typename T, typename Compare = std::less<T>>
using OrderedSet = OrderedMap<T, __gnu_pbds::null_type, Compare>;

template<typename T>
class OrderedMultiset {
  OrderedSet<std::pair<T, int>> tree;
  int timer = 0;

 public:
  void insert(const T &x) { tree.insert({x, timer++}); }

  bool erase_one(const T &x) {
    auto it = tree.lower_bound({x, -1});
    if (it == tree.end() || it->first != x) {
      return false;
    }
    tree.erase(it);
    return true;
  }

  T find_by_order(int k) const {
    assert(0 <= k && k < size());
    return tree.find_by_order(k)->first;
  }

  int order_of_key(const T &x) const { return tree.order_of_key({x, -1}); }
  int size() const { return static_cast<int>(tree.size()); }
};

template<typename K, typename V>
class OrderedMultimap {
  OrderedMap<std::pair<K, int>, V> tree;
  int timer = 0;

 public:
  void insert(const K &key, const V &value) { tree.insert({{key, timer++}, value}); }

  bool erase_one(const K &key) {
    auto it = tree.lower_bound({key, -1});
    if (it == tree.end() || it->first.first != key) {
      return false;
    }
    tree.erase(it);
    return true;
  }

  std::pair<K, V> find_by_order(int k) const {
    assert(0 <= k && k < size());
    auto it = tree.find_by_order(k);
    return {it->first.first, it->second};
  }

  int order_of_key(const K &x) const { return tree.order_of_key({x, -1}); }
  int size() const { return static_cast<int>(tree.size()); }
};

template<typename T, typename Compare = std::less<T>>
using PairingHeap = __gnu_pbds::priority_queue<T, Compare, __gnu_pbds::pairing_heap_tag>;

template<typename T, typename Compare = std::less<T>>
using BinaryHeap = __gnu_pbds::priority_queue<T, Compare, __gnu_pbds::binary_heap_tag>;

template<typename T, typename Compare = std::less<T>>
using BinomialHeap = __gnu_pbds::priority_queue<T, Compare, __gnu_pbds::binomial_heap_tag>;

template<typename T, typename Compare = std::less<T>>
using RcBinomialHeap = __gnu_pbds::priority_queue<T, Compare, __gnu_pbds::rc_binomial_heap_tag>;

/*** Example Usage ***/

int main() {
  OrderedSet<int> s;
  s.insert(10);
  s.insert(20);
  s.insert(30);
  s.insert(20);  // Duplicate ignored, like std::set.
  // Net new over std::set: order-statistic queries.
  assert(s.size() == 3);
  assert(*s.find_by_order(0) == 10);
  assert(*s.find_by_order(1) == 20);
  assert(s.order_of_key(25) == 2);
  assert(s.order_of_key(100) == 3);

  HashMap<int, int> hm;
  hm[10] = 1;
  hm[20] = 2;
  assert(hm[10] == 1);
  HashSet<int> hs;
  hs.insert(7);
  assert(hs.find(7) != hs.end());

  OrderedMultiset<int> ms;
  ms.insert(5);
  ms.insert(5);
  ms.insert(7);
  assert(ms.size() == 3);
  // Net new over std::multiset: order statistics with duplicates.
  assert(ms.order_of_key(7) == 2);
  assert(ms.find_by_order(1) == 5);
  assert(ms.erase_one(5));
  assert(ms.order_of_key(7) == 1);
  assert(ms.erase_one(5));
  assert(!ms.erase_one(5));
  assert(ms.find_by_order(0) == 7);

  OrderedMultimap<int, char> mp;
  mp.insert(5, 'a');
  mp.insert(5, 'b');
  mp.insert(7, 'c');
  // Net new over std::multimap: order statistics with duplicate keys.
  assert(mp.order_of_key(7) == 2);
  assert(mp.find_by_order(1).first == 5);
  assert(mp.erase_one(5));
  assert(mp.order_of_key(7) == 1);
  assert(mp.erase_one(5));
  assert(!mp.erase_one(5));
  assert(mp.find_by_order(0) == std::make_pair(7, 'c'));

  PairingHeap<int> h1, h2;
  auto it = h1.push(10);
  auto erased = h1.push(5);
  h1.modify(it, 20);  // Net new over std::priority_queue: update through a handle.
  h1.erase(erased);   // Also net new: remove an arbitrary heap item through a handle.
  h2.push(7);
  h1.join(h2);  // Also net new: meld another heap into this one.
  assert(h2.empty());
  assert(h1.top() == 20);
  h1.pop();
  assert(h1.top() == 7);
  return 0;
}
