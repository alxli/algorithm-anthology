/*

Bitwise convolution combines two arrays indexed by masks, grouping pairs of masks by a bitwise
operation instead of by ordinary addition. For arrays `a` and `b` over an `n`-bit universe, the XOR
convolution `c[m]` is the sum of `a[x]*b[y]` over all `x ^ y = m`. OR and AND convolution replace
`x ^ y` with `x | y` or `x & y`. These appear in subset DP, counting pairs by bitwise result, and
algebra over the Boolean hypercube.

The fast Walsh-Hadamard transform (FWHT) diagonalizes XOR convolution in the same way that the FFT
diagonalizes polynomial convolution: transform both arrays, multiply pointwise, and transform back.
The OR and AND versions are zeta/Mobius transforms over the subset lattice, but they have the same
butterfly-shaped implementation and the same O(n log n) runtime for arrays of length $n = 2^k$.

- `xor_transform(f, invert)` applies the XOR FWHT to `f` in place. The inverse transform uses
  `invert = true` and divides every coefficient by `f.size()`.
- `or_transform(f, invert)` applies the OR zeta transform in place; the inverse is Mobius inversion.
- `and_transform(f, invert)` applies the AND zeta transform in place; the inverse is Mobius
  inversion.
- `xor_convolve(a, b)` returns the XOR convolution of `a` and `b`.
- `or_convolve(a, b)` returns the OR convolution of `a` and `b`.
- `and_convolve(a, b)` returns the AND convolution of `a` and `b`.

Input arrays are padded with zeros to the next power of two. For exact integer XOR convolution, the
inverse divisions are exact; for modular arithmetic, replace the division by multiplication with the
modular inverse of the transform length.

Time Complexity:
- O(n log n) per transform or convolution, where $n$ is the padded power-of-two length.

Space Complexity:
- O(1) auxiliary for the in-place transforms.
- O(n) auxiliary heap space for each convolution.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

int next_power_of_two(int n) {
  int res = 1;
  while (res < n) {
    res <<= 1;
  }
  return res;
}

template<class T>
void xor_transform(std::vector<T> &f, bool invert) {
  int n = static_cast<int>(f.size());
  assert((n & (n - 1)) == 0);
  for (int len = 1; len < n; len <<= 1) {
    for (int i = 0; i < n; i += len << 1) {
      for (int j = 0; j < len; j++) {
        T u = f[i + j], v = f[i + j + len];
        f[i + j] = u + v;
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

template<class T>
void or_transform(std::vector<T> &f, bool invert) {
  int n = static_cast<int>(f.size());
  assert((n & (n - 1)) == 0);
  for (int bit = 1; bit < n; bit <<= 1) {
    for (int mask = 0; mask < n; mask++) {
      if (mask & bit) {
        if (invert) {
          f[mask] -= f[mask ^ bit];
        } else {
          f[mask] += f[mask ^ bit];
        }
      }
    }
  }
}

template<class T>
void and_transform(std::vector<T> &f, bool invert) {
  int n = static_cast<int>(f.size());
  assert((n & (n - 1)) == 0);
  for (int bit = 1; bit < n; bit <<= 1) {
    for (int mask = 0; mask < n; mask++) {
      if ((mask & bit) == 0) {
        if (invert) {
          f[mask] -= f[mask ^ bit];
        } else {
          f[mask] += f[mask ^ bit];
        }
      }
    }
  }
}

template<class T, class Transform>
std::vector<T> bitwise_convolve(std::vector<T> a, std::vector<T> b, Transform transform) {
  if (a.empty() || b.empty()) {
    return {};
  }
  int n = next_power_of_two(static_cast<int>(std::max(a.size(), b.size())));
  a.resize(n);
  b.resize(n);
  transform(a, false);
  transform(b, false);
  for (int i = 0; i < n; i++) {
    a[i] *= b[i];
  }
  transform(a, true);
  return a;
}

template<class T>
std::vector<T> xor_convolve(std::vector<T> a, std::vector<T> b) {
  return bitwise_convolve(a, b, xor_transform<T>);
}

template<class T>
std::vector<T> or_convolve(std::vector<T> a, std::vector<T> b) {
  return bitwise_convolve(a, b, or_transform<T>);
}

template<class T>
std::vector<T> and_convolve(std::vector<T> a, std::vector<T> b) {
  return bitwise_convolve(a, b, and_transform<T>);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int64_t> a{1, 2, 3, 4};
  vector<int64_t> b{5, 6, 7, 8};

  // XOR: c[m] sums a[x] * b[y] over pairs with x ^ y == m.
  assert((xor_convolve(a, b) == vector<int64_t>{70, 68, 62, 60}));

  // OR: c[m] sums a[x] * b[y] over pairs with x | y == m.
  assert((or_convolve(a, b) == vector<int64_t>{5, 28, 43, 184}));

  // AND: c[m] sums a[x] * b[y] over pairs with x & y == m.
  assert((and_convolve(a, b) == vector<int64_t>{103, 52, 73, 32}));

  vector<int64_t> one_mask{0, 1, 0, 0};
  vector<int64_t> two_mask{0, 0, 1};
  assert((xor_convolve(one_mask, two_mask) == vector<int64_t>{0, 0, 0, 1}));
  assert((or_convolve(one_mask, two_mask) == vector<int64_t>{0, 0, 0, 1}));
  assert((and_convolve(one_mask, two_mask) == vector<int64_t>{1, 0, 0, 0}));
  return 0;
}
