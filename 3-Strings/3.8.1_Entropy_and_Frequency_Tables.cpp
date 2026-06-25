/*

Computes byte frequency tables and empirical information-theoretic quantities for strings. These
primitives are useful when analyzing compression, building coding schemes such as Huffman coding, or
comparing symbol distributions.

For a string with symbol probabilities $p_i$, empirical entropy is $H = -\sum_i p_i \log_2 p_i$ bits
per symbol. This is a lower bound on the average number of bits per symbol achievable by any
uniquely decodable code for the same independent symbol model.

- `byte_frequencies(s)` returns a length-256 table of byte counts in string `s`.
- `entropy(freq)` returns empirical entropy in bits per symbol for a frequency table.
- `entropy(s)` returns empirical entropy in bits per symbol for string `s`.
- `expected_code_length(freq, length)` returns the average encoded bits per symbol for code lengths
  `length[c]`.

Time Complexity:
- O(n + m) per call to `entropy(s)`, where $n$ is the string length and $m = 256$.
- O(m) per call to `entropy(freq)` and `expected_code_length(freq, length)`.

Space Complexity:
- O(m) auxiliary heap space for `byte_frequencies(s)`.
- O(1) auxiliary for the other operations.

*/

#include <cmath>
#include <string>
#include <vector>
using std::string;

std::vector<int> byte_frequencies(const string &s) {
  std::vector<int> freq(256, 0);
  for (int i = 0; i < static_cast<int>(s.size()); i++) {
    freq[static_cast<unsigned char>(s[i])]++;
  }
  return freq;
}

double entropy(const std::vector<int> &freq) {
  int total = 0;
  for (int i = 0; i < static_cast<int>(freq.size()); i++) {
    total += freq[i];
  }
  if (total == 0) {
    return 0;
  }
  double res = 0;
  for (int i = 0; i < static_cast<int>(freq.size()); i++) {
    if (freq[i] == 0) {
      continue;
    }
    double p = static_cast<double>(freq[i]) / total;
    res -= p * (log(p) / log(2.0));
  }
  return res;
}

double entropy(const string &s) {
  return entropy(byte_frequencies(s));
}

double expected_code_length(const std::vector<int> &freq, const std::vector<int> &length) {
  int total = 0;
  for (int i = 0; i < static_cast<int>(freq.size()); i++) {
    total += freq[i];
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

#include <cassert>
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
  return 0;
}
