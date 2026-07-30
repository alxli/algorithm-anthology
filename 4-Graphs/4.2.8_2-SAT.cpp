/*

Solve a Boolean formula in 2-CNF, where each clause contains at most two literals. A clause like
$(a \lor b)$ is represented by the implications $\lnot a \rightarrow b$ and $\lnot b \rightarrow a$.
The formula is satisfiable if and only if no variable and its negation belong to the same strongly
connected component of the implication graph.

Variables are numbered $[0, n)$. A literal is represented by `literal(variable, value)`, where
`value == true` means the variable itself and `value == false` means its negation.

- `TwoSAT(n = 0)` constructs an empty formula over `n` variables.
- `literal(variable, value)` returns the integer ID for a variable or its negation.
- `add_implication(a, b)` adds the implication $a \rightarrow b$.
- `add_or(a, b)` adds the clause $(`a` \lor `b`)$.
- `add_true(a)` forces literal `a` to be true.
- `add_false(a)` forces literal `a` to be false.
- `add(variable, value)` forces a variable to equal `value`.
- `add(x, xval, y, yval)` adds the clause $(`x` = `xval` \lor `y` = `yval`)$.
- `satisfiable()` returns whether all added clauses can be satisfied.
- `assignment()` returns the valid assignment found by the last successful `satisfiable()` call.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the number of variables.
- O(1) per call to `literal()`, `add_implication()`, `add_or()`, `add_true()`, `add_false()`, and
  `add()`.
- O(max(n, m)) per call to `satisfiable()`, where $n$ is the number of variables and $m$ is the
  number of edges in the implication graph.

Space Complexity:
- O(max(n, m)) for the implication graph and DFS stacks.

*/

#include <algorithm>
#include <vector>

class TwoSAT {
  int variables;
  std::vector<std::vector<int>> adj, rev;
  std::vector<int> order, component, solution;

  static int neg(int x) { return x ^ 1; }

  void add_edge(int a, int b) {
    adj[a].push_back(b);
    rev[b].push_back(a);
  }

  void dfs_order(int u, std::vector<char> &visited) {
    visited[u] = true;
    for (int v : adj[u]) {
      if (!visited[v]) {
        dfs_order(v, visited);
      }
    }
    order.push_back(u);
  }

  void dfs_component(int u, int id) {
    component[u] = id;
    for (int v : rev[u]) {
      if (component[v] == -1) {
        dfs_component(v, id);
      }
    }
  }

 public:
  explicit TwoSAT(int n = 0) : variables(n), adj(2 * n), rev(2 * n), solution(n, false) {}

  int literal(int variable, bool value) const { return 2 * variable + (value ? 0 : 1); }

  void add_implication(int a, int b) {
    add_edge(a, b);
    add_edge(neg(b), neg(a));
  }

  void add_or(int a, int b) { add_implication(neg(a), b); }
  void add_true(int a) { add_edge(neg(a), a); }
  void add_false(int a) { add_edge(a, neg(a)); }
  void add(int variable, bool value) { add_true(literal(variable, value)); }
  void add(int x, bool xval, int y, bool yval) { add_or(literal(x, xval), literal(y, yval)); }

  bool satisfiable() {
    order.clear();
    component.assign(2 * variables, -1);
    std::vector<char> visited(2 * variables, false);
    for (int i = 0; i < 2 * variables; i++) {
      if (!visited[i]) {
        dfs_order(i, visited);
      }
    }
    std::reverse(order.begin(), order.end());
    int id = 0;
    for (int u : order) {
      if (component[u] == -1) {
        dfs_component(u, id++);
      }
    }
    for (int i = 0; i < variables; i++) {
      if (component[2 * i] == component[2 * i + 1]) {
        return false;
      }
    }
    for (int i = 0; i < variables; i++) {
      solution[i] = component[2 * i] > component[2 * i + 1];
    }
    return true;
  }

  const std::vector<int> &assignment() const { return solution; }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  TwoSAT solver(3);
  int x0 = solver.literal(0, true);
  int not_x0 = solver.literal(0, false);
  int x1 = solver.literal(1, true);
  int x2 = solver.literal(2, true);
  solver.add_or(x0, x1);
  solver.add_or(not_x0, x2);
  solver.add_true(x0);
  solver.add(1, true, 2, false);
  assert(solver.satisfiable());
  assert((solver.assignment() == vector<int>{true, true, true}));

  TwoSAT stateful(2);
  assert(stateful.satisfiable());
  vector<int> last_assignment = stateful.assignment();
  stateful.add(0, false);
  stateful.add(1, true);
  stateful.add(1, false);
  assert(!stateful.satisfiable());
  assert(stateful.assignment() == last_assignment);

  TwoSAT implication(2);
  int x = implication.literal(0, true);
  int y = implication.literal(1, true);
  implication.add_true(x);
  implication.add_false(y);
  implication.add_implication(x, y);
  assert(!implication.satisfiable());
  return 0;
}
