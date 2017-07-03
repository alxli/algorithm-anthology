/*

Evaluate a mathematical expression in accordance to the order
of operations (parentheses, exponents, multiplication, division,
addition, subtraction). Does not handle unary operators like '-'.

*/

/*** Example Usage ***/

#include <cctype>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>

class parser {
  int pos;
  double tokval;
  std::string s;

  bool is_dig_or_dot(char c) {
    return isdigit(c) || c == '.';
  }

  double to_double(const std::string & s) {
    std::stringstream ss(s);
    double res;
    ss >> res;
    return res;
  }

 public:
  char token;

  parser(const std::string & s) {
    this->s = s;
    pos = 0;
  }

  int next() {
    for (;;) {
      if (pos == (int)s.size())
        return token = -1;
      char c = s[pos++];
      if (std::string("+-*/^()\n").find(c) != std::string::npos)
        return token = c;
      if (isspace(c)) continue;
      if (isdigit(c) || c == '.') {
        std::string operand(1, c);
        while (pos < (int)s.size() && is_dig_or_dot(s[pos]))
          operand += (c = s[pos++]);
        tokval = to_double(operand);
        return token = 'n';
      }
      throw std::runtime_error(std::string("Bad character: ") + c);
    }
  }

  void skip(int ch) {
    if (token != ch)
      throw std::runtime_error(std::string("Bad character: ") + token + std::string(", expected: ") + (char)ch);
    next();
  }

  double number() {
    if (token == 'n') {
      double v = tokval;
      skip('n');
      return v;
    }
    skip('(');
    double v = expression();
    skip(')');
    return v;
  }

  // factor ::= number | number '^' factor
  double factor() {
    double v = number();
    if (token == '^') {
      skip('^');
      v = pow(v, factor());
    }
    return v;
  }

  // term ::= factor | term '*' factor | term '/' factor
  double term() {
    double v = factor();
    for (;;) {
      if (token == '*') {
        skip('*');
        v *= factor();
      } else if (token == '/') {
        skip('/');
        v /= factor();
      } else {
        return v;
      }
    }
  }

  // expression ::= term | expression '+' term | expression '-' term
  double expression() {
    double v = term();
    for (;;) {
      if (token == '+') {
        skip('+');
        v += term();
      } else if (token == '-') {
        skip('-');
        v -= term();
      } else {
        return v;
      }
    }
  }
};

#include <iostream>
using namespace std;

int main() {
  parser p("1+2*3*4+3*(2+2)-100\n");
  p.next();
  while (p.token != -1) {
    if (p.token == '\n') {
      p.skip('\n');
      continue;
    }
    cout << p.expression() << "\n";
  }
  return 0;
}

