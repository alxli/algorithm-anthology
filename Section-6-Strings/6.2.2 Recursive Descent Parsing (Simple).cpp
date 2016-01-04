/*

Evaluate a mathematica expression in accordance to the order
of operations (parentheses, exponents, multiplication, division,
addition, subtraction). This handles unary operators like '-'.

*/

#include <string>

template<class It> int eval(It & it, int prec) {
  if (prec == 0) {
    int sign = 1, ret = 0;
    for (; *it == '-'; it++) sign *= -1;
    if (*it == '(') {
      ret = eval(++it, 2);
      it++;
    } else while (*it >= '0' && *it <= '9') {
      ret = 10 * ret + (*(it++) - '0');
    }
    return sign * ret;
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

/*** Wrapper Function ***/

int eval(const std::string & s) {
  std::string::iterator it = std::string(s).begin();
  return eval(it, 2);
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  cout << eval("1+2*3*4+3*(2+2)-100") << "\n";
  return 0;
}
