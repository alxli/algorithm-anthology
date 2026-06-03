/*

Provides a small library of non-cryptographic hash functions and hash functors for
common contest keys. These helpers are useful for fingerprinting values, combining
keys, seeding randomized algorithms, and defining `std::unordered_map` keys for
pairs and vectors. They are not suitable for passwords, signatures, or adversarial
security.

The standalone functions below are deterministic. The `int_hasher` functor adds a
per-run random seed before mixing, which is useful for making integer-keyed hash
tables harder to hack in open-test contests.

- `mix32(x)` mixes a 32-bit unsigned integer `x`, using the MurmurHash3 finalizer.
- `mix64(x)` mixes a 64-bit unsigned integer `x`, using the SplitMix64 finalizer.
- `hash32(x)` and `hash64(x)` hash any integer type that can be
  converted to `unsigned int` or `uint64`, respectively.
- `hash_combine32(h, x)` and `hash_combine64(h, x)` fold another already-hashed
  value into an existing hash accumulator.
- `hash_float(x)` and `hash_double(x)` hash floating-point bit patterns, normalizing
  `-0.0` to `0.0`. Different NaN representations may still hash differently.
- `hash_string_fnv1a32(s)` and `hash_string_fnv1a64(s)` compute FNV-1a hashes of
  string `s`.
- `hash_range32(first, last)` and `hash_range64(first, last)` hash a sequence of
  integer-like values.
- `int_hasher<Int>`, `pair_hasher<A, B>`, and `vector_hasher<T>` are hash functors for
  `std::unordered_map` and `std::unordered_set`.
- `generic_hasher<T>` recursively handles integer, pair, and vector keys, and falls
  back to `std::hash<T>` for other hashable types.

Time Complexity:
- O(1) per call to the integer, combiner, and floating-point hash functions.
- O(n) per call to string and range hashing functions, where $n$ is the number of
  elements processed.
- O(n) per call to `vector_hasher<T>::operator()`, where $n$ is the vector size.

Space Complexity:
- O(1) auxiliary.

*/

#include <chrono>
#include <cstddef>
#include <cstring>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

typedef unsigned long long uint64;

unsigned int mix32(unsigned int x) {
  x ^= x >> 16;
  x *= 0x85ebca6bU;
  x ^= x >> 13;
  x *= 0xc2b2ae35U;
  return x ^ (x >> 16);
}

uint64 mix64(uint64 x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

template<class Int>
unsigned int hash32(Int x) {
  return mix32((unsigned int)x);
}

template<class Int>
uint64 hash64(Int x) {
  return mix64((uint64)x);
}

unsigned int hash_combine32(unsigned int h, unsigned int x) {
  return mix32(h ^ (x + 0x9e3779b9U + (h << 6) + (h >> 2)));
}

uint64 hash_combine64(uint64 h, uint64 x) {
  return mix64(h ^ (x + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2)));
}

unsigned int hash_float(float x) {
  if (x == 0.0f) {
    x = 0.0f;  // Normalize -0.0.
  }
  unsigned int bits = 0;
  std::memcpy(&bits, &x, sizeof(bits));
  return mix32(bits);
}

uint64 hash_double(double x) {
  if (x == 0.0) {
    x = 0.0;  // Normalize -0.0.
  }
  uint64 bits = 0;
  std::memcpy(&bits, &x, sizeof(bits));
  return mix64(bits);
}

unsigned int hash_string_fnv1a32(const std::string &s) {
  unsigned int h = 2166136261U;
  for (int i = 0; i < (int)s.size(); i++) {
    h ^= (unsigned char)s[i];
    h *= 16777619U;
  }
  return h;
}

uint64 hash_string_fnv1a64(const std::string &s) {
  uint64 h = 14695981039346656037ULL;
  for (int i = 0; i < (int)s.size(); i++) {
    h ^= (unsigned char)s[i];
    h *= 1099511628211ULL;
  }
  return h;
}

template<class It>
unsigned int hash_range32(It first, It last) {
  unsigned int h = 0;
  for (It it = first; it != last; ++it) {
    h = hash_combine32(h, hash32(*it));
  }
  return h;
}

template<class It>
uint64 hash_range64(It first, It last) {
  uint64 h = 0;
  for (It it = first; it != last; ++it) {
    h = hash_combine64(h, hash64(*it));
  }
  return h;
}

template<class Int>
struct int_hasher {
  std::size_t operator()(Int x) const {
    static const uint64 RANDOM =
        std::chrono::steady_clock::now().time_since_epoch().count();
    return (std::size_t)mix64((uint64)x + RANDOM);
  }
};

template<class T>
struct generic_hasher;

template<class T, bool IsInteger>
struct scalar_hasher {
  std::size_t operator()(const T &x) const {
    return std::hash<T>()(x);
  }
};

template<class T>
struct scalar_hasher<T, true> {
  std::size_t operator()(T x) const {
    return int_hasher<T>()(x);
  }
};

template<class A, class B>
struct pair_hasher {
  std::size_t operator()(const std::pair<A, B> &p) const {
    uint64 h = 0;
    h = hash_combine64(h, (uint64)generic_hasher<A>()(p.first));
    h = hash_combine64(h, (uint64)generic_hasher<B>()(p.second));
    return (std::size_t)h;
  }
};

template<class T>
struct vector_hasher {
  std::size_t operator()(const std::vector<T> &v) const {
    uint64 h = 0;
    for (int i = 0; i < (int)v.size(); i++) {
      h = hash_combine64(h, (uint64)generic_hasher<T>()(v[i]));
    }
    return (std::size_t)h;
  }
};

template<class T>
struct generic_hasher : scalar_hasher<T, std::is_integral<T>::value> {};

template<class A, class B>
struct generic_hasher<std::pair<A, B> > : pair_hasher<A, B> {};

template<class T>
struct generic_hasher<std::vector<T> > : vector_hasher<T> {};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(mix32(123U) == mix32(123U));
  assert(mix32(123U) != mix32(124U));
  assert(hash32(-1) == hash32(-1));
  assert(hash64(123) == mix64(123ULL));
  assert(hash_float(-0.0f) == hash_float(0.0f));
  assert(hash_double(-0.0) == hash_double(0.0));
  assert(hash_string_fnv1a32("abc") == hash_string_fnv1a32("abc"));
  assert(hash_string_fnv1a64("abc") != hash_string_fnv1a64("acb"));

  vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  assert(hash_range32(v.begin(), v.end()) == hash_range32(v.begin(), v.end()));
  assert(hash_range64(v.begin(), v.end()) == hash_range64(v.begin(), v.end()));

  unordered_map<long long, int, int_hasher<long long> > count;
  count[1000000000000LL] = 7;
  assert(count[1000000000000LL] == 7);

  typedef pair<int, int> point;
  unordered_map<point, int, pair_hasher<int, int> > dist;
  dist[point(3, 4)] = 5;
  assert(dist[point(3, 4)] == 5);

  unordered_map<vector<int>, int, vector_hasher<int> > seen;
  seen[v] = 1;
  assert(seen[v] == 1);

  typedef pair<vector<int>, int> state;
  unordered_map<state, int, generic_hasher<state> > dp;
  dp[state(v, 4)] = 9;
  assert(dp[state(v, 4)] == 9);
  return 0;
}
