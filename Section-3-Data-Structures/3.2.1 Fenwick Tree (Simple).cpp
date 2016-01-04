/*

Description: A Fenwick tree (a.k.a. binary indexed tree) is a
data structure that allows for the sum of an arbitrary range
of values in an array to be dynamically queried in logarithmic
time. Note that unlike the object-oriented versions of this
data structure found in later sections, the operations here
work on 1-based indices (i.e. between 1 and MAXN, inclusive).
The array a[] is always synchronized with the bit[] array and
should not be modified outside of the functions below.

Time Complexity: All functions are O(log MAXN).
Space Complexity: O(MAXN) storage and auxiliary.

*/

const int MAXN = 1000;
int a[MAXN + 1], bit[MAXN + 1];

//a[i] += v
void add(int i, int v) {
  a[i] += v;
  for (; i <= MAXN; i += i & -i)
    bit[i] += v;
}

//a[i] = v
void set(int i, int v) {
  int inc = v - a[i];
  add(i, inc);
}

//returns sum(a[i] for i = 1..hi inclusive)
int sum(int hi) {
  int ret = 0;
  for (; hi > 0; hi -= hi & -hi)
    ret += bit[hi];
  return ret;
}

//returns sum(a[i] for i = lo..hi inclusive)
int sum(int lo, int hi) {
  return sum(hi) - sum(lo - 1);
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  for (int i = 1; i <= 5; i++) set(i, i);
  add(4, -5);
  cout << "BIT values: ";
  for (int i = 1; i <= 5; i++)
    cout << a[i] << " "; //1 2 3 -1 5
  cout << "\nSum of range [1,3] is ";
  cout << sum(1, 3) << ".\n"; //6
  return 0;
}
