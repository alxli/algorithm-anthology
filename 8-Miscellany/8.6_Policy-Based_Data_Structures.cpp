/*

GNU policy-based data structures provide ordered sets and maps with order statistics. They are not
part of the C++ standard library, but they are available on many GNU C++ contest judges.

- `ordered_set<T>` behaves like a set with `find_by_order(k)` and `order_of_key(x)`.
- `ordered_map<K, V>` behaves like a map with `find_by_order(k)` and `order_of_key(x)`.
- `ordered_multiset<T>` stores duplicates by pairing each value with a unique id.
- `ordered_multimap<K, V>` stores duplicate keys by pairing each key with a unique id.
- `PairingHeap<T>` is a meldable priority queue; `push()` returns an iterator that can be passed to
  `modify()` or `erase()`.
- `BinaryHeap<T>`, `BinomialHeap<T>`, and `RcBinomialHeap<T>` are alternative GNU priority queue
  tags with the same interface.

The order statistic operations are 0-indexed. The priority queue `join()` operation melds another
heap into the current one. Since this depends on `__gnu_pbds`, keep a standard library fallback in
mind when portability matters.

Time Complexity:
- O(log n) per operation.

Space Complexity:
- O(n).

*/

#include <cassert>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/priority_queue.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <functional>
#include <utility>

template<class K, class V, class Compare = std::less<K>>
using ordered_map = __gnu_pbds::tree<
    K, V, Compare, __gnu_pbds::rb_tree_tag, __gnu_pbds::tree_order_statistics_node_update>;

template<class T, class Compare = std::less<T>>
using ordered_set = ordered_map<T, __gnu_pbds::null_type, Compare>;

template<class T>
class ordered_multiset {
  ordered_set<std::pair<T, int>> tree;
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

  int order_of_key(const T &x) const { return tree.order_of_key({x, -1}); }
  T find_by_order(int k) const { return tree.find_by_order(k)->first; }
  int size() const { return static_cast<int>(tree.size()); }
};

template<class K, class V>
class ordered_multimap {
  ordered_map<std::pair<K, int>, V> tree;
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

  int order_of_key(const K &key) const { return tree.order_of_key({key, -1}); }

  std::pair<K, V> find_by_order(int k) const {
    auto it = tree.find_by_order(k);
    return {it->first.first, it->second};
  }

  int size() const { return static_cast<int>(tree.size()); }
};

template<class T, class Compare = std::less<T>>
using PairingHeap = __gnu_pbds::priority_queue<T, Compare, __gnu_pbds::pairing_heap_tag>;

template<class T, class Compare = std::less<T>>
using BinaryHeap = __gnu_pbds::priority_queue<T, Compare, __gnu_pbds::binary_heap_tag>;

template<class T, class Compare = std::less<T>>
using BinomialHeap = __gnu_pbds::priority_queue<T, Compare, __gnu_pbds::binomial_heap_tag>;

template<class T, class Compare = std::less<T>>
using RcBinomialHeap = __gnu_pbds::priority_queue<T, Compare, __gnu_pbds::rc_binomial_heap_tag>;

/*** Example Usage ***/

int main() {
  ordered_set<int> s;
  s.insert(10);
  s.insert(20);
  s.insert(30);
  assert(*s.find_by_order(1) == 20);
  assert(s.order_of_key(25) == 2);

  ordered_multiset<int> ms;
  ms.insert(5);
  ms.insert(5);
  ms.insert(7);
  assert(ms.size() == 3);
  assert(ms.order_of_key(7) == 2);
  assert(ms.find_by_order(1) == 5);
  assert(ms.erase_one(5));
  assert(ms.order_of_key(7) == 1);

  ordered_multimap<int, char> mp;
  mp.insert(5, 'a');
  mp.insert(5, 'b');
  mp.insert(7, 'c');
  assert(mp.order_of_key(7) == 2);
  assert(mp.find_by_order(1).first == 5);
  assert(mp.erase_one(5));
  assert(mp.order_of_key(7) == 1);

  PairingHeap<int> h1, h2;
  auto it = h1.push(10);
  h1.push(5);
  h1.modify(it, 20);
  h2.push(7);
  h1.join(h2);
  assert(h1.top() == 20);
  h1.pop();
  assert(h1.top() == 7);
  return 0;
}
