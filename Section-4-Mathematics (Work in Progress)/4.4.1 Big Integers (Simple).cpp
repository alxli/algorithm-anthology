/*

Perform simple arithmetic operations on arbitrary precision big integers whose
digits are internally represented as an std::string in little-endian order.

- bigint(n) constructs a big integer from a long long (default = 0).
- bigint(s) constructs a big integer from a string s, which must strictly
  consist of a sequence of numeric digits, optionally preceded by a minus sign.
- str() returns the string representation of the big integer.
- comp(a, b) returns -1, 0, or 1 depending on whether the big integers a and b
  compare less, equal, or greater, respectively.
- add(a, b) returns the sum of big integers a and b.
- sub(a, b) returns the difference of big integers a and b.
- mul(a, b) returns the product of big integers a and b.
- div(a, b) returns the quotient of big integers a and b.

Time Complexity:
- O(n) per call to the constructor, str(), comp(), add(), and sub(), where n is
  total number of digits in the argument(s) and result for each operation.
- O(n*m) per call to mul(a, b) and div(a, b) where n is the number of digits in
  a and m is the number of digits in b.

Space Complexity:
- O(n) for storage of the big integer, where n is the number of the digits.
- O(n) auxiliary heap space for str(), add(), sub(), mul(), and div(), where n
  the total number of digits in the argument(s) and result for each operation.

*/

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

class bigint {
  std::string digits;
  int sign;

  void normalize() {
    size_t pos = digits.find_last_not_of('0');
    if (pos != std::string::npos) {
      digits.erase(pos + 1);
    }
    if (digits.empty()) {
      digits = "0";
    }
    if (digits.size() == 1 && digits[0] == '0') {
      sign = 1;
      return;
    }
  }

  static int comp(const std::string &a, const std::string &b,
                  int asign, int bsign) {
    if (asign != bsign) {
      return asign < bsign ? -1 : 1;
    }
    if (a.size() != b.size()) {
      return a.size() < b.size() ? -asign : asign;
    }
    for (int i = (int)a.size() - 1; i >= 0; i--) {
      if (a[i] != b[i]) {
        return a[i] < b[i] ? -asign : asign;
      }
    }
    return 0;
  }

  static bigint add(const std::string &a, const std::string &b,
                    int asign, int bsign) {
    if (asign != bsign) {
      return (asign == 1) ? sub(a, b, asign, 1) : sub(b, a, bsign, 1);
    }
    bigint res;
    res.sign = asign;
    res.digits.resize(std::max(a.size(), b.size()) + 1, '0');
    for (int i = 0, carry = 0; i < (int)res.digits.size(); i++) {
      int d = carry;
      if (i < (int)a.size()) {
        d += a[i] - '0';
      }
      if (i < (int)b.size()) {
        d += b[i] - '0';
      }
      res.digits[i] = '0' + (d % 10);
      carry = d/10;
    }
    res.normalize();
    return res;
  }

  static bigint sub(const std::string &a, const std::string &b,
                    int asign, int bsign) {
    if (asign == -1 || bsign == -1) {
      return add(a, b, asign, -bsign);
    }
    bigint res;
    if (comp(a, b, asign, bsign) < 0) {
      res = sub(b, a, bsign, asign);
      res.sign = -1;
      return res;
    }
    res.digits.assign(a.size(), '0');
    for (int i = 0, borrow = 0; i < (int)res.digits.size(); i++) {
      int d = (i < (int)b.size() ? a[i] - b[i] : a[i] - '0') - borrow;
      if (a[i] > '0') {
        borrow = 0;
      }
      if (d < 0) {
        d += 10;
        borrow = 1;
      }
      res.digits[i] = '0' + (d % 10);
    }
    res.normalize();
    return res;
  }

 public:
  bigint(long long n = 0) {
    sign = (n < 0) ? -1 : 1;
    if (n == 0) {
      digits = "0";
      return;
    }
    for (n = (n > 0) ? n : -n; n > 0; n /= 10) {
      digits += '0' + (n % 10);
    }
    normalize();
  }

  bigint(const std::string &s) {
    if (s.empty() || (s[0] == '-' && s.size() == 1)) {
      throw std::runtime_error("Invalid string format to construct bigint.");
    }
    digits.assign(s.rbegin(), s.rend());
    if (s[0] == '-') {
      sign = -1;
      digits.erase(digits.size() - 1);
    } else {
      sign = 1;
    }
    if (digits.find_first_not_of("0123456789") != std::string::npos) {
      throw std::runtime_error("Invalid string format to construct bigint.");
    }
    normalize();
  }

  std::string to_string() const {
    return (sign < 0 ? "-" : "") + std::string(digits.rbegin(), digits.rend());
  }

  friend int comp(const bigint &a, const bigint &b) {
    return comp(a.digits, b.digits, a.sign, b.sign);
  }

  friend bigint add(const bigint &a, const bigint &b) {
    return add(a.digits, b.digits, a.sign, b.sign);
  }

  friend bigint sub(const bigint &a, const bigint &b) {
    return sub(a.digits, b.digits, a.sign, b.sign);
  }

  friend bigint mul(const bigint &a, const bigint &b) {
    bigint res, row(a);
    for (int i = 0; i < (int)b.digits.size(); i++) {
      for (int j = 0; j < (b.digits[i] - '0'); j++) {
        res = add(res.digits, row.digits, res.sign, row.sign);
      }
      if (row.digits.size() > 1 || row.digits[0] != '0') {
        row.digits.insert(0, "0");
      }
    }
    res.sign = a.sign*b.sign;
    res.normalize();
    return res;
  }

  friend bigint div(const bigint &a, const bigint &b) {
    bigint res, row;
    res.digits.assign(a.digits.size(), '0');
    for (int i = (int)a.digits.size() - 1; i >= 0; i--) {
      row.digits.insert(row.digits.begin(), a.digits[i]);
      while (comp(row.digits, b.digits, row.sign, 1) > 0) {
        res.digits[i]++;
        row = sub(row.digits, b.digits, row.sign, 1);
      }
    }
    res.sign = a.sign*b.sign;
    res.normalize();
    return res;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  bigint a("-9899819294989142124"), b("12398124981294214");
  assert(add(a, b).to_string() == "-9887421170007847910");
  assert(sub(a, b).to_string() == "-9912217419970436338");
  assert(mul(a, b).to_string() == "-122739196911503356525379735104870536");
  assert(div(a, b).to_string() == "-798");
  assert(comp(a, b) == -1 && comp(a, a) == 0 && comp(b, a) == 1);
  return 0;
}
