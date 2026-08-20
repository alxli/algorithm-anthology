/*

Bitwise convolution combines two arrays indexed by masks, grouping pairs of masks by a bitwise
operation instead of by ordinary addition. For arrays `a` and `b` over an $n$-bit universe, the XOR
convolution `c[m]` is the sum of `a[x]*b[y]` over all `x ^ y = m`. OR and AND convolution replace
`x ^ y` with `x | y` or `x & y`. These appear in subset DP, counting pairs by bitwise result, and
algebra over the Boolean hypercube.

The fast Walsh-Hadamard transform (FWHT) diagonalizes XOR convolution in the same way that the FFT
diagonalizes polynomial convolution: transform both arrays, multiply pointwise, and transform back.
The OR and AND versions are Zeta/Mobius transforms over the subset lattice, but they have the same
butterfly-shaped implementation and the same O(n log n) runtime for arrays of length $n = 2^k$.

- `xor_transform(f, invert)` applies the XOR FWHT to `f` in place. The inverse transform uses
  `invert = true` and divides every coefficient by `f.size()`.
- `or_transform(f, invert)` applies the OR Zeta transform in place; the inverse is Mobius inversion.
- `and_transform(f, invert)` applies the AND Zeta transform in place; the inverse is Mobius
  inversion.
- `xor_convolve(a, b)` returns the XOR convolution of `a` and `b`.
- `or_convolve(a, b)` returns the OR convolution of `a` and `b`.
- `and_convolve(a, b)` returns the AND convolution of `a` and `b`.

Subset convolution restricts OR convolution to pairs of disjoint masks, so `c[m]` must count only
`x | y = m` with `x & y = 0`. Rank each entry by its number of set bits, transform every rank layer,
and multiply the layers as polynomials in the rank. Mobius inversion then recovers pairs whose union
is exactly `m`; among these pairs, the ranks sum to the number of set bits in `m` exactly when the
pair is disjoint.

- `subset_convolve(a, b)` returns the subset convolution of `a` and `b`, where `c[m]` sums
  `a[x]*b[y]` over the pairs of disjoint masks satisfying `x | y = m`.

The in-place transforms require a nonempty array whose length is a power of two. A convolution
returns empty if either input is empty; otherwise, both inputs are padded with zeros to the smallest
power of two at least their maximum length, which is also the output length. For exact integer XOR
convolution, the inverse divisions are exact; for modular arithmetic, replace the division by
multiplication with the modular inverse of the transform length.

Overflow warning: All intermediate sums, differences, and products must be representable in the
value type.

Time Complexity:
- O(n log n) per transform or bitwise convolution, where $n$ is the padded power-of-two length.
- O(n log^2 n) per call to `subset_convolve()`.

Space Complexity:
- O(1) auxiliary for the in-place transforms.
- O(n) auxiliary and O(n) for the returned array from each bitwise convolution.
- O(n log n) auxiliary for `subset_convolve()`, from the rank layers.

*/

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

template<typename T>
void xor_transform(std::vector<T> &f, bool invert) {
  int n = static_cast<int>(f.size());
  assert(n > 0 && (n & (n - 1)) == 0);
  for (int len = 1; len < n; len <<= 1) {
    for (int i = 0; i < n; i += len << 1) {
      for (int j = 0; j < len; j++) {
        T u = f[i + j], v = f[i + j + len];
        f[i + j] = u + v;  // Overflow warning.
        f[i + j + len] = u - v;
      }
    }
  }
  if (invert) {
    for (T &x : f) {
      x /= n;
    }
  }
}

template<typename T>
void or_transform(std::vector<T> &f, bool invert) {
  int n = static_cast<int>(f.size());
  assert(n > 0 && (n & (n - 1)) == 0);
  for (int bit = 1; bit < n; bit <<= 1) {
    for (int mask = 0; mask < n; mask++) {
      if (mask & bit) {
        if (invert) {
          f[mask] -= f[mask ^ bit];  // Overflow warning.
        } else {
          f[mask] += f[mask ^ bit];
        }
      }
    }
  }
}

template<typename T>
void and_transform(std::vector<T> &f, bool invert) {
  int n = static_cast<int>(f.size());
  assert(n > 0 && (n & (n - 1)) == 0);
  for (int bit = 1; bit < n; bit <<= 1) {
    for (int mask = 0; mask < n; mask++) {
      if ((mask & bit) == 0) {
        if (invert) {
          f[mask] -= f[mask ^ bit];  // Overflow warning.
        } else {
          f[mask] += f[mask ^ bit];
        }
      }
    }
  }
}

template<typename T, typename Transform>
std::vector<T> bitwise_convolve(std::vector<T> a, std::vector<T> b, Transform transform) {
  if (a.empty() || b.empty()) {
    return {};
  }
  int needed = static_cast<int>(std::max(a.size(), b.size()));
  int n = 1;
  while (n < needed) {
    n <<= 1;
  }
  a.resize(n);
  b.resize(n);
  transform(a, false);
  transform(b, false);
  for (int i = 0; i < n; i++) {
    a[i] *= b[i];  // Overflow warning.
  }
  transform(a, true);
  return a;
}

template<typename T>
std::vector<T> xor_convolve(std::vector<T> a, std::vector<T> b) {
  return bitwise_convolve(std::move(a), std::move(b), xor_transform<T>);
}

template<typename T>
std::vector<T> or_convolve(std::vector<T> a, std::vector<T> b) {
  return bitwise_convolve(std::move(a), std::move(b), or_transform<T>);
}

template<typename T>
std::vector<T> and_convolve(std::vector<T> a, std::vector<T> b) {
  return bitwise_convolve(std::move(a), std::move(b), and_transform<T>);
}

template<typename T>
std::vector<T> subset_convolve(std::vector<T> a, std::vector<T> b) {
  if (a.empty() || b.empty()) {
    return {};
  }
  int needed = static_cast<int>(std::max(a.size(), b.size()));
  int n = 1, bits = 0;
  while (n < needed) {
    n <<= 1;
    bits++;
  }
  a.resize(n);
  b.resize(n);
  std::vector<int> ranks(n);
  for (int mask = 1; mask < n; mask++) {
    ranks[mask] = ranks[mask >> 1] + (mask & 1);
  }
  // Split each input into layers by rank, so that layer k holds only the masks with k set bits.
  std::vector<std::vector<T>> fa(bits + 1, std::vector<T>(n)), fb = fa, fc = fa;
  for (int mask = 0; mask < n; mask++) {
    fa[ranks[mask]][mask] = a[mask];
    fb[ranks[mask]][mask] = b[mask];
  }
  for (int k = 0; k <= bits; k++) {
    or_transform(fa[k], false);
    or_transform(fb[k], false);
  }
  // Multiplying layers groups pairs by total rank. Once Mobius inversion recovers each pair's exact
  // union, total rank ranks[mask] selects precisely the disjoint pairs covering mask.
  for (int i = 0; i <= bits; i++) {
    for (int j = 0; i + j <= bits; j++) {
      for (int mask = 0; mask < n; mask++) {
        fc[i + j][mask] += fa[i][mask] * fb[j][mask];  // Overflow warning.
      }
    }
  }
  std::vector<T> res(n);
  for (int k = 0; k <= bits; k++) {
    or_transform(fc[k], true);
  }
  for (int mask = 0; mask < n; mask++) {
    res[mask] = fc[ranks[mask]][mask];
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <cstdint>
using namespace std;

int main() {
  {
    vector<int64_t> f{1, 2, 3, 4}, original(f);
    xor_transform(f, false);
    xor_transform(f, true);
    assert(f == original);
  }
  {
    vector<int64_t> f{1, 2, 3, 4}, original(f);
    or_transform(f, false);
    or_transform(f, true);
    assert(f == original);
  }
  {
    vector<int64_t> f{1, 2, 3, 4}, original(f);
    and_transform(f, false);
    and_transform(f, true);
    assert(f == original);
  }

  vector<int64_t> a{1, 2, 3, 4}, b{5, 6, 7, 8};

  // XOR: c[m] sums a[x] * b[y] over pairs with x ^ y == m.
  assert((xor_convolve(a, b) == vector<int64_t>{70, 68, 62, 60}));

  // OR: c[m] sums a[x] * b[y] over pairs with x | y == m.
  assert((or_convolve(a, b) == vector<int64_t>{5, 28, 43, 184}));

  // AND: c[m] sums a[x] * b[y] over pairs with x & y == m.
  assert((and_convolve(a, b) == vector<int64_t>{103, 52, 73, 32}));

  // Subset: c[m] sums a[x] * b[y] over disjoint pairs with x | y == m. Every entry is at most the
  // corresponding OR convolution, which also counts the overlapping pairs.
  assert((subset_convolve(a, b) == vector<int64_t>{5, 16, 22, 60}));

  vector<int64_t> one_mask{0, 1, 0, 0};
  vector<int64_t> two_mask{0, 0, 1};
  assert((xor_convolve(one_mask, two_mask) == vector<int64_t>{0, 0, 0, 1}));
  assert((or_convolve(one_mask, two_mask) == vector<int64_t>{0, 0, 0, 1}));
  assert((and_convolve(one_mask, two_mask) == vector<int64_t>{1, 0, 0, 0}));
  assert((subset_convolve(one_mask, two_mask) == vector<int64_t>{0, 0, 0, 1}));

  // Overlapping masks contribute nothing at all to a subset convolution.
  assert((subset_convolve(one_mask, one_mask) == vector<int64_t>{0, 0, 0, 0}));
  return 0;
}
