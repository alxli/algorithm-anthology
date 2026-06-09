/*

These functions are equivalent to `std::rotate()`, taking three iterators `lo`, `mid`, and `hi`
(`lo` $\leq$ `mid` $\leq$ `hi`) to perform a left rotation on the range `[lo, hi)`. After the
function call, `[lo, hi)` will consist of the concatenation of elements originally in `[mid, hi)` +
`[lo, mid)`. That is, the range `[lo, hi)` will be rearranged in such a way that the element at
`mid` becomes the first element of the new range and the element at `mid - 1` becomes the last
element, all while preserving the relative ordering of elements within the two rotated subarrays.

All three versions below achieve the same result using in-place algorithms.

- `rotate1(lo, mid, hi)` uses a straightforward swapping algorithm requiring ForwardIterators.
- `rotate2(lo, mid, hi)` requires BidirectionalIterators, employing a well-known trick with three
  simple inversions.
- `rotate3(lo, mid, hi)` requires random-access iterators, applying a juggling algorithm which first
  divides the range into `gcd(hi - lo, mid - lo)` sets and then rotates the corresponding elements
  in each set.

Time Complexity:
- O(n) per call to all versions, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(1) auxiliary for all versions

*/

#include <algorithm>

template<class It>
void rotate1(It lo, It mid, It hi) {
  if (lo == mid || mid == hi) {
    return;
  }
  It next = mid;
  while (lo != next) {
    std::iter_swap(lo++, next++);
    if (next == hi) {
      next = mid;
    } else if (lo == mid) {
      mid = next;
    }
  }
}

template<class It>
void rotate2(It lo, It mid, It hi) {
  if (lo == mid || mid == hi) {
    return;
  }
  std::reverse(lo, mid);
  std::reverse(mid, hi);
  std::reverse(lo, hi);
}

int gcd(int a, int b) {
  return (b == 0) ? a : gcd(b, a % b);
}

template<class It>
void rotate3(It lo, It mid, It hi) {
  if (lo == mid || mid == hi) {
    return;
  }
  int n = hi - lo, jump = mid - lo;
  int g = gcd(jump, n), cycle = n / g;
  for (int i = 0; i < g; i++) {
    int curr = i, next;
    for (int j = 0; j < cycle - 1; j++) {
      next = curr + jump;
      if (next >= n) {
        next -= n;
      }
      std::iter_swap(lo + curr, lo + next);
      curr = next;
    }
  }
}

/*** Example Usage and Output:

before sort:  2 4 2 0 5 10 7 3 7 1
after sort:   0 1 2 2 3 4 5 7 7 10
rotate left:  1 2 2 3 4 5 7 7 10 0
rotate right: 0 1 2 2 3 4 5 7 7 10

***/

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  vector<int> v0, v1, v2, v3;
  for (int i = 0; i < 10000; i++) {
    v0.push_back(i);
  }
  v1 = v2 = v3 = v0;
  int mid = 5678;
  std::rotate(v0.begin(), v0.begin() + mid, v0.end());
  rotate1(v1.begin(), v1.begin() + mid, v1.end());
  rotate2(v2.begin(), v2.begin() + mid, v2.end());
  rotate3(v3.begin(), v3.begin() + mid, v3.end());
  assert(v0 == v1 && v0 == v2 && v0 == v3);

  // Example from: http://en.cppreference.com/w/cpp/algorithm/rotate
  vector<int> v{2, 4, 2, 0, 5, 10, 7, 3, 7, 1};
  cout << "before sort:  ";
  for (const auto &x : v) {
    cout << x << " ";
  }
  cout << endl;

  // Insertion sort.
  for (auto i = v.begin(); i != v.end(); ++i) {
    rotate1(std::upper_bound(v.begin(), i, *i), i, i + 1);
  }
  cout << "after sort:   ";
  for (const auto &x : v) {
    cout << x << " ";
  }
  cout << endl;

  // Simple rotation to the left.
  rotate2(v.begin(), v.begin() + 1, v.end());
  cout << "rotate left:  ";
  for (const auto &x : v) {
    cout << x << " ";
  }
  cout << endl;

  // Simple rotation to the right.
  rotate3(v.rbegin(), v.rbegin() + 1, v.rend());
  cout << "rotate right: ";
  for (const auto &x : v) {
    cout << x << " ";
  }
  cout << endl;
  return 0;
}
