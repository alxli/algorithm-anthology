/*

These functions are equivalent to std::rotate(), taking three iterators lo, mid,
and hi (lo <= mid <= hi) to perform a left rotation on the range [lo, hi). After
the function call, [lo, hi) will comprise of the concatenation of the elements
originally in [mid, hi) + [lo, mid). That is, the range [lo, hi) will be
rearranged in such a way that the element at mid becomes the first element of
the new range and the element at mid - 1 becomes the last element, all while
preserving the relative ordering of elements within the two rotated subarrays.

All three versions below achieve the same result using in-place algorithms.
Version 1 uses a straightforward swapping algorithm requiring ForwardIterators.
Version 2 requires BidirectionalIterators, employing a well-known trick with
three simple inversions. Version 3 requires random-access iterators, applying a
juggling algorithm which first divides the range into gcd(hi - lo, mid - lo)
sets and then rotates the corresponding elements in each set.

Time Complexity:
- O(n) per call to both functions, where n is the distance between lo and hi.

Space Complexity:
- O(1) auxiliary for all versions

*/

#include <algorithm>

template<class It>
void rotate1(It lo, It mid, It hi) {
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
  std::reverse(lo, mid);
  std::reverse(mid, hi);
  std::reverse(lo, hi);
}

int gcd(int a, int b) {
  return (b == 0) ? a : gcd(b, a % b);
}

template<class It>
void rotate3(It lo, It mid, It hi) {
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
  int a[] = {2, 4, 2, 0, 5, 10, 7, 3, 7, 1};
  vector<int> v(a, a + 10);
  cout << "before sort:  ";
  for (int i = 0; i < (int)v.size(); i++) {
    cout << v[i] << " ";
  }
  cout << endl;

  // Insertion sort.
  for (vector<int>::iterator i = v.begin(); i != v.end(); ++i) {
    rotate1(std::upper_bound(v.begin(), i, *i), i, i + 1);
  }
  cout << "after sort:   ";
  for (int i = 0; i < (int)v.size(); i++) {
    cout << v[i] << " ";
  }
  cout << endl;

  // Simple rotation to the left.
  rotate2(v.begin(), v.begin() + 1, v.end());
  cout << "rotate left:  ";
  for (int i = 0; i < (int)v.size(); i++) {
    cout << v[i] << " ";
  }
  cout << endl;

  // Simple rotation to the right.
  rotate3(v.rbegin(), v.rbegin() + 1, v.rend());
  cout << "rotate right: ";
  for (int i = 0; i < (int)v.size(); i++) {
    cout << v[i] << " ";
  }
  cout << endl;

  return 0;
}
