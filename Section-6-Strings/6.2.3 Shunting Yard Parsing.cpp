/*

Evaluate a mathematica expression in accordance to the order
of operations (parentheses, exponents, multiplication, division,
addition, subtraction). This also handles unary operators like '-'.
We use strings for operators so we can even define things like "sqrt"
and "mod" as unary operators by changing prec() and split_expr()
accordingly.

Time Complexity: O(n) on the total number of operators and operands.

*/

#include <cstdlib>   /* strtol() */
#include <stack>
#include <stdexcept> /* std::runtime_error */
#include <string>
#include <vector>

// Classify the precedences of operators here.
inline int prec(const std::string & op, bool unary) {
  if (unary) {
    if (op == "+" || op == "-") return 3;
    return 0; // not a unary operator
  }
  if (op == "*" || op == "/") return 2;
  if (op == "+" || op == "-") return 1;
  return 0;  // not a binary operator
}

inline int calc1(const std::string & op, int val) {
  if (op == "+") return +val;
  if (op == "-") return -val;
  throw std::runtime_error("Invalid unary operator: " + op);
}

inline int calc2(const std::string & op, int L, int R) {
  if (op == "+") return L + R;
  if (op == "-") return L - R;
  if (op == "*") return L * R;
  if (op == "/") return L / R;
  throw std::runtime_error("Invalid binary operator: " + op);
}

inline bool is_operand(const std::string & s) {
  return s != "(" && s != ")" && !prec(s, 0) && !prec(s, 1);
}

int eval(std::vector<std::string> E) { // E stores the tokens
  E.insert(E.begin(), "(");
  E.push_back(")");
  std::stack<std::pair<std::string, bool> > ops;
  std::stack<int> vals;
  for (int i = 0; i < (int)E.size(); i++) {
    if (is_operand(E[i])) {
      vals.push(strtol(E[i].c_str(), 0, 10)); // convert to int
      continue;
    }
    if (E[i] == "(") {
      ops.push(std::make_pair("(", 0));
      continue;
    }
    if (prec(E[i], 1) && (i == 0 || E[i - 1] == "(" || prec(E[i - 1], 0))) {
      ops.push(std::make_pair(E[i], 1));
      continue;
    }
    while(prec(ops.top().first, ops.top().second) >= prec(E[i], 0)) {
      std::string op = ops.top().first;
      bool is_unary = ops.top().second;
      ops.pop();
      if (op == "(") break;
      int y = vals.top(); vals.pop();
      if (is_unary) {
        vals.push(calc1(op, y));
      } else {
        int x = vals.top(); vals.pop();
        vals.push(calc2(op, x, y));
      }
    }
    if (E[i] != ")") ops.push(std::make_pair(E[i], 0));
  }
  return vals.top();
}

/*

Split a string expression to tokens, ignoring whitespace delimiters.
A vector of tokens is a more flexible format since you can decide to
parse the expression however you wish just by modifying this function.
e.g. "1+(51 * -100)" converts to {"1","+","(","51","*","-","100",")"}

*/

std::vector<std::string> split_expr(const std::string &s,
                const std::string &delim = " \n\t\v\f\r") {
  std::vector<std::string> ret;
  std::string acc = "";
  for (int i = 0; i < (int)s.size(); i++)
    if (s[i] >= '0' && s[i] <= '9') {
      acc += s[i];
    } else {
      if (i > 0 && s[i - 1] >= '0' && s[i - 1] <= '9')
        ret.push_back(acc);
      acc = "";
      if (delim.find(s[i]) != std::string::npos) continue;
      ret.push_back(std::string("") + s[i]);
    }
  if (s[s.size() - 1] >= '0' && s[s.size() - 1] <= '9')
    ret.push_back(acc);
  return ret;
}

int eval(const std::string & s) {
  return eval(split_expr(s));
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  cout << eval("1+2*3*4+3*(2+2)-100") << endl;
  return 0;
}
