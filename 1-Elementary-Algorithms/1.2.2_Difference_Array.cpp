/*

Applies many range-add updates to an array in linear total time using a difference
array. Instead of immediately adding `delta` to every element of `[l, r)`, add
`delta` at `diff[l]` and subtract it at `diff[r]`; a final prefix sum reconstructs
the updated values.

- `difference_array(n)` constructs an initially zero array of size `n`.
- `add(l, r, delta)` adds `delta` to every position in the half-open range
  `[l, r)`.
- `build()` returns the final array after all range updates.

Time Complexity:
- O(1) per call to `add(l, r, delta)`.
- O(n) per call to `build()`.

Space Complexity:
- O(n) for the difference array.

*/

#include <vector>

class difference_array {
  std::vector<long long> diff;

 public:
  explicit difference_array(int n) : diff(n + 1, 0) {}

  void add(int l, int r, long long delta) {
    diff[l] += delta;
    diff[r] -= delta;
  }

  std::vector<long long> build() const {
    std::vector<long long> res(diff.size() - 1);
    long long cur = 0;
    for (int i = 0; i < (int)res.size(); i++) {
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
  difference_array d(5);
  d.add(0, 3, 2);
  d.add(1, 5, 4);
  d.add(2, 4, -1);
  vector<long long> a = d.build();
  long long expected[] = {2, 6, 5, 3, 4};
  for (int i = 0; i < 5; i++) {
    assert(a[i] == expected[i]);
  }
  return 0;
}
