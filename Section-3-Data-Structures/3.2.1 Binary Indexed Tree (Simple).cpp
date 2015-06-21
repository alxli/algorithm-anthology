/*

3.2.1 - Binary Indexed Tree (Simple Version for Ints)

Description: A binary indexed tree (a.k.a. Fenwick Tree or
BIT) is a data structure that allows for the sum of an
arbitrary range of values in an array to be dynamically
queried in logarithmic time. Note that unlike the object-
oriented versions of this data structure found in this
anthology, the operations work for 1-based indices only
(i.e. between 1 and MAXN, inclusive). The array a[] is
always synchronized with the bit[] array and should not be
modified outside of the functions below.

Time Complexity: All functions are O(log MAXN).
Space Complexity: O(MAXN) storage and auxiliary.

=~=~=~=~= Sample Output =~=~=~=~=
BIT values: 1 2 3 -1 5
Sum of range [1,3] is 6.

*/

const int MAXN = 1000;
int a[MAXN+1], bit[MAXN+1];

//a[i] += v
int add(int i, int v) {
  a[i] += v;
  for (; i <= MAXN; i += i & -i)
    bit[i] += v;
}

//a[i] = v
int set(int i, int v) {
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
    cout << a[i] << " ";
  cout << "\nSum of range [1,3] is ";
  cout << sum(1, 3) << ".\n";
  return 0;
}
