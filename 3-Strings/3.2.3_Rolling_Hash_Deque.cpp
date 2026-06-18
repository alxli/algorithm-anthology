/*

Maintain a polynomial rolling hash over a sequence of elements of an arbitrary type `T`, under
insertion and deletion at either end, in O(1) time per operation. Each element is first reduced to a
nonnegative integer "digit" by a user-supplied hasher, so any comparable type (characters, integers,
whole words, tuples) can be hashed. This is the deque generalization of the classic Rabin-Karp
sliding window: pushing at the back and popping from the front is exactly a moving window, and this
structure additionally supports pushing at the front and popping from the back.

The sequence of digits is hashed with the front element as the most significant digit, so a sequence
of length $n$ has hash $\sum_{i=0}^{n-1} a_i B^{n-1-i}$ (the front element $a_0$ carries the highest
power). This makes the four end operations pure O(1) updates of a running accumulator:

- `push_back(c)`:  $H \to H \cdot B + c$ (Horner's step; the existing weights all shift up).
- `pop_front()`:   $H \to H - a_0 \cdot B^{n-1}$ (drop the most significant term).
- `push_front(c)`: $H \to c \cdot B^{n} + H$ (the new front becomes the most significant term).
- `pop_back()`:    $H \to (H - a_{n-1}) \cdot B^{-1}$ (drop the constant term, then renormalize).

Only `pop_back()` needs a modular inverse of the base, which is precomputed once. The weight of the
front element, $B^{n-1}$, is maintained incrementally (multiplied by $B$ on a push, by $B^{-1}$ on a
pop), so no power table or length-dependent exponentiation is needed. Each element's digit is stored
when pushed, so popping never re-invokes the hasher. Because the hash depends only on the current
contents, two deques with equal sequences hash equally regardless of how they were built, and a
sliding window can be compared against a fixed target in O(1) per step.

`MOD1` and `MOD2` are two distinct primes, each kept below $2^{31}$ so that the product of any two
residues fits in a 64-bit integer; hashing modulo both and comparing the resulting pair makes a
collision astronomically unlikely. `BASE1` and `BASE2` are the corresponding polynomial bases, which
should exceed the largest digit value and may be randomized per run to resist adversarial inputs.

The hasher's output must be nonnegative. As with any polynomial hash, comparing sequences of
different lengths is only safe when digits are positive (for example, map an alphabet to $[1, B)$)
so that a leading zero cannot make a shorter sequence collide with a longer one.

- `HashDeque<T, Hash>(hasher)` constructs an empty deque whose `hasher` maps each element to its
  nonnegative integer digit. For integer-like `T` the default hasher casts the element to
  `uint64_t`, so `HashDeque<char>` and `HashDeque<int>` work with no hasher supplied.
- `size()` returns the number of elements, and `empty()` whether there are none.
- `push_back(x)` and `push_front(x)` append `x` at the back or front.
- `pop_back()` and `pop_front()` remove the back or front element; the deque must be nonempty.
- `hash()` returns the pair of hashes of the current sequence.
- `a == b` (and `a != b`) compares two deques: it checks equal length, then equal hash pairs, so two
  deques hold equal sequences with high probability if and only if they compare equal.

Time Complexity:
- O(1) per call to every operation, plus one hasher evaluation per `push_back()` / `push_front()`.

Space Complexity:
- O(n) for storage, where $n$ is the current number of elements.
- O(1) auxiliary per operation.

*/

#include <cassert>
#include <cstdint>
#include <deque>
#include <utility>

template<class T>
struct identity_digit {
  uint64_t operator()(const T &x) const { return static_cast<uint64_t>(x); }
};

template<class T = int, class Hash = identity_digit<T>>
class HashDeque {
  static const uint64_t MOD1 = 1000000007, MOD2 = 1000000009;  // Distinct primes below 2^31.
  static const uint64_t BASE1 = 131, BASE2 = 137;  // Should exceed the maximum digit value.

  Hash hasher;                  // Maps an element to its nonnegative integer digit.
  std::deque<uint64_t> digits;  // Per-element digit values, retained so pops never re-hash.
  uint64_t h1 = 0, h2 = 0;      // Hash of the current sequence modulo MOD1, MOD2.
  uint64_t top1, top2;          // B^(len-1): weight of the front element (B^-1 while empty).
  uint64_t inv1, inv2;          // Modular inverses of BASE1, BASE2, for pop_back().

  // Products and sums of residues below 2^30 stay under 2^60, so 64-bit arithmetic never overflows.
  static uint64_t powmod(uint64_t b, uint64_t e, uint64_t m) {
    uint64_t res = 1;
    for (b %= m; e > 0; e >>= 1) {
      if (e & 1) {
        res = res * b % m;
      }
      b = b * b % m;
    }
    return res;
  }

 public:
  explicit HashDeque(Hash hasher = Hash()) : hasher(hasher) {
    inv1 = powmod(BASE1, MOD1 - 2, MOD1);
    inv2 = powmod(BASE2, MOD2 - 2, MOD2);
    top1 = inv1;  // B^(0-1) = B^-1, so the first push lifts the front weight to B^0 = 1.
    top2 = inv2;
  }

  int size() const { return static_cast<int>(digits.size()); }
  bool empty() const { return digits.empty(); }
  std::pair<uint64_t, uint64_t> hash() const { return {h1, h2}; }

  void push_back(const T &x) {
    uint64_t d = hasher(x), c1 = d % MOD1, c2 = d % MOD2;
    h1 = (h1 * BASE1 + c1) % MOD1;
    h2 = (h2 * BASE2 + c2) % MOD2;
    top1 = top1 * BASE1 % MOD1;
    top2 = top2 * BASE2 % MOD2;
    digits.push_back(d);
  }

  void push_front(const T &x) {
    uint64_t d = hasher(x), c1 = d % MOD1, c2 = d % MOD2;
    top1 = top1 * BASE1 % MOD1;  // Now B^len: the weight the new front element receives.
    top2 = top2 * BASE2 % MOD2;
    h1 = (c1 * top1 + h1) % MOD1;
    h2 = (c2 * top2 + h2) % MOD2;
    digits.push_front(d);
  }

  void pop_front() {
    assert(!digits.empty());
    uint64_t d = digits.front(), c1 = d % MOD1, c2 = d % MOD2;
    h1 = (h1 + MOD1 - c1 * top1 % MOD1) % MOD1;
    h2 = (h2 + MOD2 - c2 * top2 % MOD2) % MOD2;
    top1 = top1 * inv1 % MOD1;
    top2 = top2 * inv2 % MOD2;
    digits.pop_front();
  }

  void pop_back() {
    assert(!digits.empty());
    uint64_t d = digits.back(), c1 = d % MOD1, c2 = d % MOD2;
    h1 = (h1 + MOD1 - c1) % MOD1 * inv1 % MOD1;
    h2 = (h2 + MOD2 - c2) % MOD2 * inv2 % MOD2;
    top1 = top1 * inv1 % MOD1;
    top2 = top2 * inv2 % MOD2;
    digits.pop_back();
  }

  // Compares lengths first, so sequences of different lengths never compare equal.
  friend bool operator==(const HashDeque &a, const HashDeque &b) {
    return a.digits.size() == b.digits.size() && a.h1 == b.h1 && a.h2 == b.h2;
  }

  friend bool operator!=(const HashDeque &a, const HashDeque &b) { return !(a == b); }
};

/*** Example Usage ***/

#include <functional>
#include <string>
using namespace std;

int main() {
  // Default hasher: characters hash by their code point.
  HashDeque<char> hd, hdrev;
  for (char c : string("abc")) {
    hd.push_back(c);
  }
  for (char c : string("cba")) {
    hdrev.push_front(c);  // Prepending c, b, a also yields "abc".
  }
  assert(hd.size() == 3 && hd == hdrev);

  // Slide a width-3 window across the text like Rabin-Karp: push_back + pop_front.
  string text = "Xabcab";
  auto target = hd.hash();  // Hash of "abc".
  HashDeque<char> window;
  for (int i = 0; i < 3; i++) {
    window.push_back(text[i]);  // "Xab".
  }
  bool found = (window.hash() == target);
  for (int i = 3; i < static_cast<int>(text.size()); i++) {
    window.push_back(text[i]);
    window.pop_front();
    found = found || (window.hash() == target);
  }
  assert(found);  // "abc" occurs in "Xabcab".

  // pop_back undoes a push_back, restoring an earlier state's hash.
  HashDeque<char> d;
  d.push_back('a');
  d.push_back('b');
  auto ab = d.hash();
  d.push_back('c');
  d.pop_back();
  assert(d.hash() == ab);  // Back to "ab".

  // An arbitrary element type via a custom hasher: a deque of whole words.
  auto whash = [](const string &w) -> uint64_t { return std::hash<string>{}(w); };
  HashDeque<string, decltype(whash)> sentence(whash), reversed(whash);
  for (string w : {string("the"), string("quick"), string("fox")}) {
    sentence.push_back(w);
  }
  for (string w : {string("fox"), string("quick"), string("the")}) {
    reversed.push_front(w);  // Same word sequence, assembled from the front.
  }
  assert(sentence == reversed);

  sentence.pop_back();  // Drop "fox", leaving the words "the quick".
  HashDeque<string, decltype(whash)> prefix(whash);
  prefix.push_back("the");
  prefix.push_back("quick");
  assert(sentence == prefix);
  return 0;
}
