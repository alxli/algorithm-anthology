/*

3.3.7 - Square Root Decomposition

Description: To solve the dynamic range query problem using
square root decomposition, we split an array of size N into
sqrt(N) buckets, each bucket of size sqrt(N). As a result,
each query and update operation will be sqrt(N) in running time.

Time Complexity: O(N*sqrt(N)) to construct the initial
decomposition. After, query() and update() are O(sqrt N)/call.

Space Complexity: O(N) for the array. O(sqrt N) for the buckets.

Note: This implementation is 0-based, meaning that all
indices from 0 to N - 1, inclusive, are accessible.

=~=~=~=~= Sample Input =~=~=~=~=
5 10
35232
390942
649675
224475
18709
Q 1 3
M 4 475689
Q 2 3
Q 1 3
Q 1 2
Q 3 3
Q 2 3
M 2 645514
M 2 680746
Q 0 4

=~=~=~=~= Sample Output =~=~=~=~=
224475
224475
224475
390942
224475
224475
35232

*/

#include <cmath> /* sqrt() */
#include <limits> /* std::numeric_limits<T>::max() */
#include <vector>

template<class T> class sqrt_decomp {
  //define the following yourself. merge(x, nullv) must return x for all valid x
  static inline const T nullv() { return std::numeric_limits<T>::max(); }
  static inline const T merge(const T &a, const T &b) { return a < b ? a : b; }

  int len, blocklen, blocks;
  std::vector<T> array, block;

 public:
  sqrt_decomp(int n, T * a = 0): len(n), array(n) {
    blocklen = (int)sqrt(n);
    blocks = (n + blocklen - 1) / blocklen;
    block.resize(blocks);
    for (int i = 0; i < n; i++)
      array[i] = a ? a[i] : nullv();
    for (int i = 0; i < blocks; i++) {
      int h = (i + 1)*blocklen;
      if (h > n) h = n;
      block[i] = nullv();
      for (int j = i * blocklen; j < h; j++)
        block[i] = merge(block[i], array[j]);
    }
  }

  void update(int idx, const T & val) {
    array[idx] = val;
    int b = idx / blocklen;
    int h = (b + 1)*blocklen;
    if (h > len) h = len;
    block[b] = nullv();
    for (int i = b * blocklen; i < h; i++)
      block[b] = merge(block[b], array[i]);
  }

  T query(int lo, int hi) {
    T ret = nullv();
    int lb = ceil((double)lo / blocklen);
    int hb = (hi + 1) / blocklen - 1;
    if (lb > hb) {
      for (int i = lo; i <= hi; i++)
        ret = merge(ret, array[i]);
    } else {
      int l = lb*blocklen - 1;
      int h = (hb + 1)*blocklen;
      for (int i = lo; i <= l; i++)
        ret = merge(ret, array[i]);
      for (int i = lb; i <= hb; i++)
        ret = merge(ret, block[i]);
      for (int i = h; i <= hi; i++)
        ret = merge(ret, array[i]);
    }
    return ret;
  }

  inline int size() { return len; }
  inline int at(int idx) { return array[idx]; }
};

/*** Example Usage (wcipeg.com/problem/segtree) ***/

#include <cstdio>

int N, M, A, B, init[100005];

int main() {
  scanf("%d%d", &N, &M);
  for (int i = 0; i < N; i++) scanf("%d", &init[i]);
  sqrt_decomp<int> a(N, init);
  char op;
  for (int i = 0; i < M; i++) {
    scanf(" %c%d%d", &op, &A, &B);
    if (op == 'Q') {
      printf("%d\n", a.query(A, B));
    } else if (op == 'M') {
      a.update(A, B);
    }
  }
  return 0;
}
