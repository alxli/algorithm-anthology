/*

Evaluate an expression with custom operand types, prefix unary operators, binary operators, and
precedences. Evaluation is performed using the shunting yard algorithm, which maintains operator and
value stacks while respecting precedence and parentheses. Multiplication by juxtaposition is not
supported.

An arbitrary operand type is supported, with its string representation defined by a user-specified
`is_operand()` and `eval_operand()` functions. For maximum reliability, the string representation of
operands should not use characters shared by any operator. For instance, the best practice instead
of accepting $-1$ as a valid operand (since the `-` sign may conflict with the identical binary
operator), is to specify non-negative numbers as operands alongside the unary operator `-`.

Operators may be non-empty strings of any length, but should not contain any parentheses or shared
characters with the string representations of operands. Ideally, operators should not be prefixes or
suffixes of one another, else the tokenization process may be ambiguous. For example, if `++` and
`+` are both operators, then `++` may be split into either `["+", "+"]` or `["++"]` depending on the
lexicographical ordering of conflicting operators.

- `ShuntingYardParser(unary_op, binary_op)` initializes a parser with operators specified by hash
  tables `unary_op` (of operator to function pointer) and `binary_op` (of operator to pair of
  function pointer and operator precedence). Operator precedences should be numbered upwards
  starting at 0 (lowest precedence, evaluated last).
- `split(s)` returns a vector of tokens for the expression `s`, split on the given operators during
  construction. Each parenthesis, operator, and operand satisfying `is_operand()` will be split into
  a separate token. The algorithm is naive, matching operators lazily in the case of overlapping
  operators as mentioned above. Under these circumstances, the parse may not always succeed.
- `eval(lo, hi)` returns the evaluation of a range `[lo, hi)` of already split-up expression tokens,
  where `lo` and `hi` must be random-access iterators.
- `eval(s)` returns the evaluation of expression `s`, after first calling `split(s)` to obtain the
  tokens.

Time Complexity:
- O(m) per call to the constructor, where $m$ is the total number of operators.
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
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
using std::string;

// Define the custom operand type and representation below.
using Operand = double;
using UnaryOp = Operand (*)(Operand a);
using BinaryOp = Operand (*)(Operand a, Operand b);

struct BinaryRule {
  BinaryOp op;
  int precedence;
};

class ShuntingYardParser {
  using UnaryOpMap = std::unordered_map<string, UnaryOp>;
  using BinaryOpMap = std::unordered_map<string, BinaryRule>;
  UnaryOpMap unary_ops;
  BinaryOpMap binary_ops;
  std::set<string> ops;

  static Operand eval_operand(const string &s) {
    Operand res;
    std::stringstream ss(s);
    ss >> res;
    return res;
  }

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

  static string strip(string s) {
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
  }

 public:
  ShuntingYardParser(const UnaryOpMap &unary_ops, const BinaryOpMap &binary_ops)
      : unary_ops(unary_ops), binary_ops(binary_ops) {
    for (const auto &[op, fn] : unary_ops) {
      ops.insert(op);
    }
    for (const auto &[op, fn_prec] : binary_ops) {
      ops.insert(op);
    }
  }

  std::vector<string> split(const string &s) {
    std::vector<string> res;
    for (int i = 0; i < static_cast<int>(s.size()); i++) {
      if (s[i] == ' ') {
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
          for (const auto &op : ops) {
            if (s.substr(j, op.size()) == op) {
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

  template<class StrIt>
  Operand eval(StrIt lo, StrIt hi) {
    std::stack<Operand> vals;
    std::stack<std::pair<string, bool>> ops;
    ops.emplace("(", false);
    StrIt prev = hi;
    do {
      string curr = (lo == hi) ? ")" : *lo;
      if (is_operand(curr)) {
        vals.push(eval_operand(curr));
      } else if (curr == "(") {
        ops.emplace(curr, false);
      } else if (
          unary_ops.find(curr) != unary_ops.end() &&
          (prev == hi || *prev == "(" || binary_ops.find(*prev) != binary_ops.end())
      ) {
        ops.emplace(curr, true);
      } else {
        for (;;) {
          auto [op, is_unary] = ops.top();
          auto it1 = binary_ops.find(op);
          auto it2 = binary_ops.find(curr);
          if (!is_unary && (it1 == binary_ops.end() ? -1 : it1->second.precedence) <
                               (it2 == binary_ops.end() ? -1 : it2->second.precedence)) {
            break;
          }
          ops.pop();
          if (op == "(") {
            break;
          }
          Operand b = vals.top();
          vals.pop();
          if (is_unary) {
            if (auto it = unary_ops.find(op); it != unary_ops.end()) {
              vals.push((it->second)(b));
            } else {
              throw std::runtime_error("Failed to eval unary op: " + op);
            }
          } else {
            Operand a = vals.top();
            vals.pop();
            if (it1 == binary_ops.end()) {
              throw std::runtime_error("Failed to eval binary op: " + op);
            }
            vals.push((it1->second.op)(a, b));
          }
        }
        if (curr != ")") {
          ops.emplace(*lo, false);
        }
      }
      prev = lo;
    } while (lo++ != hi);
    return vals.top();
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

int main() {
  unordered_map<string, UnaryOp> unary_ops;
  unary_ops["+"] = [](double x) { return +x; };
  unary_ops["-"] = [](double x) { return -x; };

  unordered_map<string, BinaryRule> binary_ops;
  binary_ops["+"] = {[](double a, double b) { return a + b; }, 0};
  binary_ops["-"] = {[](double a, double b) { return a - b; }, 0};
  binary_ops["*"] = {[](double a, double b) { return a * b; }, 1};
  binary_ops["/"] = {[](double a, double b) { return a / b; }, 1};
  binary_ops["^"] = {std::pow, 2};

  ShuntingYardParser p(unary_ops, binary_ops);
  assert(EQ(p.eval("-+-((--(-+1)))"), -1));
  assert(EQ(p.eval("5*(3+3)-2-2"), 26));
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
