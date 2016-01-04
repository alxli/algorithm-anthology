/*

Description: A Fenwick tree (a.k.a. binary indexed tree) is a
data structure that allows for the sum of an arbitrary range
of values in an array to be dynamically queried in logarithmic
time. Range updating in a Fenwick tree can only increment
values in a range, not set them all to the same value. This
version is a very concise version if only point queries are
needed. The functions below work on 1-based indices (between
1 and MAXN, inclusive).

Time Complexity: add() and at() are O(log MAXN).
Space Complexity: O(N).

*/

const int MAXN = 1000;
int bit[MAXN + 1];

//a[i] += v
void add(int i, int v) {
  for (i++; i <= MAXN; i += i & -i) bit[i] += v;
}

//a[i] += v for i = lo..hi, inclusive
void add(int lo, int hi, int v) {
  add(lo, v);
  add(hi + 1, -v);
}

//returns a[i]
int at(int i) {
  int sum = 0;
  for (i++; i > 0; i -= i & -i) sum += bit[i];
  return sum;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  add(1, 2, 5);
  add(2, 3, 5);
  add(3, 5, 10);
  cout << "BIT values: "; //5 10 15 10 10
  for (int i = 1; i <= 5; i++)
    cout << at(i) << " ";
  cout << "\n";
  return 0;
}
