/*

Perform simple arithmetic operations on arbitrary precision big integers whose digits are internally
represented as an `std::string` in little-endian order.

This version is intentionally small and decimal-oriented. Addition and subtraction are the usual
schoolbook digit scans with carry or borrow. Multiplication keeps a shifted copy of the left operand
for each digit of the right operand, adds that row once per digit value, then shifts by one decimal
place. Division is long division: it scans the dividend from most significant digit to least,
maintains a running remainder, and repeatedly subtracts the divisor to discover each quotient digit.

- `BigInt(n)` constructs a big integer from a long long (default: 0).
- `BigInt(s)` constructs a big integer from a string `s`, which must strictly consist of a sequence
  of numeric digits, optionally preceded by a minus sign.
- `to_string()` returns the string representation of the big integer.
- `comp(a, b)` returns $-1$, $0$, or $1$ depending on whether the big integers `a` and `b` compare
  less, equal, or greater, respectively.
- `add(a, b)` returns the sum of big integers `a` and `b`.
- `sub(a, b)` returns the difference of big integers `a` and `b`.
- `mul(a, b)` returns the product of big integers `a` and `b`.
- `div(a, b)` returns the quotient of big integers `a` and `b`.

Time Complexity:
- O(n) per call to the constructor, `to_string()`, `comp()`, `add()`, and `sub()`, where $n$ is
  the total number of digits in the arguments and result for each operation.
- O(n*m) per call to `mul(a, b)` and `div(a, b)` where $n$ is the number of digits in `a` and $m$ is
  the number of digits in `b`.

Space Complexity:
- O(n) for storage of the big integer, where $n$ is the number of digits.
- O(n) auxiliary heap space for `to_string()`, `add()`, `sub()`, `mul()`, and `div()`, where $n$ is
  the total number of digits in the arguments and result for each operation.

*/

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

class BigInt {
  std::string digits;
  int sign;

  void normalize() {
    size_t pos = digits.find_last_not_of('0');
    if (pos != std::string::npos) {
      digits.erase(pos + 1);
    } else {
      digits = "0";  // An all-zero (or empty) magnitude collapses to a single "0".
    }
    if (digits.size() == 1 && digits[0] == '0') {
      sign = 1;
    }
  }

  static int comp(const std::string &a, const std::string &b, int asign, int bsign) {
    if (asign != bsign) {
      return asign < bsign ? -1 : 1;
    }
    if (a.size() != b.size()) {
      return a.size() < b.size() ? -asign : asign;
    }
    for (int i = static_cast<int>(a.size()) - 1; i >= 0; i--) {
      if (a[i] != b[i]) {
        return a[i] < b[i] ? -asign : asign;
      }
    }
    return 0;
  }

  static BigInt add(const std::string &a, const std::string &b, int asign, int bsign) {
    if (asign != bsign) {
      return (asign == 1) ? sub(a, b, asign, 1) : sub(b, a, bsign, 1);
    }
    BigInt res;
    res.sign = asign;
    res.digits.resize(std::max(a.size(), b.size()) + 1, '0');
    for (int i = 0, carry = 0; i < static_cast<int>(res.digits.size()); i++) {
      int d = carry;
      if (i < static_cast<int>(a.size())) {
        d += a[i] - '0';
      }
      if (i < static_cast<int>(b.size())) {
        d += b[i] - '0';
      }
      res.digits[i] = '0' + (d % 10);
      carry = d / 10;
    }
    res.normalize();
    return res;
  }

  static BigInt sub(const std::string &a, const std::string &b, int asign, int bsign) {
    if (asign == -1 || bsign == -1) {
      return add(a, b, asign, -bsign);
    }
    BigInt res;
    if (comp(a, b, asign, bsign) < 0) {
      res = sub(b, a, bsign, asign);
      res.sign = -1;
      return res;
    }
    res.digits.assign(a.size(), '0');
    for (int i = 0, borrow = 0; i < static_cast<int>(res.digits.size()); i++) {
      int d = (i < static_cast<int>(b.size()) ? a[i] - b[i] : a[i] - '0') - borrow;
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
  BigInt(long long n = 0) {
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

  BigInt(const std::string &s) {
    if (s.empty() || (s[0] == '-' && s.size() == 1)) {
      throw std::runtime_error("Invalid string format to construct BigInt.");
    }
    digits.assign(s.rbegin(), s.rend());
    if (s[0] == '-') {
      sign = -1;
      digits.erase(digits.size() - 1);
    } else {
      sign = 1;
    }
    if (digits.find_first_not_of("0123456789") != std::string::npos) {
      throw std::runtime_error("Invalid string format to construct BigInt.");
    }
    normalize();
  }

  std::string to_string() const {
    return (sign < 0 ? "-" : "") + std::string(digits.rbegin(), digits.rend());
  }

  friend int comp(const BigInt &a, const BigInt &b) {
    return comp(a.digits, b.digits, a.sign, b.sign);
  }

  friend BigInt add(const BigInt &a, const BigInt &b) {
    return add(a.digits, b.digits, a.sign, b.sign);
  }

  friend BigInt sub(const BigInt &a, const BigInt &b) {
    return sub(a.digits, b.digits, a.sign, b.sign);
  }

  friend BigInt mul(const BigInt &a, const BigInt &b) {
    BigInt res, row(a);
    for (char dc : b.digits) {
      for (int j = 0; j < (dc - '0'); j++) {
        res = add(res.digits, row.digits, res.sign, row.sign);
      }
      if (row.digits.size() > 1 || row.digits[0] != '0') {
        row.digits.insert(0, "0");
      }
    }
    res.sign = a.sign * b.sign;
    res.normalize();
    return res;
  }

  friend BigInt div(const BigInt &a, const BigInt &b) {
    if (b.digits.size() == 1 && b.digits[0] == '0') {
      throw std::runtime_error("Division by zero in BigInt.");
    }
    BigInt res, row;
    res.digits.assign(a.digits.size(), '0');
    row.digits.clear();  // Running remainder; kept normalized and nonnegative below.
    for (int i = static_cast<int>(a.digits.size()) - 1; i >= 0; i--) {
      row.digits.insert(row.digits.begin(), a.digits[i]);
      row.normalize();  // Strip the spurious high-order zero left by the previous remainder.
      // Subtract while the remainder is >= b. A strict ">" undercounts the quotient digit when the
      // remainder equals b, and leaves the remainder too large, overflowing the next digit past 9.
      while (comp(row.digits, b.digits, 1, 1) >= 0) {
        res.digits[i]++;
        row = sub(row.digits, b.digits, 1, 1);
      }
    }
    res.sign = a.sign * b.sign;
    res.normalize();
    return res;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  BigInt a("-9899819294989142124"), b("12398124981294214");
  assert(add(a, b).to_string() == "-9887421170007847910");
  assert(sub(a, b).to_string() == "-9912217419970436338");
  assert(mul(a, b).to_string() == "-122739196911503356525379735104870536");
  assert(div(a, b).to_string() == "-798");
  assert(comp(a, b) == -1 && comp(a, a) == 0 && comp(b, a) == 1);
  return 0;
}
