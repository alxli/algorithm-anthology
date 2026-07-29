/*

Evaluate an expression with custom operand types, prefix unary operators, binary operators, and
precedences. Evaluation is performed by recursive descent: the parser recursively evaluates tighter
precedence levels before looser ones. Parentheses are supported, but multiplication by juxtaposition
is not.

An arbitrary operand type is supported by changing the `Operand` alias and the static `is_operand()`
and `eval_operand()` helpers. For maximum reliability, the string representation of operands should
not use characters shared by any operator. For instance, the best practice instead of accepting `-1`
as a valid operand (since the `-` sign may conflict with the identical binary operator), is to
specify nonnegative numbers as operands alongside the unary operator `-`.

Operators may be non-empty strings of any length, but should not contain any parentheses or shared
characters with the string representations of operands. When multiple operators match at the same
position, the longest one is chosen, so `<=` remains one token when `<` is also an operator.

- `RecursiveDescentParser(unary_ops, binary_ops)` initializes a parser with operators specified by
  hash tables `unary_ops` (of operator to unary function object) and `binary_ops` (of operator to
  pair of binary function object and operator precedence). Operator precedences should be numbered
  upwards starting at $0$ (lowest precedence, evaluated last).
- `split(s)` returns a vector of tokens for the expression `s`, split on the given operators during
  construction. Each parenthesis, operator, and operand satisfying `is_operand()` will be split into
  a separate token.
- `eval(lo, hi)` returns the evaluation of a range $[`lo`, `hi`)$ of already split-up expression
  tokens, where `lo` and `hi` must be random-access iterators.
- `eval(s)` returns the evaluation of expression `s`, after first calling `split(s)` to obtain the
  tokens.

Time Complexity:
- O(m*k*log m) per call to the constructor, where $m$ is the total number of operators and $k$ is
  their maximum length.
- O(n*m*k) per call to `split(s)`, where $n$ is the length of `s`, $m$ is the total number of
  operators defined for the parser instance, and $k$ is the maximum length for any operator
  representation.
- O(n) expected per call to `eval(lo, hi)`, where $n$ is the distance between `lo` and `hi`.
- O(n*m*k + n) expected per call to `eval(s)`, where $n$ is the length of `s`, and $m$ and $k$ are
  as defined previously.

Space Complexity:
- O(m*k) for storage of the $m$ operators, of maximum length $k$.
- O(n) auxiliary stack space for `split(s)`, `eval(lo, hi)`, and `eval(s)`, where $n$ is the length
  of the argument.

*/

#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
using std::string;

// Define the custom operand type and representation below.
using Operand = double;
using UnaryOp = std::function<Operand(Operand)>;
using BinaryOp = std::function<Operand(Operand, Operand)>;

struct BinaryRule {
  BinaryOp op;
  int precedence;
};

class RecursiveDescentParser {
  using UnaryOpMap = std::unordered_map<string, UnaryOp>;
  using BinaryOpMap = std::unordered_map<string, BinaryRule>;
  UnaryOpMap unary_ops;
  BinaryOpMap binary_ops;
  std::vector<string> op_tokens;
  int max_precedence;

  static bool is_operand(const string &s) {
    int npoints = 0;
    for (char c : s) {
      if (c == '.') {
        if (++npoints > 1) {
          return false;
        }
      } else if (!isdigit(static_cast<unsigned char>(c))) {
        return false;
      }
    }
    return !s.empty();
  }

  static Operand eval_operand(const string &s) {
    Operand res;
    std::stringstream ss(s);
    ss >> res;
    return res;
  }

  template<typename StrIt>
  Operand eval_unary(StrIt &lo, StrIt hi) const {
    if (lo == hi) {
      throw std::runtime_error("Unexpected end of expression.");
    }
    if (is_operand(*lo)) {
      return eval_operand(*(lo++));
    }
    if (auto it = unary_ops.find(*lo); it != unary_ops.end()) {
      return (it->second)(eval_unary(++lo, hi));
    }
    if (*lo != "(") {
      throw std::runtime_error("Expected \"(\" during eval.");
    }
    Operand res = eval_binary(++lo, hi, 0);
    if (lo == hi || *lo != ")") {
      throw std::runtime_error("Expected \")\" during eval.");
    }
    ++lo;
    return res;
  }

  template<typename StrIt>
  Operand eval_binary(StrIt &lo, StrIt hi, int precedence) const {
    if (precedence > max_precedence) {
      return eval_unary(lo, hi);
    }
    Operand v = eval_binary(lo, hi, precedence + 1);
    while (lo != hi) {
      auto it = binary_ops.find(*lo);
      if (it == binary_ops.end() || it->second.precedence != precedence) {
        return v;
      }
      v = (it->second.op)(v, eval_binary(++lo, hi, precedence + 1));
    }
    return v;
  }

  static string strip(string s) {
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
  }

 public:
  RecursiveDescentParser(const UnaryOpMap &unary_ops, const BinaryOpMap &binary_ops)
      : unary_ops(unary_ops), binary_ops(binary_ops) {
    for (const auto &[op, _] : unary_ops) {
      op_tokens.push_back(op);
    }
    max_precedence = 0;
    for (const auto &[op, fn_prec] : binary_ops) {
      op_tokens.push_back(op);
      max_precedence = std::max(max_precedence, fn_prec.precedence);
    }
    std::sort(op_tokens.begin(), op_tokens.end(), [](const string &a, const string &b) {
      return a.size() != b.size() ? a.size() > b.size() : a < b;
    });
    op_tokens.erase(std::unique(op_tokens.begin(), op_tokens.end()), op_tokens.end());
  }

  std::vector<string> split(const string &s) const {
    std::vector<string> res;
    for (int i = 0; i < static_cast<int>(s.size()); i++) {
      if (std::isspace(static_cast<unsigned char>(s[i]))) {
        continue;
      }
      int next_paren = static_cast<int>(s.size());
      for (int j = i; j < static_cast<int>(s.size()); j++) {
        if (s[j] == '(' || s[j] == ')') {
          next_paren = j;
          break;
        }
      }
      while (i < next_paren) {
        int found = next_paren;
        string found_op;
        for (int j = i; j < next_paren && found == next_paren; j++) {
          for (const auto &op : op_tokens) {
            if (s.compare(j, op.size(), op) == 0) {
              found = j;
              found_op = op;
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
          i = found + static_cast<int>(found_op.size());
        } else {
          i = next_paren;
        }
      }
      if (next_paren < static_cast<int>(s.size())) {
        res.emplace_back(1, s[next_paren]);
      }
    }
    return res;
  }

  template<typename StrIt>
  Operand eval(StrIt lo, StrIt hi) const {
    Operand res = eval_binary(lo, hi, 0);
    if (lo != hi) {
      throw std::runtime_error("Eval failed at token " + *lo + ".");
    }
    return res;
  }

  Operand eval(const string &s) const {
    std::vector<string> tokens = split(s);
    return eval(tokens.begin(), tokens.end());
  }
};

/*** Example Usage ***/

#include <cassert>
#include <cmath>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-7;
}

int main() {
  unordered_map<string, UnaryOp> unary_ops;
  unary_ops["+"] = [](double x) { return +x; };
  unary_ops["-"] = [](double x) { return -x; };

  unordered_map<string, BinaryRule> binary_ops;
  binary_ops["+"] = {[](double a, double b) { return a + b; }, 1};
  binary_ops["-"] = {[](double a, double b) { return a - b; }, 1};
  binary_ops["*"] = {[](double a, double b) { return a * b; }, 2};
  binary_ops["/"] = {[](double a, double b) { return a / b; }, 2};
  binary_ops["^"] = {[](double a, double b) { return std::pow(a, b); }, 3};
  binary_ops["<"] = {[](double a, double b) { return a < b; }, 0};
  binary_ops["<="] = {[](double a, double b) { return a <= b; }, 0};

  RecursiveDescentParser p(unary_ops, binary_ops);
  assert(EQ(p.eval("-+-((--(-+1)))"), -1));
  assert(EQ(p.eval("5*(3+3)-2-2"), 26));
  assert(EQ(p.eval("1<=1"), 1));  // Prefer the longest operator token at a shared prefix.
  assert(EQ(p.eval("1+2*3*4+3*(+2)-100"), -69));
  assert(EQ(p.eval("3*3*3*3*3*3-2*2*2*2*2*2*2*2"), 473));
  assert(EQ(p.eval("3.14 + 3 * (7.7/9.8^32.9  )"), 3.14));
  assert(EQ(p.eval("5*(3+2)/-1*-2+(-2-2-2+3)-3-(-2)+15/2/2/2+(-2)"), 45.875));
  assert(EQ(p.eval("123456789./3/3/3*2*2*2+456/6-23/3"), 36579857.6666666667));
  assert(EQ(p.eval("10/3+10/4+10/5+10/6+10/7+10/8+10/9+10/10+15*23456"), 351854.28968253968));
  assert(
      EQ(p.eval(
             "-(5-(5-(5-(5-(5-2)))))+(3-(3-(3-(3-(3+3)))))*"
             "(7-(7-(7-(7-(7-7+4*5)))))"
         ),
         117)
  );
  return 0;
}
