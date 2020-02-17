/*

Evaluate an expression in accordance to the order of operations (parentheses,
unary plus and minus signs, multiplication/division, addition/subtraction). The
following is a minimalistic recursive descent implementation using iterators.

- eval(s) returns an evaluation of the arithmetic expression s.

Time Complexity:
- O(n) per call to eval(s), where n is the length of s.

Space Complexity:
- O(n) auxiliary stack space for eval(s), where n is the length of s.

*/

#include <string>

template<class It>
int eval(It &it, int prec) {
  if (prec == 0) {
    int sign = 1, ret = 0;
    for (; *it == '-'; it++) {
      sign *= -1;
    }
    if (*it == '(') {
      ret = eval(++it, 2);
      it++;
    } else while (*it >= '0' && *it <= '9') {
      ret = 10*ret + (*(it++) - '0');
    }
    return sign*ret;
  }
  int num = eval(it, prec - 1);
  while (!((prec == 2 && *it != '+' && *it != '-') ||
           (prec == 1 && *it != '*' && *it != '/'))) {
    switch (*(it++)) {
      case '+': num += eval(it, prec - 1); break;
      case '-': num -= eval(it, prec - 1); break;
      case '*': num *= eval(it, prec - 1); break;
      case '/': num /= eval(it, prec - 1); break;
    }
  }
  return num;
}

int eval(const std::string &s) {
  std::string::iterator it = std::string(s).begin();
  return eval(it, 2);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(eval("1++1") == 2);
  assert(eval("1+2*3*4+3*(2+2)-100") == -63);
  return 0;
}
