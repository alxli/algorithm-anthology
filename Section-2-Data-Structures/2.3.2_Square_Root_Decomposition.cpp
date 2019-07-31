/*

Maintain a fixed-size array (from 0 to size() - 1) while supporting dynamic
queries of contiguous sub-arrays and dynamic updates of individual indices.

The query operation is defined by an associative join_values() function which
satisfies join_values(x, join_values(y, z)) = join_values(join_values(x, y), z)
for all values x, y, and z in the array. The default definition below assumes a
numerical array type, supporting queries for the "min" of the target range.
Another possible query operation is "sum", in which the join_values() function
should be defined to return "a + b".

The update operation is defined by the join_value_with_delta() function which
determines the change made to array values. The default definition below
supports updates that "set" the chosen array index to a new value. Another
possible update operation is "increment", in which join_value_with_delta(v, d)
should be defined to return "v + d".

The operations supported by this data structure are identical to those of the
point update segment tree found in this section.

Time Complexity:
- O(n) per call to both constructors, where n is the size of the array.
- O(1) per call to size().
- O(sqrt n) per call to at(), update(), and query().

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <vector>

template<class T>
class sqrt_decomposition {
  static T join_values(const T &a, const T &b) {
    return std::min(a, b);
  }

  static T join_value_with_delta(const T &v, const T &d) {
    return d;
  }

  int len, blocklen;
  std::vector<T> value, block;

  void init() {
    blocklen = (int)sqrt(len);
    int nblocks = (len + blocklen - 1)/blocklen;
    for (int i = 0; i < nblocks; i++) {
      T blockval = value[i*blocklen];
      int blockhi = std::min(len, (i + 1)*blocklen);
      for (int j = i*blocklen + 1; j < blockhi; j++) {
        blockval = join_values(blockval, value[j]);
      }
      block.push_back(blockval);
    }
  }

 public:
  sqrt_decomposition(int n, const T &v = T()) : len(n), value(n, v) {
    init();
  }

  template<class It>
  sqrt_decomposition(It lo, It hi) : len(hi - lo), value(lo, hi) {
    init();
  }

  int size() const {
    return len;
  }

  T at(int i) const {
    return query(i, i);
  }

  T query(int lo, int hi) const {
    T res;
    int blocklo = ceil((double)lo/blocklen), blockhi = (hi + 1)/blocklen - 1;
    if (blocklo > blockhi) {
      res = value[lo];
      for (int i = lo + 1; i <= hi; i++) {
        res = join_values(res, value[i]);
      }
    } else {
      res = block[blocklo];
      for (int i = blocklo + 1; i <= blockhi; i++) {
        res = join_values(res, block[i]);
      }
      for (int i = lo; i < blocklo*blocklen; i++) {
        res = join_values(res, value[i]);
      }
      for (int i = (blockhi + 1)*blocklen; i <= hi; i++) {
        res = join_values(res, value[i]);
      }
    }
    return res;
  }

  void update(int i, const T &d) {
    value[i] = join_value_with_delta(value[i], d);
    int b = i/blocklen;
    int blockhi = std::min(len, (b + 1)*blocklen);
    block[b] = value[b*blocklen];
    for (int i = b*blocklen + 1; i < blockhi; i++) {
      block[b] = join_values(block[b], value[i]);
    }
  }
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  int arr[5] = {6, -2, 1, 8, 10};
  sqrt_decomposition<int> sd(arr, arr + 5);
  sd.update(2, 4);
  cout << "Values:";
  for (int i = 0; i < sd.size(); i++) {
    cout << " " << sd.at(i);
  }
  cout << endl;
  assert(sd.query(0, 3) == -2);
  return 0;
}
