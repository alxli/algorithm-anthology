/*

Computes polynomial rolling hashes for sequences of an arbitrary hashable type `T`, supporting O(1)
contiguous subsequence hash queries after preprocessing. This is useful for probabilistic string
matching, substring equality checks, repeated subarray detection, and binary-searching over
candidate lengths.

Given a sequence $a_0, a_1, \ldots, a_{n-1}$ and a value hash $h(a_i)$ for each element, the hash is
the polynomial $H(a) = \sum_{i=0}^{n-1} h(a_i) B^{n-1-i} \pmod M$, where $B$ is `HASH_BASE` and $M$
is `HASH_MOD`. Let $p(i)$ be the hash of the first $i$ values. It is built left-to-right by the
prefix hash recurrence $p(0) = 0$ and $p(i + 1) = p(i) B + h(a_i) \pmod M$. The hash of a half-open
subsequence $[l, r)$ is then computed as $p(r) - p(l) B^{r-l} \pmod M$.

The implementation works modulo the Mersenne prime $2^{61} - 1$, using `__uint128_t` for the
multiplication where available and falling back to a double-and-add modular multiply otherwise. The
base `HASH_BASE` should be changed or chosen randomly for open-hacking environments. With a fixed
base, hashing remains fast and practical, but it is still probabilistic and should not be used as
proof of equality when exact verification is required.

By default, each sequence value is cast to `uint64_t` and mixed. For non-integer element types,
instantiate `RollingHash<T, Hash>` and pass a custom value hasher as the final constructor or
`hash()` argument. It must map each element to a stable nonzero value in $[1, `HASH_MOD`)$.

- `RollingHash<T>()` constructs an empty hash sequence.
- `RollingHash<T>(lo, hi)` constructs prefix hashes from the values in the half-open iterator range
  $[`lo`, `hi`)$.
- `RollingHash<T>(v)` constructs prefix hashes for vector `v`.
- `h.hash(lo, hi)` returns the hash of the half-open subsequence $[`lo`, `hi`)$.
- `RollingHash<T>::hash(lo, hi)` returns the hash of the half-open iterator range $[`lo`, `hi`)$.
- `RollingHash<T>::hash(v)` returns the hash of vector `v`.
- `concat(left, right, right_len)` returns the hash of the concatenation of a sequence with hash
  `left` and a sequence with hash `right` and length `right_len`.

Time Complexity:
- O(n) per call to the constructor or static `hash()`, where $n$ is the sequence length.
- O(1) per call to the member `hash()` and `concat()`.

Space Complexity:
- O(n) for storage of prefix hashes and powers, where $n$ is the maximum sequence length processed
  so far.
- O(1) auxiliary per query.

*/

#include <cassert>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

const uint64_t HASH_MOD = (1ULL << 61) - 1;
const uint64_t HASH_BASE = 911382323ULL;  // Change or randomize.

uint64_t hash_add(uint64_t a, uint64_t b) {
  uint64_t c = a + b;
  return c >= HASH_MOD ? c - HASH_MOD : c;
}

uint64_t hash_sub(uint64_t a, uint64_t b) {
  return a >= b ? a - b : a + HASH_MOD - b;
}

uint64_t hash_mul(uint64_t a, uint64_t b) {
#if defined(__SIZEOF_INT128__)
  __uint128_t p = static_cast<__uint128_t>(a) * b;
  return hash_add(static_cast<uint64_t>(p) & HASH_MOD, static_cast<uint64_t>(p >> 61));
#else
  // Portable fallback when no 128-bit type exists: double-and-add modular multiply.
  uint64_t res = 0;
  for (a %= HASH_MOD; b > 0; b >>= 1) {
    if (b & 1) {
      res = hash_add(res, a);
    }
    a = hash_add(a, a);
  }
  return res;
#endif
}

// SplitMix64 mixer.
uint64_t mix64(uint64_t x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

template<typename T>
struct ValueHasher {
  // +1 ensures the result is in [1, HASH_MOD) and never zero; a zero element in a polynomial
  // hash is invisible and enables trivial collisions.
  uint64_t operator()(const T &x) const {
    return mix64(static_cast<uint64_t>(x)) % (HASH_MOD - 1) + 1;
  }
};

template<typename T, typename Hash = ValueHasher<T>>
class RollingHash {
  static std::vector<uint64_t> pow_base;
  std::vector<uint64_t> pref;
  Hash hasher;

  static void ensure_powers(int n) {
    while (static_cast<int>(pow_base.size()) <= n) {
      pow_base.push_back(hash_mul(pow_base.back(), HASH_BASE));
    }
  }

  template<typename It>
  void build(It lo, It hi) {
    pref.clear();
    pref.push_back(0);
    for (; lo != hi; ++lo) {
      pref.push_back(hash_add(hash_mul(pref.back(), HASH_BASE), hasher(*lo)));
    }
    ensure_powers(static_cast<int>(pref.size()) - 1);
  }

 public:
  explicit RollingHash(const Hash &hasher = Hash{}) : hasher(hasher) {
    ensure_powers(0);
    pref.push_back(0);
  }

  template<typename It>
  RollingHash(It lo, It hi, const Hash &hasher = Hash{}) : hasher(hasher) {
    build(lo, hi);
  }

  explicit RollingHash(const std::vector<T> &v, const Hash &hasher = Hash{})
      : RollingHash(v.begin(), v.end(), hasher) {}

  int size() const { return static_cast<int>(pref.size()) - 1; }

  uint64_t hash(int lo, int hi) const {
    assert(0 <= lo && lo <= hi && hi <= size());
    return hash_sub(pref[hi], hash_mul(pref[lo], pow_base[hi - lo]));
  }

  template<typename It>
  static uint64_t hash(It lo, It hi, const Hash &hasher = Hash{}) {
    RollingHash<T, Hash> h(lo, hi, hasher);
    return h.hash(0, h.size());
  }

  static uint64_t hash(const std::vector<T> &v, const Hash &hasher = Hash{}) {
    RollingHash<T, Hash> h(v, hasher);
    return h.hash(0, h.size());
  }

  static uint64_t concat(uint64_t left, uint64_t right, int right_len) {
    assert(right_len >= 0);
    ensure_powers(right_len);
    return hash_add(hash_mul(left, pow_base[right_len]), right);
  }
};

template<typename T, typename Hash>
std::vector<uint64_t> RollingHash<T, Hash>::pow_base(1, 1);

/*** Example Usage ***/

using namespace std;

using PointI = pair<int, int>;

struct PointHasher {
  uint64_t operator()(const PointI &p) const {
    // Pack the two 32-bit coordinates losslessly into one 64-bit key before mixing, so distinct
    // points never collide pre-mix (a small-multiplier combine like a * C + b would).
    uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(p.first)) << 32) |
                   static_cast<uint32_t>(p.second);
    return mix64(key) % (HASH_MOD - 1) + 1;
  }
};

int main() {
  string s = "abracadabra";
  RollingHash<char> hs(s.begin(), s.end());
  assert(hs.hash(0, 4) == hs.hash(7, 11));  // "abra" == "abra"
  assert(hs.hash(0, 4) != hs.hash(3, 7));   // "abra" != "acad"

  string a = "abc", b = "def";
  uint64_t ab = RollingHash<char>::concat(
      RollingHash<char>::hash(a.begin(), a.end()), RollingHash<char>::hash(b.begin(), b.end()),
      static_cast<int>(b.size())
  );
  string c = a + b;
  assert(ab == RollingHash<char>::hash(c.begin(), c.end()));

  vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  v.push_back(1);
  v.push_back(2);
  RollingHash<int> hv(v);
  assert(hv.hash(0, 2) == hv.hash(3, 5));  // Both ranges are [1, 2].
  assert(hv.hash(0, 3) == RollingHash<int>::hash(v.begin(), v.begin() + 3));

  vector<PointI> poly{{1, 2}, {3, 4}};
  RollingHash<PointI, PointHasher> hp(poly);
  assert((hp.hash(0, 2) == RollingHash<PointI, PointHasher>::hash(poly)));
  return 0;
}
