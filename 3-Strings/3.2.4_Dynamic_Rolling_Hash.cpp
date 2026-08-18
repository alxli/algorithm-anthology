/*

Maintains a polynomial rolling hash for a sequence of nonnegative integers, supporting point updates
and O(log n) contiguous subsequence hash queries. Unlike the static rolling hash, which fixes its
prefix hashes at construction, this structure allows any element to change between queries, which is
useful for problems that compare substrings while characters are edited.

The key idea is to weight each element by a power of the base tied to its absolute position: storing
$a_i B^i$ at index `i` turns the hash of any prefix into an ordinary prefix sum, which a Fenwick
tree maintains under point updates. The raw sum over a half-open range $[l, h)$ is $\sum_i a_i B^i$,
and multiplying by $B^{-l}$ shifts it to the position-independent value $\sum_i a_i B^{i - l}$, so
two ranges with equal contents receive equal hashes (the leftmost element carries the lowest power).
A segment tree could replace the Fenwick tree if lazy range updates are needed.

`MOD1` and `MOD2` are two distinct primes, each kept below $2^{31}$ so that the product of any two
residues fits in a 64-bit integer; hashing modulo both and comparing the resulting pair makes a
collision astronomically unlikely. `BASE1` and `BASE2` are the corresponding polynomial bases, which
must be nonzero modulo their corresponding primes and may be randomized per run to resist
adversarial inputs. For a small raw alphabet, choosing bases larger than every value also prevents
carry-style collisions before modular reduction, but general integer values need not satisfy this
bound.

Element values must be nonnegative. As with any polynomial hash, comparing subsequences of different
lengths is only safe when values are positive (for example, map an alphabet to $[1, B)$) so that
trailing zeros cannot make a shorter sequence collide with a longer one.

- `DynamicRollingHash(a)` builds prefix hashes over the integer sequence `a`.
- `size()` returns the length of the sequence.
- `at(i)` returns the value at index `i`.
- `set(i, x)` assigns the value at index `i` to `x`.
- `hash(lo, hi)` returns the hash pair of the half-open subsequence $[`lo`, `hi`)$. Equal ranges
  always have equal hash pairs, while equal hash pairs indicate equal ranges with high probability.

Time Complexity:
- O(n log n) per call to the constructor, where $n$ is the length of the sequence.
- O(1) per call to `size()` and `at()`.
- O(log n) per call to `set()` and `hash()`.

Space Complexity:
- O(n) for storage.
- O(1) auxiliary per query.

*/

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

class DynamicRollingHash {
  static const uint64_t MOD1 = 1000000007, MOD2 = 1000000009;  // Distinct primes below 2^31.
  static const uint64_t BASE1 = 131, BASE2 = 137;              // Nonzero modulo MOD1 and MOD2.

  int len;
  std::vector<int> cur;
  std::vector<uint64_t> pw1, pw2, ip1, ip2;  // Base powers and inverse base powers.
  std::vector<uint64_t> ft1, ft2;            // Fenwick trees, 1-indexed of size len + 1.

  // Products and sums of residues below 2^30 stay under 2^60, so 64-bit arithmetic never overflows.
  static uint64_t powmod(uint64_t x, uint64_t n, uint64_t m) {
    uint64_t res = 1 % m;
    for (x %= m; n > 0; n >>= 1) {
      if (n & 1) {
        res = res * x % m;
      }
      x = x * x % m;
    }
    return res;
  }

  static void add(std::vector<uint64_t> &ft, int i, uint64_t delta, uint64_t m) {
    for (i++; i < static_cast<int>(ft.size()); i += i & -i) {
      ft[i] = (ft[i] + delta) % m;
    }
  }

  static uint64_t sum(const std::vector<uint64_t> &ft, int i, uint64_t m) {
    uint64_t res = 0;
    for (i++; i > 0; i -= i & -i) {
      res = (res + ft[i]) % m;
    }
    return res;
  }

 public:
  explicit DynamicRollingHash(const std::vector<int> &a)
      : len(static_cast<int>(a.size())),
        cur(len, 0),
        pw1(len + 1),
        pw2(len + 1),
        ip1(len + 1),
        ip2(len + 1),
        ft1(len + 1),
        ft2(len + 1) {
    pw1[0] = pw2[0] = ip1[0] = ip2[0] = 1;
    uint64_t invb1 = powmod(BASE1, MOD1 - 2, MOD1), invb2 = powmod(BASE2, MOD2 - 2, MOD2);
    for (int i = 1; i <= len; i++) {
      pw1[i] = pw1[i - 1] * BASE1 % MOD1;
      pw2[i] = pw2[i - 1] * BASE2 % MOD2;
      ip1[i] = ip1[i - 1] * invb1 % MOD1;
      ip2[i] = ip2[i - 1] * invb2 % MOD2;
    }
    for (int i = 0; i < len; i++) {
      set(i, a[i]);
    }
  }

  int size() const { return len; }

  int at(int i) const {
    assert(0 <= i && i < len);
    return cur[i];
  }

  void set(int i, int x) {
    assert(0 <= i && i < len && x >= 0);
    uint64_t old1 = static_cast<uint64_t>(cur[i]) % MOD1 * pw1[i] % MOD1;
    uint64_t old2 = static_cast<uint64_t>(cur[i]) % MOD2 * pw2[i] % MOD2;
    uint64_t new1 = static_cast<uint64_t>(x) % MOD1 * pw1[i] % MOD1;
    uint64_t new2 = static_cast<uint64_t>(x) % MOD2 * pw2[i] % MOD2;
    add(ft1, i, (new1 + MOD1 - old1) % MOD1, MOD1);
    add(ft2, i, (new2 + MOD2 - old2) % MOD2, MOD2);
    cur[i] = x;
  }

  std::pair<uint64_t, uint64_t> hash(int lo, int hi) const {
    assert(0 <= lo && lo <= hi && hi <= len);
    uint64_t raw1 = (sum(ft1, hi - 1, MOD1) + MOD1 - sum(ft1, lo - 1, MOD1)) % MOD1;
    uint64_t raw2 = (sum(ft2, hi - 1, MOD2) + MOD2 - sum(ft2, lo - 1, MOD2)) % MOD2;
    return {raw1 * ip1[lo] % MOD1, raw2 * ip2[lo] % MOD2};
  }
};

/*** Example Usage ***/

#include <string>
using namespace std;

int main() {
  string s = "abcabc";
  vector<int> a(s.begin(), s.end());
  DynamicRollingHash h(a);
  assert(h.size() == 6);
  assert(h.hash(0, 3) == h.hash(3, 6));  // "abc" == "abc".

  h.set(0, 'x');  // "xbcabc".
  assert(!(h.hash(0, 3) == h.hash(3, 6)));
  h.set(3, 'x');                         // "xbcxbc".
  assert(h.hash(0, 3) == h.hash(3, 6));  // "xbc" == "xbc".

  // A single element's hash is invariant under where an equal value sits.
  assert(h.hash(1, 2) == h.hash(4, 5));  // both 'b'.
  assert(h.at(0) == 'x' && h.at(1) == 'b');
  return 0;
}
