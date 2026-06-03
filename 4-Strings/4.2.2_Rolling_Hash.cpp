/*

Computes polynomial rolling hashes for sequences, supporting O(1) contiguous
subsequence hash queries after preprocessing. This is useful for probabilistic
string matching, substring equality checks, repeated subarray detection, and
binary-searching over candidate lengths.

The implementation works modulo the Mersenne prime $2^{61} - 1$ and uses
`__uint128_t` for multiplication. The base `HASH_BASE` should be changed or chosen
randomly for open-hacking environments. With a fixed base, hashing remains fast and
practical, but it is still probabilistic and should not be used as proof of
equality when exact verification is required.

By default, each sequence value is cast to `uint64` and mixed. For non-integer
element types, pass a custom value hasher that maps each element to a stable
nonzero value in `[1, HASH_MOD)`.

- `rolling_hash<T>(first, last)` constructs prefix hashes for any iterator range of
  values accepted by the value hasher.
- `rolling_hash<T>(v)` constructs prefix hashes for vector `v`.
- `get(l, r)` returns the hash of the half-open subsequence `[l, r)`.
- `hash(first, last)` returns the hash of an iterator range.
- `hash(v)` returns the hash of vector `v`.
- `concat(left, right, right_len)` returns the hash of the concatenation of a
  sequence with hash `left` and a sequence with hash `right` and length `right_len`.

Time Complexity:
- O(n) per constructor call and whole-sequence hash, where $n$ is the sequence
  length.
- O(1) per call to `get(l, r)` and `concat(left, right, right_len)`.

Space Complexity:
- O(n) for storage of prefix hashes and powers, where $n$ is the maximum sequence
  length processed so far.
- O(1) auxiliary per query.

*/

#include <string>
#include <utility>
#include <vector>

typedef unsigned long long uint64;

const uint64 HASH_MOD = (1ULL << 61) - 1;
const uint64 HASH_BASE = 911382323ULL;  // Change or randomize.

uint64 hash_add(uint64 a, uint64 b) {
  uint64 c = a + b;
  return c >= HASH_MOD ? c - HASH_MOD : c;
}

uint64 hash_sub(uint64 a, uint64 b) {
  return a >= b ? a - b : a + HASH_MOD - b;
}

uint64 hash_mul(uint64 a, uint64 b) {
  __uint128_t p = (__uint128_t)a * b;
  uint64 low = (uint64)p & HASH_MOD;
  uint64 high = (uint64)(p >> 61);
  uint64 res = low + high;
  if (res >= HASH_MOD) {
    res -= HASH_MOD;
  }
  return res;
}

uint64 hash_mix(uint64 x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

template<class T>
struct rolling_value_hasher {
  uint64 operator()(const T &x) const {
    return hash_mix((uint64)x) % (HASH_MOD - 1) + 1;
  }
};

template<class T, class ValueHasher = rolling_value_hasher<T> >
class rolling_hash {
  static std::vector<uint64> pow_base;
  std::vector<uint64> pref;
  ValueHasher value_hasher;

  static void ensure_powers(int n) {
    while ((int)pow_base.size() <= n) {
      pow_base.push_back(hash_mul(pow_base.back(), HASH_BASE));
    }
  }

  template<class It>
  void build(It first, It last) {
    pref.clear();
    pref.push_back(0);
    for (It it = first; it != last; ++it) {
      pref.push_back(hash_add(hash_mul(pref.back(), HASH_BASE),
                              value_hasher(*it)));
    }
    ensure_powers(pref.size() - 1);
  }

 public:
  explicit rolling_hash(const ValueHasher &hasher = ValueHasher())
      : value_hasher(hasher) {
    ensure_powers(0);
    pref.push_back(0);
  }

  template<class It>
  rolling_hash(It first, It last, const ValueHasher &hasher = ValueHasher())
      : value_hasher(hasher) {
    build(first, last);
  }

  explicit rolling_hash(const std::vector<T> &v,
                        const ValueHasher &hasher = ValueHasher())
      : value_hasher(hasher) {
    build(v.begin(), v.end());
  }

  int size() const {
    return (int)pref.size() - 1;
  }

  uint64 get(int l, int r) const {
    return hash_sub(pref[r], hash_mul(pref[l], pow_base[r - l]));
  }

  template<class It>
  static uint64 hash(It first, It last,
                     const ValueHasher &hasher = ValueHasher()) {
    rolling_hash<T, ValueHasher> h(first, last, hasher);
    return h.get(0, h.size());
  }

  static uint64 hash(const std::vector<T> &v,
                     const ValueHasher &hasher = ValueHasher()) {
    rolling_hash<T, ValueHasher> h(v, hasher);
    return h.get(0, h.size());
  }

  static uint64 concat(uint64 left, uint64 right, int right_len) {
    ensure_powers(right_len);
    return hash_add(hash_mul(left, pow_base[right_len]), right);
  }
};

template<class T, class ValueHasher>
std::vector<uint64> rolling_hash<T, ValueHasher>::pow_base(1, 1);

/*** Example Usage ***/

#include <cassert>
using namespace std;

typedef pair<int, int> point;

struct point_hasher {
  uint64 operator()(const point &p) const {
    return hash_mix((uint64)p.first * 1000003ULL + (uint64)p.second) %
               (HASH_MOD - 1) + 1;
  }
};

int main() {
  string s = "abracadabra";
  rolling_hash<char> hs(s.begin(), s.end());
  assert(hs.get(0, 4) == hs.get(7, 11));  // "abra" == "abra"
  assert(hs.get(0, 4) != hs.get(3, 7));   // "abra" != "acad"

  string a = "abc", b = "def";
  uint64 ab =
      rolling_hash<char>::concat(rolling_hash<char>::hash(a.begin(), a.end()),
                                 rolling_hash<char>::hash(b.begin(), b.end()),
                                 b.size());
  string c = a + b;
  assert(ab == rolling_hash<char>::hash(c.begin(), c.end()));

  vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  v.push_back(1);
  v.push_back(2);
  rolling_hash<int> hv(v);
  assert(hv.get(0, 2) == hv.get(3, 5));  // [1, 2] == [1, 2]
  assert(hv.get(0, 3) == rolling_hash<int>::hash(v.begin(), v.begin() + 3));

  vector<point> poly;
  poly.push_back(point(1, 2));
  poly.push_back(point(3, 4));
  rolling_hash<point, point_hasher> hp(poly);
  assert((hp.get(0, 2) == rolling_hash<point, point_hasher>::hash(poly)));
  return 0;
}
