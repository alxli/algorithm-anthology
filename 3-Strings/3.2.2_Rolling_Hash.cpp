/*

Computes polynomial rolling hashes for sequences, supporting O(1) contiguous subsequence hash
queries after preprocessing. This is useful for probabilistic string matching, substring equality
checks, repeated subarray detection, and binary-searching over candidate lengths.

Given a sequence $a_0, a_1, \ldots, a_{n-1}$ and a value hash $h(a_i)$ for each element, the hash is
the polynomial $H(a) = \sum_{i=0}^{n-1} h(a_i) B^{n-1-i} \pmod M$, where $B$ is `HASH_BASE` and $M$
is `HASH_MOD`. Equivalently, it is built left-to-right by the recurrence
`pref[i + 1] = pref[i] * HASH_BASE + h(a[i])`. A subsequence hash for [`l`, `r`) is obtained by
subtracting away the prefix before $l$: `pref[r] - pref[l] * pow(HASH_BASE, r - l)`.

The implementation works modulo the Mersenne prime $2^{61} - 1$, using `__uint128_t` for the
multiplication where available and falling back to a double-and-add modular multiply otherwise. The
base `HASH_BASE` should be changed or chosen randomly for open-hacking environments. With a fixed
base, hashing remains fast and practical, but it is still probabilistic and should not be used as
proof of equality when exact verification is required.

By default, each sequence value is cast to `uint64_t` and mixed. For non-integer element types, pass
a custom value hasher that maps each element to a stable nonzero value in [$1$, `HASH_MOD`).

- `RollingHash<T, ValueHasher>(first, last)` constructs prefix hashes for any iterator range of
  values accepted by the value hasher.
- `RollingHash<T, ValueHasher>(v)` constructs prefix hashes for vector `v`.
- `get(l, r)` returns the hash of the half-open subsequence [`l`, `r`).
- `hash(first, last)` returns the hash of an iterator range.
- `hash(v)` returns the hash of vector `v`.
- `concat(left, right, right_len)` returns the hash of the concatenation of a sequence with hash
  `left` and a sequence with hash `right` and length `right_len`.

Time Complexity:
- O(n) per constructor call and whole-sequence hash, where $n$ is the sequence length.
- O(1) per call to `get(l, r)` and `concat(left, right, right_len)`.

Space Complexity:
- O(n) for storage of prefix hashes and powers, where $n$ is the maximum sequence length processed
  so far.
- O(1) auxiliary per query.

*/

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
struct RollingValueHasher {
  // +1 ensures the result is in [$1$, HASH_MOD) and never zero; a zero element in a polynomial
  // hash is invisible and enables trivial collisions.
  uint64_t operator()(const T &x) const { return mix64((uint64_t)x) % (HASH_MOD - 1) + 1; }
};

template<typename T, typename ValueHasher = RollingValueHasher<T>>
class RollingHash {
  static std::vector<uint64_t> pow_base;
  std::vector<uint64_t> pref;
  ValueHasher value_hasher;

  static void ensure_powers(int n) {
    while (static_cast<int>(pow_base.size()) <= n) {
      pow_base.push_back(hash_mul(pow_base.back(), HASH_BASE));
    }
  }

  template<typename It>
  void build(It first, It last) {
    pref.clear();
    pref.push_back(0);
    for (; first != last; ++first) {
      pref.push_back(hash_add(hash_mul(pref.back(), HASH_BASE), value_hasher(*first)));
    }
    ensure_powers(static_cast<int>(pref.size()) - 1);
  }

 public:
  explicit RollingHash(const ValueHasher &hasher = ValueHasher()) : value_hasher(hasher) {
    ensure_powers(0);
    pref.push_back(0);
  }

  template<typename It>
  RollingHash(It first, It last, const ValueHasher &hasher = ValueHasher()) : value_hasher(hasher) {
    build(first, last);
  }

  explicit RollingHash(const std::vector<T> &v, const ValueHasher &hasher = ValueHasher())
      : value_hasher(hasher) {
    build(v.begin(), v.end());
  }

  int size() const { return static_cast<int>(pref.size()) - 1; }

  uint64_t get(int lo, int hi) const {
    return hash_sub(pref[hi], hash_mul(pref[lo], pow_base[hi - lo]));
  }

  template<typename It>
  static uint64_t hash(It first, It last, const ValueHasher &hasher = ValueHasher()) {
    RollingHash<T, ValueHasher> h(first, last, hasher);
    return h.get(0, h.size());
  }

  static uint64_t hash(const std::vector<T> &v, const ValueHasher &hasher = ValueHasher()) {
    RollingHash<T, ValueHasher> h(v, hasher);
    return h.get(0, h.size());
  }

  static uint64_t concat(uint64_t left, uint64_t right, int right_len) {
    ensure_powers(right_len);
    return hash_add(hash_mul(left, pow_base[right_len]), right);
  }
};

template<typename T, typename ValueHasher>
std::vector<uint64_t> RollingHash<T, ValueHasher>::pow_base(1, 1);

/*** Example Usage ***/

#include <cassert>
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
  assert(hs.get(0, 4) == hs.get(7, 11));  // "abra" == "abra"
  assert(hs.get(0, 4) != hs.get(3, 7));   // "abra" != "acad"

  string a = "abc", b = "def";
  uint64_t ab = RollingHash<char>::concat(
      RollingHash<char>::hash(a.begin(), a.end()), RollingHash<char>::hash(b.begin(), b.end()),
      b.size()
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
  assert(hv.get(0, 2) == hv.get(3, 5));  // [$1$, 2] == [$1$, 2]
  assert(hv.get(0, 3) == RollingHash<int>::hash(v.begin(), v.begin() + 3));

  vector<PointI> poly{{1, 2}, {3, 4}};
  RollingHash<PointI, PointHasher> hp(poly);
  assert((hp.get(0, 2) == RollingHash<PointI, PointHasher>::hash(poly)));
  return 0;
}
