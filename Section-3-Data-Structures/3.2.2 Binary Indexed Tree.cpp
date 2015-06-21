/*

3.2.2 - Binary Indexed Tree Class (Point Update, Range Query)

Description: A binary indexed tree (a.k.a. Fenwick Tree or
BIT) is a data structure that allows for the sum of an
arbitrary range of values in an array to be dynamically
queried in logarithmic time. All methods below operate on
the indices of a 0-based array.

Time Complexity: add(), set(), and sum() are all O(log N).
size() and at() are O(1).

Space Complexity: O(N) storage and O(N) auxiliary on size().

=~=~=~=~= Sample Output =~=~=~=~=
BIT values: 5 1 2 3 4
Sum of range [1, 3] is 6.

*/

#include <vector>

template<class T> class binary_indexed_tree {
  int len;
  std::vector<int> a, bit;

 public:
  binary_indexed_tree(int n):
    len(n), a(n + 1), bit(n + 1) {}

  void add(int i, const T & v) {
    a[++i] += v;
    for (; i <= len; i += i & -i)
      bit[i] += v;
  }

  void set(int i, const T & v) {
    T inc = v - a[i + 1];
    add(i, inc);
  }

  T sum(int hi) {
    T res = 0;
    for (hi++; hi > 0; hi -= hi & -hi)
      res += bit[hi];
    return res;
  }

  T sum(int lo, int hi) {
    return sum(hi) - sum(lo - 1);
  }

  inline int size() { return len; }
  inline T at(int i) { return a[i + 1]; }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int a[] = {10, 1, 2, 3, 4};
  binary_indexed_tree<int> t(5);
  for (int i = 0; i < 5; i++) t.set(i, a[i]);
  t.add(0, -5);
  cout << "BIT values: ";
  for (int i = 0; i < t.size(); i++)
    cout << t.at(i) << " ";
  cout << "\nSum of range [1, 3] is ";
  cout << t.sum(1, 3) << ".\n";
  return 0;
}
