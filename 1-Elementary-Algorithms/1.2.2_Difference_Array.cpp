/*

Applies many range-add updates to an array in linear total time using a difference array. Instead of
immediately adding `delta` to every element of `[lo, hi)`, add `delta` at `diff[lo]` and subtract it at
`diff[hi]`; a final prefix sum reconstructs the updated values.

- `DifferenceArray(n)` constructs an initially zero array of size `n`.
- `add(lo, hi, delta)` adds `delta` to every position in the half-open range `[lo, hi)`.
- `build()` returns the final array after all range updates.

Time Complexity:
- O(1) per call to `add(lo, hi, delta)`.
- O(n) per call to `build()`.

Space Complexity:
- O(n) for the difference array.

*/

#include <vector>

class DifferenceArray {
  std::vector<long long> diff;

 public:
  explicit DifferenceArray(int n) : diff(n + 1, 0) {}

  void add(int lo, int hi, long long delta) {
    diff[lo] += delta;
    diff[hi] -= delta;
  }

  std::vector<long long> build() const {
    std::vector<long long> res(diff.size() - 1);
    long long cur = 0;
    for (int i = 0; i < static_cast<int>(res.size()); i++) {
      cur += diff[i];
      res[i] = cur;
    }
    return res;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  DifferenceArray d(5);
  d.add(0, 3, 2);
  d.add(1, 5, 4);
  d.add(2, 4, -1);
  vector<long long> a = d.build();
  vector<long long> expected{2, 6, 5, 3, 4};
  for (int i = 0; i < static_cast<int>(expected.size()); i++) {
    assert(a[i] == expected[i]);
  }
  return 0;
}
