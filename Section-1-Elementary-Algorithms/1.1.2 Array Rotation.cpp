/*

The following functions are equivalent to std::rotate(), taking three
iterators lo, mid, hi, and swapping the elements in the range [lo, hi)
in such a way that the element at mid becomes the first element of the
new range and the element at mid - 1 becomes the last element.

All three versions achieve the same result using no temporary arrays.
Version 1 uses a straightforward swapping algorithm listed on many C++
reference sites, requiring only forward iterators. Version 2 requires
bidirectional iterators, employing the well-known technique of three
simple reversals. Version 3 applies a "juggling" algorithm which first
divides the range into gcd(n, k) sets (n = hi - lo and k = mid - lo)
and then rotates the corresponding elements in each set. This version
requires random access iterators.

Time Complexity: O(n) on the distance between lo and hi.
Space Complexity: O(1) auxiliary.

*/

#include <algorithm> /* std::reverse(), std::rotate(), std::swap() */

template<class It> void rotate1(It lo, It mid, It hi) {
  It next = mid;
  while (lo != next) {
    std::swap(*lo++, *next++);
    if (next == hi)
      next = mid;
    else if (lo == mid)
      mid = next;
  }
}

template<class It> void rotate2(It lo, It mid, It hi) {
  std::reverse(lo, mid);
  std::reverse(mid, hi);
  std::reverse(lo, hi);
}

int gcd(int a, int b) {
  return b == 0 ? a : gcd(b, a % b);
}

template<class It> void rotate3(It lo, It mid, It hi) {
  int n = hi - lo, jump = mid - lo;
  int g = gcd(jump, n), cycle = n / g;
  for (int i = 0; i < g; i++) {
    int curr = i, next;
    for (int j = 0; j < cycle - 1; j++) {
      next = curr + jump;
      if (next >= n)
        next -= n;
      std::swap(*(lo + curr), *(lo + next));
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
  std::vector<int> v0, v1, v2, v3;
  for (int i = 0; i < 10000; i++)
    v0.push_back(i);
  v1 = v2 = v3 = v0;
  int mid = 5678;
  std::rotate(v0.begin(), v0.begin() + mid, v0.end());
  rotate1(v1.begin(), v1.begin() + mid, v1.end());
  rotate2(v2.begin(), v2.begin() + mid, v2.end());
  rotate3(v3.begin(), v3.begin() + mid, v3.end());
  assert(v0 == v1 && v0 == v2 && v0 == v3);

  //example from: http://en.cppreference.com/w/cpp/algorithm/rotate
  int a[] = {2, 4, 2, 0, 5, 10, 7, 3, 7, 1};
  vector<int> v(a, a + 10);
  cout << "before sort:  ";
  for (int i = 0; i < (int)v.size(); i++)
    cout << v[i] << ' ';
  cout << endl;

  //insertion sort
  for (vector<int>::iterator i = v.begin(); i != v.end(); ++i)
    rotate1(std::upper_bound(v.begin(), i, *i), i, i + 1);
  cout << "after sort:   ";
  for (int i = 0; i < (int)v.size(); i++)
    cout << v[i] << ' ';
  cout << endl;

  //simple rotation to the left
  rotate2(v.begin(), v.begin() + 1, v.end());
  cout << "rotate left:  ";
  for (int i = 0; i < (int)v.size(); i++)
    cout << v[i] << ' ';
  cout << endl;

  //simple rotation to the right
  rotate3(v.rbegin(), v.rbegin() + 1, v.rend());
  cout << "rotate right: ";
  for (int i = 0; i < (int)v.size(); i++)
    cout << v[i] << ' ';
  cout << endl;
  return 0;
}
