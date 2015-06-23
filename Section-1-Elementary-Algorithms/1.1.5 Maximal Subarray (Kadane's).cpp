/*

1.1.5 - Maximal Subarray Sum (Kadane's Algorithm)

Given an array of (potentially negative) integers,
finds the maximal sum of any contiguous subarray.

Time Complexity: O(n) on the length of the array
Space Complexty: O(1) auxiliary.

*/

template<class It> int max_subarray_sum(It lo, It hi) {
  int max_ending_here = 0, max_so_far = 0, max_overall;
  for (max_overall = *lo; lo != hi; ++lo) {
    max_ending_here += *lo;
    if (max_overall < *lo) max_overall = *lo;
    if (max_ending_here < 0) {
      max_ending_here = 0;
    } else if (max_so_far < max_ending_here) {
      max_so_far = max_ending_here;
    }
  }
  if (max_overall < 0) return max_overall; //all negative
  return max_so_far;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int a[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
  //answer is 4 + (-1) + 2 + 1 = 6;
  cout << max_subarray_sum(a, a + 9) << "\n";
  return 0;
}
