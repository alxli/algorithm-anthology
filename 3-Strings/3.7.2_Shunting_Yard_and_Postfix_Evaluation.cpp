/*

Convert an infix expression to postfix notation, also called reverse Polish notation (RPN), and
evaluate the result. In postfix notation each operator follows its operands, so evaluation needs no
parentheses or precedence rules: operands are pushed onto a value stack, and each operator replaces
its operands with their result.

The shunting yard algorithm produces postfix notation by sending operands directly to the output and
holding operators on a stack. An operator is emitted before a lower-precedence operator, or before a
left-associative operator of equal precedence. Parentheses delimit independent portions of the
operator stack. Whether `+` or `-` is unary follows from context: a prefix operator is expected at
the beginning, after another operator, or after an opening parenthesis.

Customize the operand type and syntax by changing `Operand` and `eval_operand(token)`. The unary and
binary operator tables define their functions and precedences; binary rules additionally specify
right-associativity. Prefix unary operators are emitted with a `u` prefix, so unary `-` becomes the
distinct postfix token `u-`. Operators may contain multiple non-operand characters, and the longest
matching operator is chosen. Operands are maximal strings of letters, digits, underscores, and
periods.

- `tokenize(s)` splits expression `s` into operands, operators, and parentheses. It throws
  `runtime_error` for unknown tokens.
- `to_postfix(tokens)` converts infix `tokens` into postfix notation. It throws `runtime_error` for
  malformed expressions or mismatched parentheses.
- `eval_postfix(tokens)` evaluates a tokenized postfix expression. It throws `runtime_error` if an
  operator lacks operands or the expression does not produce one value.
- `eval_expression(s)` converts and evaluates infix expression `s`.

Time Complexity:
- O(m*k*log m + n*m*k) expected per call to `tokenize()` and `eval_expression()`, where $n$ is the
  length of `s`, $m$ is the total number of operators, and $k$ is their maximum length.
- O(n) expected per call to `to_postfix()` and `eval_postfix()`, where $n$ is the total length of
  the tokens.

Space Complexity:
- O(n + m*k) auxiliary space for `tokenize()` and `eval_expression()`.
- O(n) auxiliary space for `to_postfix()` and `eval_postfix()`.

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

std::vector<string> tokenize(const string &s) {
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
  auto is_operand_char = [](unsigned char c) { return std::isalnum(c) || c == '_' || c == '.'; };
  std::vector<string> tokens;
  int n = static_cast<int>(s.size());
  for (int i = 0; i < n;) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    if (std::isspace(c)) {
      i++;
    } else if (is_operand_char(c)) {
      int j = i + 1;
      while (j < n && is_operand_char(static_cast<unsigned char>(s[j]))) {
        j++;
      }
      tokens.push_back(s.substr(i, j - i));
      i = j;
    } else if (s[i] == '(' || s[i] == ')') {
      tokens.emplace_back(1, s[i]);
      i++;
    } else {
      string op;
      for (const string &candidate : op_tokens) {
        if (s.compare(i, candidate.size(), candidate) == 0) {
          op = candidate;
          break;
        }
      }
      require(!op.empty(), "Unknown token at position " + std::to_string(i) + ".");
      tokens.push_back(op);
      i += static_cast<int>(op.size());
    }
  }
  return tokens;
}

std::vector<string> to_postfix(const std::vector<string> &tokens) {
  struct PendingOp {
    string token;
    int precedence;
  };
  std::vector<string> output;
  std::vector<PendingOp> op_stack;
  bool expect_operand = true;
  for (const string &token : tokens) {
    bool is_operator = unary_ops.count(token) || binary_ops.count(token);
    if (!is_operator && token != "(" && token != ")") {
      require(expect_operand, "Expected an operator before token '" + token + "'.");
      output.push_back(token);
      expect_operand = false;
    } else if (token == "(") {
      require(expect_operand, "Expected an operator before opening parenthesis.");
      op_stack.push_back({"(", -1});
    } else if (token == ")") {
      require(!expect_operand, "Unexpected closing parenthesis.");
      while (!op_stack.empty() && op_stack.back().token != "(") {
        output.push_back(op_stack.back().token);
        op_stack.pop_back();
      }
      require(!op_stack.empty(), "Mismatched parentheses.");
      op_stack.pop_back();
      expect_operand = false;
    } else if (expect_operand) {
      auto it = unary_ops.find(token);
      require(it != unary_ops.end(), "Expected an operand before token '" + token + "'.");
      op_stack.push_back({"u" + token, it->second.precedence});
    } else {
      auto it = binary_ops.find(token);
      require(it != binary_ops.end(), "Expected a binary operator, got '" + token + "'.");
      const BinaryRule &rule = it->second;
      while (!op_stack.empty() && op_stack.back().token != "(" &&
             (op_stack.back().precedence > rule.precedence ||
              (op_stack.back().precedence == rule.precedence && !rule.right_associative))) {
        output.push_back(op_stack.back().token);
        op_stack.pop_back();
      }
      op_stack.push_back({token, rule.precedence});
      expect_operand = true;
    }
  }
  require(!expect_operand, "Expression ends before an operand.");
  while (!op_stack.empty()) {
    require(op_stack.back().token != "(", "Mismatched parentheses.");
    output.push_back(op_stack.back().token);
    op_stack.pop_back();
  }
  return output;
}

Operand eval_postfix(const std::vector<string> &tokens) {
  std::vector<Operand> values;
  for (const string &token : tokens) {
    auto binary = binary_ops.find(token);
    auto unary =
        token.size() > 1 && token[0] == 'u' ? unary_ops.find(token.substr(1)) : unary_ops.end();
    if (binary != binary_ops.end()) {
      require(values.size() >= 2, "Binary operator lacks two operands: " + token);
      Operand b = values.back();
      values.pop_back();
      values.back() = binary->second.op(values.back(), b);
    } else if (unary != unary_ops.end()) {
      require(!values.empty(), "Unary operator lacks an operand: " + token);
      values.back() = unary->second.op(values.back());
    } else {
      values.push_back(eval_operand(token));
    }
  }
  require(values.size() == 1, "Invalid postfix expression.");
  return values.back();
}

Operand eval_expression(const string &s) {
  return eval_postfix(to_postfix(tokenize(s)));
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
      {"<=", {[](Operand a, Operand b) { return a <= b; }, 0}},
      {"+", {[](Operand a, Operand b) { return a + b; }, 1}},
      {"-", {[](Operand a, Operand b) { return a - b; }, 1}},
      {"*", {[](Operand a, Operand b) { return a * b; }, 2}},
      {"/", {[](Operand a, Operand b) { return a / b; }, 2}},
      {"^", {[](Operand a, Operand b) { return pow(a, b); }, 4, true}},
  };
  assert((tokenize("left <= -12.5") == vector<string>{"left", "<=", "-", "12.5"}));
  assert((to_postfix(tokenize("-(2+3)*4")) == vector<string>{"2", "3", "+", "u-", "4", "*"}));
  assert((to_postfix(tokenize("2^-3^2")) == vector<string>{"2", "3", "2", "^", "u-", "^"}));
  assert(EQ(eval_postfix({"2", "3", "4", "*", "+"}), 14));
  assert(EQ(eval_expression("-2^2"), -4));
  assert(EQ(eval_expression("2^-3"), 0.125));
  assert(EQ(eval_expression("1<=1"), 1));
  assert(EQ(eval_expression("5*(3+2)/-1*-2"), 50));
  return 0;
}
