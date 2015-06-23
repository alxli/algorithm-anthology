/*

1.1.2 - Array Rotation

All of the functions below are equal in behavior
to std::rotate(). Even though the function is
already implemented, this is just a demo of how
it could be done in other languages.

*/

#include <algorithm> /* reverse(), rotate(), swap() */

template<class ForwardIterator>
void rotate1(ForwardIterator lo, ForwardIterator mid,
             ForwardIterator hi) {
  ForwardIterator next = mid;
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
    int cur = i, next;
    for (int j = 0; j < cycle - 1; j++) {
      next = cur + jump;
      if (next >= n) next -= n;
      std::swap(*(lo + cur), *(lo + next));
      cur = next;
    }
  }
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

int main () {
  int a[] = {-10, 11, 5, 7, 9, 4, 8, 2};
  rotate3(a, a + 5, a + 8); //4, 8, 2, -10, 11, 5, 7, 9
  for (int i = 0; i < 8; i++)
    cout << a[i] << " ";
  cout << "\n";

  std::vector<int> v, v1, v2, v3;
  for (int i = 0; i < 10000; i++) {
    v.push_back(i);
    v1.push_back(i);
    v2.push_back(i);
    v3.push_back(i);
  }
  int mid = 5678;
  std::rotate(v.begin(), v.begin() + mid, v.end());
  rotate1(v1.begin(), v1.begin() + mid, v1.end());
  rotate2(v2.begin(), v2.begin() + mid, v2.end());
  rotate3(v3.begin(), v3.begin() + mid, v3.end());

  assert(v == v1 && v == v2 && v == v3);

  return 0;
}
