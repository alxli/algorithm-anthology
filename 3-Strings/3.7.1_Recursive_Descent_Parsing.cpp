/*

Evaluate integer arithmetic expressions containing parentheses, unary plus and minus, multiplication
and division, and addition and subtraction. The parser represents these as three precedence levels.
At the tightest level, it consumes a chain of unary signs followed by either a decimal literal or a
parenthesized expression.

At each binary-operator level, the parser first evaluates one operand at the next tighter level,
then repeatedly consumes operators of the current level and their right operands. These loops make
binary operators left-associative, while recursively restarting at the loosest level inside
parentheses enforces grouping.

Literals must be nonnegative decimal integers, although unary signs may precede them. The input must
be valid and contain no whitespace. Division truncates toward zero, and every divisor must be
nonzero.

- `eval(s)` returns an evaluation of the arithmetic expression `s`.

Overflow warning: All intermediate results must fit in `int`.

Time Complexity:
- O(n) per call, where $n$ is the length of `s`.

Space Complexity:
- O(n) auxiliary stack space, where $n$ is the length of `s`.

*/

#include <string>

int eval(const std::string &s) {
  std::string expr = s + '\0';
  auto it = expr.begin();
  auto rec = [&](auto &&rec, int prec) -> int {
    if (prec == 0) {
      int sign = 1, res = 0;
      for (; *it == '-' || *it == '+'; it++) {
        if (*it == '-') {
          sign *= -1;
        }
      }
      if (*it == '(') {
        it++;
        res = rec(rec, 2);
        it++;
      } else {
        while (*it >= '0' && *it <= '9') {
          res = 10 * res + (*(it++) - '0');
        }
      }
      return sign * res;
    }
    int num = rec(rec, prec - 1);
    while (!((prec == 2 && *it != '+' && *it != '-') || (prec == 1 && *it != '*' && *it != '/'))) {
      switch (*(it++)) {
        case '+':
          num += rec(rec, prec - 1);
          break;
        case '-':
          num -= rec(rec, prec - 1);
          break;
        case '*':
          num *= rec(rec, prec - 1);
          break;
        case '/':
          num /= rec(rec, prec - 1);
          break;
      }
    }
    return num;
  };
  return rec(rec, 2);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(eval("+1") == 1);
  assert(eval("1+-2") == -1);
  assert(eval("1++1") == 2);
  assert(eval("1+2*3*4+3*(2+2)-100") == -63);
  return 0;
}
