/*

Evaluate an expression with configurable prefix unary and infix binary operators using Pratt
parsing, also called top-down operator-precedence parsing. Unlike recursive descent with one
function per precedence level, a Pratt parser uses numeric binding powers to decide whether the next
operator belongs to the current expression or to its caller.

Parsing begins with an operand, parenthesized expression, or prefix operator. It then repeatedly
consumes binary operators whose precedence is high enough for the current call. A left-associative
operator raises the minimum precedence for its right operand, preventing another equal-precedence
operator from joining that operand; a right-associative operator leaves the threshold unchanged.

Customize the operand type and syntax by changing `Operand` and `eval_operand()`. Operators may
contain multiple non-operand characters, and the longest matching operator is chosen. Operands are
maximal strings of letters, digits, underscores, and periods. Multiplication by juxtaposition and
postfix operators are not supported.

- `eval_pratt(s)` evaluates expression `s` using the global operator tables. It throws
  `runtime_error` for malformed expressions, unknown tokens, or mismatched parentheses.

Time Complexity:
- O(m*k*log m + n*m*k) expected per call, where $n$ is the length of `s`, $m$ is the total number of
  operators, and $k$ is their maximum length.

Space Complexity:
- O(n + m*k) auxiliary space, including the recursive stack.

*/

#include <algorithm>
#include <cctype>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
using std::string;

using Operand = double;

struct UnaryRule {
  std::function<Operand(Operand)> op;
  int precedence;
};

struct BinaryRule {
  std::function<Operand(Operand, Operand)> op;
  int precedence;
  bool right_associative = false;
};

std::unordered_map<string, UnaryRule> unary_ops;
std::unordered_map<string, BinaryRule> binary_ops;

Operand eval_operand(const string &token) {
  return std::stod(token);
}

void require(bool condition, const string &message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

Operand eval_pratt(const string &s) {
  std::vector<string> op_tokens;
  for (const auto &[op, rule] : unary_ops) {
    op_tokens.push_back(op);
  }
  for (const auto &[op, rule] : binary_ops) {
    op_tokens.push_back(op);
  }
  std::sort(op_tokens.begin(), op_tokens.end(), [](const string &a, const string &b) {
    return a.size() != b.size() ? a.size() > b.size() : a < b;
  });
  op_tokens.erase(std::unique(op_tokens.begin(), op_tokens.end()), op_tokens.end());
  int pos = 0, n = static_cast<int>(s.size());
  auto skip_spaces = [&] {
    while (pos < n && std::isspace(static_cast<unsigned char>(s[pos]))) {
      pos++;
    }
  };
  auto match_operator = [&](bool unary) {
    skip_spaces();
    for (const string &op : op_tokens) {
      const bool allowed = unary ? unary_ops.count(op) : binary_ops.count(op);
      if (allowed && s.compare(pos, op.size(), op) == 0) {
        return op;
      }
    }
    return string();
  };
  auto is_operand_char = [](unsigned char c) { return std::isalnum(c) || c == '_' || c == '.'; };
  auto parse = [&](auto &&parse, int min_precedence) -> Operand {
    skip_spaces();
    require(pos != n, "Expression ends before an operand.");
    Operand lhs;
    if (s[pos] == '(') {
      pos++;
      lhs = parse(parse, 0);
      skip_spaces();
      require(pos != n && s[pos] == ')', "Mismatched parentheses.");
      pos++;
    } else if (string op = match_operator(true); !op.empty()) {
      pos += static_cast<int>(op.size());
      const UnaryRule &rule = unary_ops.at(op);
      lhs = rule.op(parse(parse, rule.precedence));
    } else {
      require(
          is_operand_char(static_cast<unsigned char>(s[pos])),
          "Expected operand at position " + std::to_string(pos) + "."
      );
      int start = pos++;
      while (pos < n && is_operand_char(static_cast<unsigned char>(s[pos]))) {
        pos++;
      }
      lhs = eval_operand(s.substr(start, pos - start));
    }
    while (true) {
      string op = match_operator(false);
      if (op.empty()) {
        return lhs;
      }
      const BinaryRule &rule = binary_ops.at(op);
      if (rule.precedence < min_precedence) {
        return lhs;
      }
      pos += static_cast<int>(op.size());
      lhs = rule.op(lhs, parse(parse, rule.precedence + !rule.right_associative));
    }
  };
  Operand value = parse(parse, 0);
  skip_spaces();
  require(pos == n, "Unexpected token at position " + std::to_string(pos) + ".");
  return value;
}

/*** Example Usage ***/

#include <cassert>
#include <cmath>
using namespace std;

bool EQ(double a, double b) {
  return a == b || fabs(a - b) <= 1e-9;
}

int main() {
  unary_ops = {
      {"+", {[](Operand x) { return x; }, 3}},
      {"-", {[](Operand x) { return -x; }, 3}},
  };
  binary_ops = {
      {"+", {[](Operand a, Operand b) { return a + b; }, 1}},
      {"-", {[](Operand a, Operand b) { return a - b; }, 1}},
      {"*", {[](Operand a, Operand b) { return a * b; }, 2}},
      {"/", {[](Operand a, Operand b) { return a / b; }, 2}},
      {"^", {[](Operand a, Operand b) { return pow(a, b); }, 4, true}},
  };
  assert(EQ(eval_pratt("-2^2"), -4));
  assert(EQ(eval_pratt("2^-3"), 0.125));
  assert(EQ(eval_pratt("2^3^2"), 512));
  assert(EQ(eval_pratt("5*(3+2)/-1*-2"), 50));
  return 0;
}
