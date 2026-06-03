/*

Solve a Boolean formula in 2-CNF, where each clause contains at most two literals. A clause like $(a
\lor b)$ is represented by the implications $\lnot a \rightarrow b$ and $\lnot b \rightarrow a$. The
formula is satisfiable if and only if no variable and its negation belong to the same strongly
connected component of the implication graph.

Variables are numbered from $0$ to $n - 1$. A literal is represented by `literal(variable, value)`,
where `value == true` means the variable itself and `value == false` means its negation.

- `add_or(a, b)` adds the clause $(a \lor b)$.
- `add_true(a)` forces literal `a` to be true.
- `satisfiable()` returns whether all added clauses can be satisfied and stores one valid assignment
  in `answer[]`.

Time Complexity:
- O(max(n, m)) per call to `satisfiable()`, where $n$ is the number of variables and $m$ is the
  number of clauses.

Space Complexity:
- O(max(n, m)) for the implication graph and DFS stacks.

*/

#include <algorithm>
#include <vector>

struct TwoSAT {
  int variables;
  std::vector<std::vector<int>> graph, reverse_graph;
  std::vector<int> order, component, answer;

  TwoSAT(int n = 0) { init(n); }

  void init(int n) {
    variables = n;
    graph.assign(2 * n, std::vector<int>());
    reverse_graph.assign(2 * n, std::vector<int>());
    answer.assign(n, false);
  }

  int literal(int variable, bool value) { return 2 * variable + (value ? 0 : 1); }
  int neg(int x) { return x ^ 1; }

  void add_implication(int a, int b) {
    graph[a].push_back(b);
    reverse_graph[b].push_back(a);
  }

  void add_or(int a, int b) {
    add_implication(neg(a), b);
    add_implication(neg(b), a);
  }

  void add_true(int a) { add_implication(neg(a), a); }
  void add_false(int a) { add_implication(a, neg(a)); }

  void dfs_order(int u, std::vector<bool> &visited) {
    visited[u] = true;
    for (int i = 0; i < static_cast<int>(graph[u].size()); i++) {
      int v = graph[u][i];
      if (!visited[v]) {
        dfs_order(v, visited);
      }
    }
    order.push_back(u);
  }

  void dfs_component(int u, int id) {
    component[u] = id;
    for (int i = 0; i < static_cast<int>(reverse_graph[u].size()); i++) {
      int v = reverse_graph[u][i];
      if (component[v] == -1) {
        dfs_component(v, id);
      }
    }
  }

  bool satisfiable() {
    order.clear();
    component.assign(2 * variables, -1);
    std::vector<bool> visited(2 * variables, false);
    for (int i = 0; i < 2 * variables; i++) {
      if (!visited[i]) {
        dfs_order(i, visited);
      }
    }
    std::reverse(order.begin(), order.end());
    int id = 0;
    for (int i = 0; i < static_cast<int>(order.size()); i++) {
      if (component[order[i]] == -1) {
        dfs_component(order[i], id++);
      }
    }
    for (int i = 0; i < variables; i++) {
      if (component[2 * i] == component[2 * i + 1]) {
        return false;
      }
      answer[i] = component[2 * i] > component[2 * i + 1];
    }
    return true;
  }
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
  assert(solver.satisfiable());
  assert(solver.answer[0]);
  return 0;
}
