/*

Computes byte frequency tables and empirical information-theoretic quantities for strings. These
primitives are useful when analyzing compression, building coding schemes such as Huffman coding, or
comparing symbol distributions.

Shannon entropy measures the average uncertainty of a random symbol, in bits. It is called
*empirical* here because the probabilities are estimated from the string itself rather than supplied
by a known source. The *order* of the estimate is how much preceding context it conditions on: an
order-0 model treats symbols as independent and looks only at how often each byte occurs, while an
order-$k$ model conditions each symbol on the $k$ bytes before it.

For symbol probabilities $p_i$, the order-0 empirical entropy is $H = -\sum_i p_i \log_2 p_i$ bits
per symbol, a lower bound on the average number of bits per symbol achievable by any uniquely
decodable code under that independent model. Because it ignores ordering, `"abab"` and `"aabb"` have
identical order-0 entropy. Real data has structure across positions, so conditioning lowers the
estimate: order-$k$ entropy never exceeds order-$(k - 1)$ entropy, which is why practical
compressors beat the order-0 bound.

- `byte_frequencies(s)` returns a length-256 table of byte counts in string `s`.
- `entropy(freq)` returns order-0 empirical entropy in bits per symbol for a frequency table.
- `entropy(s)` returns order-0 empirical entropy in bits per symbol for string `s`.
- `conditional_entropy(s, order = 1)` returns the order-`order` empirical entropy of `s` in bits per
  symbol, that is, the average uncertainty of each byte given the `order` bytes preceding it.
  `conditional_entropy(s, 0)` equals `entropy(s)`, and raising `order` never increases the result. A
  string of at most `order` bytes has no complete context, so the result is $0$.
- `expected_code_length(freq, length)` returns the average encoded bits per symbol for code lengths
  `length[c]`. The two input vectors must have the same size.

Time Complexity:
- O(n + m) per call to `entropy(s)`, where $n$ is the string length and $m = 256$.
- O(m) per call to `entropy(freq)` and `expected_code_length(freq, length)`.
- O(n*k) expected per call to `conditional_entropy(s, k)`, for hashing the context of each position.

Space Complexity:
- O(m) for the table returned by `byte_frequencies()` and O(m) auxiliary for `entropy(s)`.
- O(n*k) auxiliary for `conditional_entropy(s, k)`, since at most $n$ distinct contexts are stored.
- O(1) auxiliary for the other operations.

*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
using std::string;

std::vector<int> byte_frequencies(const string &s) {
  std::vector<int> freq(256, 0);
  for (unsigned char c : s) {
    freq[c]++;
  }
  return freq;
}

double entropy(const std::vector<int> &freq) {
  int64_t total = 0;
  for (int count : freq) {
    total += count;
  }
  if (total == 0) {
    return 0;
  }
  double res = 0;
  for (int count : freq) {
    if (count == 0) {
      continue;
    }
    double p = static_cast<double>(count) / total;
    res -= p * std::log2(p);
  }
  return res;
}

double entropy(const string &s) {
  return entropy(byte_frequencies(s));
}

double conditional_entropy(const string &s, int order = 1) {
  assert(order >= 0);
  int n = static_cast<int>(s.size());
  if (n <= order) {
    return 0;
  }
  // Contexts are keyed by their bytes rather than indexed by an alphabet-sized table, so the space
  // used depends on how many distinct contexts occur instead of on the alphabet size raised to the
  // order. Every context is exactly that many bytes, so splitting an event key is unambiguous.
  std::unordered_map<string, int> context_count, event_count;
  for (int i = order; i < n; i++) {
    string context = s.substr(i - order, order);
    context_count[context]++;
    event_count[context + s[i]]++;
  }
  double total = n - order;
  double res = 0;
  for (const auto &[event, count] : event_count) {
    double conditional = count / static_cast<double>(context_count[event.substr(0, order)]);
    res -= (count / total) * std::log2(conditional);
  }
  return res;
}

double expected_code_length(const std::vector<int> &freq, const std::vector<int> &length) {
  assert(freq.size() == length.size());
  int64_t total = 0;
  for (int count : freq) {
    total += count;
  }
  if (total == 0) {
    return 0;
  }
  double res = 0;
  for (int i = 0; i < static_cast<int>(freq.size()); i++) {
    res += static_cast<double>(freq[i]) * length[i] / total;
  }
  return res;
}

/*** Example Usage ***/

#include <cmath>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  vector<int> freq = byte_frequencies("aabb");
  assert(freq['a'] == 2);
  assert(freq['b'] == 2);
  assert(EQ(entropy(freq), 1.0));
  assert(EQ(entropy("aaaa"), 0.0));

  vector<int> length(256, 0);
  length['a'] = 1;
  length['b'] = 1;
  assert(EQ(expected_code_length(freq, length), 1.0));

  // Order 0 conditions on nothing, so it reproduces the plain entropy.
  assert(EQ(conditional_entropy("aabb", 0), entropy("aabb")));

  // Order-0 entropy ignores ordering; higher orders do not. In "abab" the previous symbol
  // determines the next one exactly, so no uncertainty remains.
  assert(EQ(entropy("abab"), entropy("aabb")));
  assert(EQ(conditional_entropy("abab"), 0.0));
  assert(EQ(conditional_entropy("aabb"), 2.0 / 3));
  assert(conditional_entropy("aabb") < entropy("aabb"));

  // Raising the order never increases the estimate, and too little text yields no context at all.
  assert(conditional_entropy("abcabcabc", 2) <= conditional_entropy("abcabcabc", 1));
  assert(EQ(conditional_entropy("a"), 0.0));
  assert(EQ(conditional_entropy("abc", 3), 0.0));
  return 0;
}
