/*

Finds candidate frequent elements in a stream using the Misra-Gries algorithm. With parameter `k`,
the algorithm keeps at most `k - 1` counters and guarantees that every value occurring more than
$\lfloor n/k \rfloor$ times appears among the returned candidates.

The candidates are not automatically verified, since the algorithm intentionally uses sublinear
memory and does not retain the stream. If exact frequencies are needed, make a second pass over the
input and count only the returned candidates.

- `misra_gries(lo, hi, k)` returns a map of candidate values to their residual counters. Use `k = 2`
  for the Boyer-Moore majority-candidate special case.

Time Complexity:
- O(nk) worst-case for this compact map-based implementation, where $n$ is the number of stream
  values.

Space Complexity:
- O(k) auxiliary heap space.

*/

#include <map>
#include <vector>

template<class T, class It>
std::map<T, int> misra_gries(It lo, It hi, int k) {
  typedef std::map<T, int> counter_map;
  counter_map count;
  for (It it = lo; it != hi; ++it) {
    typename counter_map::iterator found = count.find(*it);
    if (found != count.end()) {
      found->second++;
    } else if ((int)count.size() < k - 1) {
      count[*it] = 1;
    } else {
      std::vector<T> erase;
      for (typename counter_map::iterator jt = count.begin(); jt != count.end(); ++jt) {
        if (--jt->second == 0) {
          erase.push_back(jt->first);
        }
      }
      for (int i = 0; i < (int)erase.size(); i++) {
        count.erase(erase[i]);
      }
    }
  }
  return count;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  int raw[] = {1, 2, 1, 3, 1, 2, 1, 4, 2, 2, 2};
  map<int, int> candidates = misra_gries<int>(raw, raw + 11, 3);
  assert(candidates.count(1));
  assert(candidates.count(2));
  assert(!candidates.count(3));
  return 0;
}
