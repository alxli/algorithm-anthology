/*

3.2.2 - Binary Indexed Tree (Point Update, Range Query)

Description: A binary indexed tree (a.k.a. Fenwick Tree or BIT)
is a data structure that allows for the sum of an arbitrary
range of values in an array to be dynamically queried in
logarithmic time.

Time Complexity: query() and update() are O(log(N)). All other
functions are O(1).

Space Complexity: O(N) storage and O(N) auxiliary on the number
of elements in the array.

=~=~=~=~= Sample Output =~=~=~=~=
BIT values: 10 1 2 3 4
Sum of range [1, 3] is 6.

*/

template<class T> class binary_indexed_tree {
  int SIZE;
  T *data, *bits;
  
  T internal_query(int hi) {
    T sum = 0;
    for (; hi > 0; hi -= hi & -hi)
      sum += bits[hi];
    return sum;
  }
  
 public:
  binary_indexed_tree(int N): SIZE(N+1) {
    data = new T[SIZE];
    bits = new T[SIZE];
    for (int i = 0; i < SIZE; i++)
      data[i] = bits[i] = 0;
  }
  
  ~binary_indexed_tree() {
    delete[] data;
    delete[] bits;
  }
  
  void update(int i, const T & newval) {
    T inc = newval - data[++i];
    data[i] = newval;
    for (; i < SIZE; i += i & -i)
      bits[i] += inc;
  }
  
  int size()  { return SIZE - 1; }
  T at(int i) { return data[i + 1]; }
  T query(int lo = 0, int hi = 0) {
    return internal_query(hi + 1) - internal_query(lo);
  }
};

/*** Example Usage: ***/

#include <iostream>
using namespace std;

int main() {
  int a[] = {10, 1, 2, 3, 4};
  binary_indexed_tree<int> BIT(5);
  for (int i = 0; i < 5; i++) BIT.update(i, a[i]);
  cout << "BIT values: ";
  for (int i = 0; i < BIT.size(); i++)
    cout << BIT.at(i) << " ";
  cout << "\nSum of range [1, 3] is ";
  cout << BIT.query(1, 3) << ".\n";
  return 0;
}
