/*

A positional numeral is a sequence of digits weighted by powers of its base, so converting between
bases is repeated division: divide by the target base, record the remainder as the next digit, and
continue with the quotient. Doing that division on a digit vector rather than on a machine integer
is what lets `convert_base()` handle values far larger than any built-in type. Roman numerals are
the counterexample that shows what positional notation buys, being additive with a subtractive
special case and having no digit for zero.

- `to_base(x, b = 10)` returns the digits of the unsigned integer `x` in base `b`, where index $0$
  of the result stores the least significant digit.
- `to_roman(x)` returns the Roman numeral representation of the unsigned integer `x` as a string.
- `from_roman(s)` returns the value of the Roman numeral `s`, which must consist of Roman digits.
  Subtractive pairs such as `"IX"` are handled, but the numeral is not otherwise validated, so a
  malformed one such as `"IC"` simply returns the value its symbols imply.
- `convert_base(d, a, b)` converts an integer in base `a` as a vector `d` of digits (where `d[0]` is
  the least significant digit) to base `b` as a vector of digits (again with index $0$ holding the
  least significant digit). This uses repeated long division, so the value itself does not need to
  fit in a machine integer.

Overflow warning: Each intermediate `rem*a + digit` must fit in `uint64_t`.

Time Complexity:
- O(log_b(x + 1) + 1) per call to `to_base()`.
- O(x / 1000 + 1) per call to `to_roman()` due to the repeated `M` prefix.
- O(n) per call to `from_roman()`, where $n$ is the length of the numeral.
- O(d*e) per call to `convert_base(d, a, b)`, where $d$ is the number of input digits and $e$ is the
  number of output digits.

Space Complexity:
- O(e) auxiliary for `to_base(x, b)`, where $e$ is the number of output digits.
- O(x / 1000 + 1) auxiliary for `to_roman(x)`.
- O(1) auxiliary for `from_roman()`.
- O(d + e) auxiliary for `convert_base(d, a, b)`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

std::vector<int> to_base(unsigned int x, int b = 10) {
  assert(b >= 2);
  std::vector<int> res;
  do {  // do-while so that a value of 0 yields the single digit {0}
    res.push_back(x % b);
    x /= b;
  } while (x != 0);
  return res;
}

std::string to_roman(unsigned int x) {
  static const std::string h[] = {"", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM"};
  static const std::string t[] = {"", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC"};
  static const std::string o[] = {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX"};
  std::string prefix(x / 1000, 'M');
  x %= 1000;
  return prefix + h[x / 100] + t[x / 10 % 10] + o[x % 10];
}

unsigned int from_roman(const std::string &s) {
  static const std::map<char, unsigned int> VALUES = {{'I', 1},   {'V', 5},   {'X', 10},  {'L', 50},
                                                      {'C', 100}, {'D', 500}, {'M', 1000}};
  unsigned int res = 0;
  for (int i = 0; i < static_cast<int>(s.size()); i++) {
    auto it = VALUES.find(s[i]);
    assert(it != VALUES.end());
    auto next = i + 1 < static_cast<int>(s.size()) ? VALUES.find(s[i + 1]) : VALUES.end();
    // A smaller symbol before a larger one is subtracted, as the IV in XIV.
    res += next != VALUES.end() && it->second < next->second ? -it->second : it->second;
  }
  return res;
}

std::vector<int> convert_base(const std::vector<int> &d, int a, int b) {
  assert(a >= 2 && b >= 2);
  assert(std::all_of(d.begin(), d.end(), [a](int x) { return 0 <= x && x < a; }));
  std::vector<int> cur = d, res;
  auto trim = [](std::vector<int> &v) {
    while (v.size() > 1 && v.back() == 0) {
      v.pop_back();
    }
  };
  trim(cur);
  if (cur.empty() || (cur.size() == 1 && cur[0] == 0)) {
    return {0};
  }
  while (!(cur.size() == 1 && cur[0] == 0)) {
    std::vector<int> q(cur.size());
    uint64_t rem = 0;
    for (int i = static_cast<int>(cur.size()) - 1; i >= 0; i--) {
      uint64_t x = rem * static_cast<uint64_t>(a) + static_cast<uint64_t>(cur[i]);
      q[i] = static_cast<int>(x / static_cast<uint64_t>(b));
      rem = x % static_cast<uint64_t>(b);
    }
    res.push_back(static_cast<int>(rem));
    trim(q);
    cur = std::move(q);
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(to_roman(1234) == "MCCXXXIV");
  assert(to_roman(5678) == "MMMMMDCLXXVIII");
  assert(from_roman("MCCXXXIV") == 1234);
  assert(from_roman("IX") == 9 && from_roman("MMXXVI") == 2026);

  vector<int> digits{6, 5, 4, 3, 2, 1};
  assert(convert_base(to_base(123456, 20), 20, 10) == digits);
  assert(convert_base(vector<int>{0, 0, 0}, 10, 2) == vector<int>{0});

  vector<int> big_decimal(30, 9);  // 10^30 - 1, larger than uint64_t.
  assert(convert_base(convert_base(big_decimal, 10, 2), 2, 10) == big_decimal);
  return 0;
}
