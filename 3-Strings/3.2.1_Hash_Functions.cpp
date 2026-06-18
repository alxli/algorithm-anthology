/*

Provides a small library of non-cryptographic hash functions and hash functors for common contest
keys. These functions are useful for fingerprinting values, combining keys, seeding randomized
algorithms, and defining `std::unordered_map` keys for pairs, tuples, and vectors. They are not
suitable for passwords, signatures, or adversarial security.

The standalone functions below are deterministic. The `IntHasher` functor adds a per-run random
seed before mixing, which is useful for making integer-keyed hash tables harder to hack in open-test
contests.

The composite hashers separate the mixing primitive (`mix64`/`hash_combine64`) from each type's
traversal of its parts, so swapping in a different mixing function only requires editing one place.

FNV-1a (Fowler-Noll-Vo) is a tiny hash for variable-length byte sequences. Starting from a fixed
offset basis, it XORs each byte into the accumulator and then multiplies by a fixed prime; the `1a`
variant XORs before multiplying, which mixes slightly better than the original FNV-1. Reach for it
when the key is a string or byte buffer, whereas `mix32` and `mix64` are mixers for a single
fixed-width integer. Its appeal is simplicity: a few lines, no lookup tables, and no seed, with
distribution good enough for the short keys common in contests. For very long inputs a block hash
such as MurmurHash or xxHash is faster and mixes more thoroughly, and because FNV-1a is unseeded it
offers no protection against adversarial inputs.

- `mix32(x)` mixes a 32-bit unsigned integer `x`, using MurmurHash3's fmix32 mixer.
- `mix64(x)` mixes a 64-bit unsigned integer `x`, using the SplitMix64 mixer.
- `hash32(x)` and `hash64(x)` hash any integer type that can be converted to `uint32_t` or
  `uint64_t`, respectively.
- `hash_combine32(h, x)` and `hash_combine64(h, x)` fold another already-hashed value into an
  existing hash accumulator.
- `hash_float(x)` and `hash_double(x)` hash floating-point bit patterns, normalizing `-0.0` to
  `0.0`. Different NaN representations may still hash differently.
- `hash_string_fnv1a32(s)` and `hash_string_fnv1a64(s)` compute FNV-1a hashes of string `s`.
- `hash_range32(first, last)` and `hash_range64(first, last)` hash a sequence of integer-like
  values.
- `PairIntHasher` is a self-contained hasher for `std::pair<int, int>` keys, written without any
  dependency on the templates below so that it can be copy-pasted on its own.
- `IntHasher<Int>`, `PairHasher<A, B>`, `TupleHasher<Ts...>`, and `VectorHasher<T>` are hash
  functors passed as the third template argument of `std::unordered_map` or `std::unordered_set`.
- `GenericHasher<T>` recursively handles integer, pair, tuple, and vector keys, and falls back to
  `std::hash<T>` for other hashable types.
- Specializations of `std::hash` for `std::pair` and `std::tuple` are also provided, letting those
  keys be used in `std::unordered_map`/`std::unordered_set` without an explicit hasher argument.
  Specializing `std::hash` for purely standard types is technically nonstandard, so the functor
  forms above are the portable alternative. (Vector keys are left to the functors to avoid clashing
  with the standard `std::hash<std::vector<bool>>`.)

Time Complexity:
- O(1) per call to the integer, combiner, and floating-point hash functions.
- O(n) per call to string and range hashing functions, where $n$ is the number of elements
  processed.
- O(n) per call to `VectorHasher<T>::operator()`, where $n$ is the vector size.

Space Complexity:
- O(1) auxiliary.

*/

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// MurmurHash3's fmix32 mixer.
uint32_t mix32(uint32_t x) {
  x ^= x >> 16;
  x *= 0x85ebca6bU;
  x ^= x >> 13;
  x *= 0xc2b2ae35U;
  return x ^ (x >> 16);
}

// SplitMix64 mixer.
uint64_t mix64(uint64_t x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

template<typename Int>
uint32_t hash32(Int x) {
  return mix32(static_cast<uint32_t>(x));
}

template<typename Int>
uint64_t hash64(Int x) {
  return mix64(static_cast<uint64_t>(x));
}

uint32_t hash_combine32(uint32_t h, uint32_t x) {
  return mix32(h ^ (x + 0x9e3779b9U + (h << 6) + (h >> 2)));
}

uint64_t hash_combine64(uint64_t h, uint64_t x) {
  return mix64(h ^ (x + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2)));
}

uint32_t hash_float(float x) {
  if (x == 0.0f) {
    x = 0.0f;  // Normalize -0.0.
  }
  uint32_t bits = 0;
  std::memcpy(&bits, &x, sizeof(bits));
  return mix32(bits);
}

uint64_t hash_double(double x) {
  if (x == 0.0) {
    x = 0.0;  // Normalize -0.0.
  }
  uint64_t bits = 0;
  std::memcpy(&bits, &x, sizeof(bits));
  return mix64(bits);
}

uint32_t hash_string_fnv1a32(const std::string &s) {
  uint32_t h = 2166136261U;
  for (unsigned char c : s) {
    h ^= c;
    h *= 16777619U;
  }
  return h;
}

uint64_t hash_string_fnv1a64(const std::string &s) {
  uint64_t h = 14695981039346656037ULL;
  for (unsigned char c : s) {
    h ^= c;
    h *= 1099511628211ULL;
  }
  return h;
}

template<typename It>
uint32_t hash_range32(It first, It last) {
  uint32_t h = 0;
  for (It it = first; it != last; ++it) {
    h = hash_combine32(h, hash32(*it));
  }
  return h;
}

template<typename It>
uint64_t hash_range64(It first, It last) {
  uint64_t h = 0;
  for (It it = first; it != last; ++it) {
    h = hash_combine64(h, hash64(*it));
  }
  return h;
}

template<typename Int>
struct IntHasher {
  std::size_t operator()(Int x) const {
    static const uint64_t RAND_SEED = std::chrono::steady_clock::now().time_since_epoch().count();
    return static_cast<std::size_t>(mix64(static_cast<uint64_t>(x) + RAND_SEED));
  }
};

template<typename T>
struct GenericHasher;

// Use std::hash by default.
template<typename T, bool IsInteger>
struct ScalarHasher {
  std::size_t operator()(const T &x) const { return std::hash<T>()(x); }
};

// Optional: Use our seeded hasher for ints in open-hacking environments.
template<typename T>
struct ScalarHasher<T, true> {
  std::size_t operator()(T x) const { return IntHasher<T>{}(x); }
};

// A self-contained hasher for std::pair<int, int> keys: copy just this struct when you do not need
// the rest of this file. The two 32-bit halves pack losslessly into one 64-bit word, which the
// SplitMix64 mixer (the same one used by mix64 above) then scrambles. Add a random seed to `key`
// before mixing, as IntHasher does, if you need anti-hacking protection.
struct PairIntHasher {
  std::size_t operator()(const std::pair<int, int> &p) const {
    uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(p.first)) << 32) |
                   static_cast<uint32_t>(p.second);
    key += 0x9e3779b97f4a7c15ULL;
    key = (key ^ (key >> 30)) * 0xbf58476d1ce4e5b9ULL;
    key = (key ^ (key >> 27)) * 0x94d049bb133111ebULL;
    return static_cast<std::size_t>(key ^ (key >> 31));
  }
};

template<typename A, typename B>
struct PairHasher {
  std::size_t operator()(const std::pair<A, B> &p) const {
    uint64_t h = 0;
    h = hash_combine64(h, static_cast<uint64_t>(GenericHasher<A>()(p.first)));
    h = hash_combine64(h, static_cast<uint64_t>(GenericHasher<B>()(p.second)));
    return static_cast<std::size_t>(h);
  }
};

template<typename T>
struct VectorHasher {
  std::size_t operator()(const std::vector<T> &v) const {
    uint64_t h = 0;
    for (const auto &x : v) {
      h = hash_combine64(h, static_cast<uint64_t>(GenericHasher<T>()(x)));
    }
    h = hash_combine64(h, v.size());  // Mix in the length so different sizes (e.g. empty) separate.
    return static_cast<std::size_t>(h);
  }
};

template<class... Ts>
struct TupleHasher {
  std::size_t operator()(const std::tuple<Ts...> &t) const {
    uint64_t h = 0;
    std::apply(
        [&](const Ts &...xs) {
          ((h = hash_combine64(h, static_cast<uint64_t>(GenericHasher<Ts>()(xs)))), ...);
        },
        t
    );
    return static_cast<std::size_t>(h);
  }
};

template<typename T>
struct GenericHasher : ScalarHasher<T, std::is_integral<T>::value> {};

template<typename A, typename B>
struct GenericHasher<std::pair<A, B>> : PairHasher<A, B> {};

template<class... Ts>
struct GenericHasher<std::tuple<Ts...>> : TupleHasher<Ts...> {};

template<typename T>
struct GenericHasher<std::vector<T>> : VectorHasher<T> {};

// Specializing std::hash lets pair and tuple keys be used directly in std::unordered_map and
// std::unordered_set, with no explicit hasher template argument. The C++ standard only sanctions
// specializing std::hash when a user-defined type is involved, so the functors above remain the
// portable choice; these specializations are a widely supported contest convenience. Vector keys
// are intentionally left out here to avoid clashing with the standard library's own
// std::hash<std::vector<bool>>; pass VectorHasher or GenericHasher explicitly for those.
namespace std {

template<typename A, typename B>
struct hash<pair<A, B>> {
  std::size_t operator()(const pair<A, B> &p) const { return ::GenericHasher<pair<A, B>>()(p); }
};

template<class... Ts>
struct hash<tuple<Ts...>> {
  std::size_t operator()(const tuple<Ts...> &t) const { return ::GenericHasher<tuple<Ts...>>()(t); }
};

}  // namespace std

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

  unordered_map<int64_t, int, IntHasher<int64_t>> count;
  count[1000000000000LL] = 7;
  assert(count[1000000000000LL] == 7);

  using Point = pair<int, int>;
  unordered_map<Point, int, PairHasher<int, int>> dist;
  dist[Point(3, 4)] = 5;
  assert(dist[Point(3, 4)] == 5);

  // The standalone PairIntHasher is the quick choice for pair<int, int> keys.
  unordered_map<Point, int, PairIntHasher> dist_fast;
  dist_fast[Point(3, 4)] = 5;
  assert(dist_fast[Point(3, 4)] == 5);

  unordered_map<vector<int>, int, VectorHasher<int>> seen;
  seen[v] = 1;
  assert(seen[v] == 1);

  using State = pair<vector<int>, double>;
  unordered_map<State, int, GenericHasher<State>> dp;
  dp[State(v, 4.5)] = 9;
  assert(dp[State(v, 4.5)] == 9);

  // Tuple keys via the explicit functor.
  using Triple = tuple<int, char, double>;
  unordered_map<Triple, int, GenericHasher<Triple>> grid;
  grid[Triple(1, 'a', 3.14)] = 6;
  assert(grid[Triple(1, 'a', 3.14)] == 6);

  // With the std::hash specializations, pair and tuple keys need no hasher argument.
  unordered_map<Point, int> dist_default;
  dist_default[Point(3, 4)] = 5;
  assert(dist_default[Point(3, 4)] == 5);

  unordered_map<Triple, int> grid_default;
  grid_default[Triple(1, 'a', 3.14)] = 6;
  assert(grid_default[Triple(1, 'a', 3.14)] == 6);
  return 0;
}
