/*

3.2.4 - Binary Indexed Tree (OOP - Range Update, Range Query)

Description: Using two arrays, a BIT can be made to support
range updates and range queries simultaneously. However,
the range updates can only be used to add an increment to all
values in a range, not set them to the same value. The latter
will require a segment tree with lazy propagation.

Time Complexity: query(), update(), at(), and inc_range() are O(log(N))

Space Complexity: O(N) storage and O(N) auxiliary on the number
of elements in the array.

*/

template<class T> class binary_indexed_tree {
  int SIZE;
  T *B1, *B2;

  T Pquery(T* B, int i) {
    T sum = 0;
    for (; i != 0; i -= i & -i) sum += B[i];
    return sum;
  }

  T Rquery(int i) {
    return Pquery(B1, i)*i - Pquery(B2, i);
  }

  T Rquery(int L, int H) {
    return Rquery(H) - Rquery(L-1);
  }

  void Pupdate(T* B, int i, const T &inc) {
    for (; i <= SIZE; i += i & -i) B[i] += inc;
  }

  void Rupdate(int L, int H, T inc) {
    Pupdate(B1, L, inc);
    Pupdate(B1, H + 1, -inc);
    Pupdate(B2, L, inc*(L-1));
    Pupdate(B2, H + 1, -inc*H);
  }

 public:
  binary_indexed_tree(int N): SIZE(N+1) {
    B1 = new T[SIZE + 1];
    B2 = new T[SIZE + 1];
    for (int i = 0; i <= SIZE; i++)
      B1[i] = B2[i] = 0;
  }

  ~binary_indexed_tree() {
    delete[] B1;
    delete[] B2;
  }

  int size() { return SIZE - 1; }
  T at(int idx) { return Rquery(idx + 1, idx + 1); }
  T query(int L, int H) { return Rquery(L + 1, H + 1); }

  void update(int i, const T &newval) {
    Rupdate(i + 1, i + 1, newval - at(i + 1));
  } /* sets index value at index i to newval */

  void inc_range(int L, int H, const T &inc) {
    Rupdate(L + 1, H + 1, inc);
  } /* adds inc to every value in range [L,H] */
};

/*** Example Usage: ***/

#include <iostream>
using namespace std;

int main() {
  int a[] = {10, 1, 2, 3, 4};
  binary_indexed_tree<int> BIT(5);
  for (int i = 0; i < 5; i++) BIT.update(i, a[i]);
  BIT.inc_range(0, 2, 5);
  cout << BIT.query(0, 4) << endl; //35
  return 0;
}
