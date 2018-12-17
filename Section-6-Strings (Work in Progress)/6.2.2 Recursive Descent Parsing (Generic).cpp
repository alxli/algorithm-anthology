/*

Evaluate an expression using a generalized parser class for custom-defined
operand types, prefix unary operators, binary operators, and precedences.
Typical parentheses behavior is supported, but multiplication by juxtaposition
is not. Evaluation is performed using the recursive descent algorithm.

An arbitrary operand type is supported, with its string representation defined
by a user-specified is_operand() and eval_operand() functions. For maximum
reliability, the string representation of operands should not use characters
shared by any operator. For instance, the best practice instead of accepting
"-1" as a valid operand (since the "-" sign may conflict with the identical
binary operator), is to specify non-negative number as operands alongside the
unary operator "-".

Operators may be non-empty strings of any length, but should not contain any
parentheses or shared characters with the string representations of operands.
Ideally, operators should not be prefixes or suffices of one another, else the
tokenization process may be ambiguous. For example, if ++ and + are both
operators, then ++ may be split into either ["+", "+"] or ["++"] depending on
the lexicographical ordering of conflicting operators.

- parser(unary_op, binary_op) initializes a parser with operators specified by
  maps unary_op (of operator to function pointer) and binary_op (of operator to
  pair of function pointer and operator precedence). Operator precedences should
  be numbered upwards starting at 0 (lowest precedence, evaluated last).
- split(s) returns a vector of tokens for the expression s, split on the given
  operators during construction. Each parenthesis, operator, and operand
  satisfying is_operand() will be split into a separate token. The algorithm is
  naive, matching operators lazily in the case of overlapping operators as
  mentioned above. Under these circumstances, the parse may not always succeed.
- eval(lo, hi) returns the evaluation of a range [lo, hi) of already split-up
  expression tokens, where lo and hi must be RandomAccessIterators.
- eval(s) returns the evaluation of expression s, after first calling split(s)
  to obtain the tokens.

Time Complexity:
- O(m) per call to the constructor, where m is the total number of operators.
- O(nmk) per call to split(s), where n is the length of s, m is the total number
  of operators defined for the parser instance, and k is the maximum length for
  any operator representation.
- O(n log m) per call to eval(lo, hi), where n is the distance between lo and hi
  and m is the total number of operators defined for the parser instance. In
  C++11 and later, std::unordered_map may be used in place of std::map for
  storing the unary_ops and binary_ops, which will eliminate the log m factor
  for a time complexity of O(n) per call.
- O(nmk + n log m) per call to eval(s), where n is the distance between lo and
  hi, and m and k are as defined previous.

Space Complexity:
- O(mk) for storage of the m operators, of maximum length k.
- O(n) auxiliary stack space for split(s), eval(lo, hi), and eval(s), where n is
  the length of the argument.

*/

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
using std::string;

// Define the custom operand type and representation below.
typedef double Operand;
typedef Operand (*UnaryOp)(Operand a);
typedef Operand (*BinaryOp)(Operand a, Operand b);

bool is_operand(const string &s) {
  int npoints = 0;
  for (int i = 0; i < (int)s.size(); i++) {
    if (s[i] == '.') {
      if (++npoints > 1) {
        return false;
      }
    } else if (!isdigit(s[i])) {
      return false;
    }
  }
  return !s.empty();
}

Operand eval_operand(const string &s) {
  Operand res;
  std::stringstream ss(s);
  ss >> res;
  return res;
}

class parser {
  typedef std::map<string, UnaryOp> unary_op_map;
  typedef std::map<string, std::pair<BinaryOp, int> > binary_op_map;
  unary_op_map unary_ops;
  binary_op_map binary_ops;
  std::set<string> ops;
  int max_precedence;

  template<class StrIt>
  Operand eval_unary(StrIt &lo, StrIt hi) {
    if (is_operand(*lo)) {
      return eval_operand(*(lo++));
    }
    unary_op_map::iterator it = unary_ops.find(*lo);
    if (it != unary_ops.end()) {
      return (it->second)(eval_unary(++lo, hi));
    }
    if (*lo != "(") {
      throw std::runtime_error("Expected \"(\" during eval.");
    }
    Operand res = eval_binary(++lo, hi, 0);
    if (*lo != ")") {
      throw std::runtime_error("Expected \")\" during eval.");
    }
    ++lo;
    return res;
  }

  template<class StrIt>
  Operand eval_binary(StrIt &lo, StrIt hi, Operand precedence) {
    if (precedence > max_precedence) {
      return eval_unary(lo, hi);
    }
    Operand v = eval_binary(lo, hi, precedence + 1);
    while (lo != hi) {
      binary_op_map::iterator it;
      it = binary_ops.find(*lo);
      if (it == binary_ops.end() || it->second.second != precedence) {
        return v;
      }
      v = (it->second.first)(v, eval_binary(++lo, hi, precedence + 1));
    }
    return v;
  }

  static string strip(string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }

 public:
  parser(const unary_op_map &unary_ops, const binary_op_map &binary_ops)
      : unary_ops(unary_ops), binary_ops(binary_ops) {
    for (unary_op_map::const_iterator it = unary_ops.begin();
         it != unary_ops.end(); ++it) {
      ops.insert(it->first);
    }
    max_precedence = 0;
    for (binary_op_map::const_iterator it = binary_ops.begin();
         it != binary_ops.end(); ++it) {
      ops.insert(it->first);
      max_precedence = std::max(max_precedence, it->second.second);
    }
  }

  std::vector<string> split(const string &s) {
    std::vector<string> res;
    for (int i = 0; i < (int)s.size(); i++) {
      if (s[i] == ' ') {
        continue;
      }
      int next_paren = s.size();
      for (int j = i; j < (int)s.size(); j++) {
        if (s[j] == '(' || s[j] == ')') {
          next_paren = j;
          break;
        }
      }
      while (i < next_paren) {
        int found = next_paren;
        string found_op;
        for (int j = i; j < next_paren && found == next_paren; j++) {
          for (std::set<string>::iterator it = ops.begin();
               it != ops.end(); ++it) {
            if (s.substr(j, it->size()) == *it) {
              found = j;
              found_op = *it;
              break;
            }
          }
        }
        string term = strip(s.substr(i, found - i));
        if (!term.empty()) {
          res.push_back(term);
          if (!is_operand(term)) {
            throw std::runtime_error("Failed to split term: \"" + term + "\".");
          }
        }
        if (found < next_paren) {
          res.push_back(found_op);
          i = found + found_op.size();
        } else {
          i = next_paren;
        }
      }
      if (next_paren < s.size()) {
        res.push_back(string(1, s[next_paren]));
      }
    }
    return res;
  }

  template<class StrIt>
  Operand eval(StrIt lo, StrIt hi) {
    Operand res = eval_binary(lo, hi, 0);
    if (lo != hi) {
      throw std::runtime_error("Eval failed at token " + *lo + ".");
    }
    return res;
  }

  Operand eval(const string &s) {
    std::vector<string> tokens = split(s);
    return eval(tokens.begin(), tokens.end());
  }
};

/*** Example Usage ***/

#include <cassert>
#include <cmath>
using namespace std;

#define EQ(a, b) (fabs((a) - (b)) < 1e-7)

double pos(double a) { return +a; }
double neg(double a) { return -a; }
double add(double a, double b) { return a + b; }
double sub(double a, double b) { return a - b; }
double mul(double a, double b) { return a * b; }
double div(double a, double b) { return a / b; }

int main() {
  map<string, UnaryOp> unary_ops;
  unary_ops["+"] = pos;
  unary_ops["-"] = neg;

  map<string, pair<BinaryOp, int> > binary_ops;
  binary_ops["+"] = make_pair((BinaryOp)add, 0);
  binary_ops["-"] = make_pair((BinaryOp)sub, 0);
  binary_ops["*"] = make_pair((BinaryOp)mul, 1);
  binary_ops["/"] = make_pair((BinaryOp)div, 1);
  binary_ops["^"] = make_pair((BinaryOp)pow, 2);

  parser p(unary_ops, binary_ops);
  assert(EQ(p.eval("-+-((--(-+1)))"), -1));
  assert(EQ(p.eval("5*(3+3)-2-2"), 26));
  assert(EQ(p.eval("1+2*3*4+3*(+2)-100"), -69));
  assert(EQ(p.eval("3*3*3*3*3*3-2*2*2*2*2*2*2*2"), 473));
  assert(EQ(p.eval("3.14 + 3 * (7.7/9.8^32.9  )"), 3.14));
  assert(EQ(p.eval("5*(3+2)/-1*-2+(-2-2-2+3)-3-(-2)+15/2/2/2+(-2)"), 45.875));
  assert(EQ(p.eval("123456789./3/3/3*2*2*2+456/6-23/3"), 36579857.6666666667));
  assert(EQ(p.eval("10/3+10/4+10/5+10/6+10/7+10/8+10/9+10/10+15*23456"),
            351854.28968253968));
  assert(EQ(p.eval("-(5-(5-(5-(5-(5-2)))))+(3-(3-(3-(3-(3+3)))))*"
                   "(7-(7-(7-(7-(7-7+4*5)))))"), 117));
  return 0;
}
