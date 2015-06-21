/*

3.2.4 - Binary Indexed Tree (Range Update/Query)

Description: Using two arrays, a BIT can be made to
support range updates and range queries simultaneously.
However, the range updates can only be used to add an
increment to all values in a range, not set them to the
same value. The latter problem may be solved using a
segment tree with lazy propagation.

Time Complexity: add(), set(), at(), and sum() are all
                 O(log N). size() is O(1).

Space Complexity: O(N) storage and auxiliary.

=~=~=~=~= Sample Output =~=~=~=~=
BIT values: 15 6 7 -5 4
Sum of range [0, 4] is 27.

*/

#include <vector>

template<class T> class binary_indexed_tree {
  int len;
  std::vector<T> b1, b2;

  T sum(const std::vector<T> & b, int i) {
    T res = 0;
    for (; i != 0; i -= i & -i) res += b[i];
    return res;
  }

  void add(std::vector<T> & b, int i, const T & v) {
    for (; i <= len; i += i & -i) b[i] += v;
  }

 public:
  binary_indexed_tree(int n):
    len(n + 1), b1(n + 2), b2(n + 2) {}

  //a[i] += v for i = lo..hi, inclusive
  void add(int lo, int hi, const T & v) {
    lo++, hi++;
    add(b1, lo, v);
    add(b1, hi + 1, -v);
    add(b2, lo, v * (lo - 1));
    add(b2, hi + 1, -v * hi);
  }

  //a[i] = v
  void set(int i, const T & v) { add(i, i, v - at(i)); }

  //returns sum(a[i] for i = 1..hi inclusive)
  T sum(int hi) { return sum(b1, hi)*hi - sum(b2, hi); }

  //returns sum(a[i] for i = lo..hi inclusive)
  T sum(int lo, int hi) { return sum(hi + 1) - sum(lo); }

  inline int size() const { return len - 1; }
  inline T at(int i) { return sum(i, i); }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int a[] = {10, 1, 2, 3, 4};
  binary_indexed_tree<int> t(5);
  for (int i = 0; i < 5; i++) t.set(i, a[i]);
  t.add(0, 2, 5); //15 6 7 3 4
  t.set(3, -5);   //15 6 7 -5 4
  cout << "BIT values: ";
  for (int i = 0; i < t.size(); i++)
    cout << t.at(i) << " ";
  cout << "\nSum of range [0, 4] is ";
  cout << t.sum(0, 4) << ".\n"; //27
  return 0;
}
