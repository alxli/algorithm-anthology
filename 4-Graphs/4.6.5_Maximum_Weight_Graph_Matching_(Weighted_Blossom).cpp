/*

Given an undirected graph with positive edge weights, determine a maximum-weight matching: a subset
of edges, no two sharing a node, whose total weight is as large as possible. Unlike the bipartite
case (which Hungarian or min-cost max-flow solves), general graphs may contain odd cycles, so this
is the weighted generalization of Edmonds' blossom algorithm.

The algorithm runs a primal-dual method. Each node carries a dual label, and an edge is "tight" when
its label sum equals twice its weight; the search uses only tight edges, exactly as unweighted
blossom matching does, contracting odd cycles into blossoms. When no tight augmenting structure
remains, the labels are adjusted by the largest step that keeps every dual constraint satisfied,
which makes a new edge tight or lets a blossom expand, and the search resumes. Because the matching
is over the implicit complete graph with absent edges weighted 0, a node may be left effectively
unmatched whenever staying unmatched is at least as good as any incident edge.

All edge weights must be positive; an absent edge is treated as weight 0. The node count should be
kept modest, as the algorithm uses dense storage.

- `WeightedGeneralMatching(n)` creates a graph of `n` nodes numbered $[0, `n`)$.
- `add_edge(u, v, w)` adds an undirected edge of positive weight `w` between `u` and `v`. If several
  edges join the same pair, the last weight set is used.
- `solve()` computes a maximum-weight matching and returns its total weight.
- `partner(u)` returns the node matched with `u`, or $-1$ if `u` is unmatched. Valid after
  `solve()`.

Time Complexity:
- O(n^3) per call to `solve()`, where $n$ is the number of nodes.

Space Complexity:
- O(n^2) for the dense edge and blossom storage.

*/

#include <algorithm>
#include <cstdint>
#include <queue>
#include <vector>

class WeightedGeneralMatching {
  struct Edge {
    int u, v;
    int64_t w;
  };

  int n, nx;  // n original nodes (1-based internally); nx includes blossom nodes.
  std::vector<std::vector<Edge>> g;
  std::vector<std::vector<int>> flo, flo_from;
  std::vector<int64_t> lab;
  std::vector<int> match_, slack, st, par, s_, aux_;
  std::queue<int> q;
  int aux_clock;

  int64_t e_delta(const Edge &e) const { return lab[e.u] + lab[e.v] - 2 * e.w; }

  void update_slack(int u, int x) {
    if (slack[x] == 0 || e_delta(g[u][x]) < e_delta(g[slack[x]][x])) {
      slack[x] = u;
    }
  }

  void set_slack(int x) {
    slack[x] = 0;
    for (int u = 1; u <= n; u++) {
      if (g[u][x].w > 0 && st[u] != x && s_[st[u]] == 0) {
        update_slack(u, x);
      }
    }
  }

  void queue_push(int x) {
    if (x <= n) {
      q.push(x);
    } else {
      for (int t : flo[x]) {
        queue_push(t);
      }
    }
  }

  void set_st(int x, int b) {
    st[x] = b;
    if (x > n) {
      for (int t : flo[x]) {
        set_st(t, b);
      }
    }
  }

  int get_pr(int b, int xr) {
    int pr = static_cast<int>(std::find(flo[b].begin(), flo[b].end(), xr) - flo[b].begin());
    if (pr & 1) {
      std::reverse(flo[b].begin() + 1, flo[b].end());
      return static_cast<int>(flo[b].size()) - pr;
    }
    return pr;
  }

  void set_match(int u, int v) {
    match_[u] = g[u][v].v;
    if (u <= n) {
      return;
    }
    Edge e = g[u][v];
    int xr = flo_from[u][e.u], pr = get_pr(u, xr);
    for (int i = 0; i < pr; i++) {
      set_match(flo[u][i], flo[u][i ^ 1]);
    }
    set_match(xr, v);
    std::rotate(flo[u].begin(), flo[u].begin() + pr, flo[u].end());
  }

  void augment(int u, int v) {
    while (true) {
      int xnv = st[match_[u]];
      set_match(u, v);
      if (xnv == 0) {
        return;
      }
      set_match(xnv, st[par[xnv]]);
      u = st[par[xnv]];
      v = xnv;
    }
  }

  int lca(int u, int v) {
    aux_clock++;
    while (u != 0 || v != 0) {
      if (u == 0) {
        std::swap(u, v);
        continue;
      }
      if (aux_[u] == aux_clock) {
        return u;
      }
      aux_[u] = aux_clock;
      u = st[match_[u]];
      if (u != 0) {
        u = st[par[u]];
      }
      std::swap(u, v);
    }
    return 0;
  }

  void add_blossom(int u, int anc, int v) {
    int b = n + 1;
    while (b <= nx && st[b] != 0) {
      b++;
    }
    if (b > nx) {
      nx++;
    }
    lab[b] = 0;
    s_[b] = 0;
    match_[b] = match_[anc];
    flo[b] = {anc};
    auto build = [&](int x) {
      for (int y; x != anc; x = st[par[y]]) {
        flo[b].push_back(x);
        y = st[match_[x]];
        flo[b].push_back(y);
        queue_push(y);
      }
    };
    build(u);
    std::reverse(flo[b].begin() + 1, flo[b].end());
    build(v);
    set_st(b, b);
    for (int x = 1; x <= nx; x++) {
      g[b][x].w = g[x][b].w = 0;
    }
    for (int x = 1; x <= n; x++) {
      flo_from[b][x] = 0;
    }
    for (int xs : flo[b]) {
      for (int x = 1; x <= nx; x++) {
        if (g[b][x].w == 0 || e_delta(g[xs][x]) < e_delta(g[b][x])) {
          g[b][x] = g[xs][x];
          g[x][b] = g[x][xs];
        }
      }
      for (int x = 1; x <= n; x++) {
        if (flo_from[xs][x] != 0) {
          flo_from[b][x] = xs;
        }
      }
    }
    set_slack(b);
  }

  void expand_blossom(int b) {
    for (int t : flo[b]) {
      set_st(t, t);
    }
    int xr = flo_from[b][g[b][par[b]].u], pr = get_pr(b, xr);
    for (int i = 0; i < pr; i += 2) {
      int xs = flo[b][i], xns = flo[b][i + 1];
      par[xs] = g[xns][xs].u;
      s_[xs] = 1;
      s_[xns] = 0;
      slack[xs] = 0;
      set_slack(xns);
      queue_push(xns);
    }
    s_[xr] = 1;
    par[xr] = par[b];
    for (int i = pr + 1; i < static_cast<int>(flo[b].size()); i++) {
      int xs = flo[b][i];
      s_[xs] = -1;
      set_slack(xs);
    }
    st[b] = 0;
  }

  bool on_found_edge(const Edge &e) {
    int u = st[e.u], v = st[e.v];
    if (s_[v] == -1) {
      par[v] = e.u;
      s_[v] = 1;
      int nu = st[match_[v]];
      slack[v] = slack[nu] = 0;
      s_[nu] = 0;
      queue_push(nu);
    } else if (s_[v] == 0) {
      int anc = lca(u, v);
      if (anc == 0) {
        augment(u, v);
        augment(v, u);
        return true;
      }
      add_blossom(u, anc, v);
    }
    return false;
  }

  bool matching() {
    std::fill(s_.begin(), s_.begin() + nx + 1, -1);
    std::fill(slack.begin(), slack.begin() + nx + 1, 0);
    q = std::queue<int>();
    for (int x = 1; x <= nx; x++) {
      if (st[x] == x && match_[x] == 0) {
        par[x] = s_[x] = 0;
        queue_push(x);
      }
    }
    if (q.empty()) {
      return false;
    }
    while (true) {
      while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (s_[st[u]] == 1) {
          continue;
        }
        for (int v = 1; v <= n; v++) {
          if (g[u][v].w > 0 && st[u] != st[v]) {
            if (e_delta(g[u][v]) == 0) {
              if (on_found_edge(g[u][v])) {
                return true;
              }
            } else {
              update_slack(u, st[v]);
            }
          }
        }
      }
      int64_t d = INT64_MAX;
      for (int b = n + 1; b <= nx; b++) {
        if (st[b] == b && s_[b] == 1) {
          d = std::min(d, lab[b] / 2);
        }
      }
      for (int x = 1; x <= nx; x++) {
        if (st[x] == x && slack[x] != 0) {
          if (s_[x] == -1) {
            d = std::min(d, e_delta(g[slack[x]][x]));
          } else if (s_[x] == 0) {
            d = std::min(d, e_delta(g[slack[x]][x]) / 2);
          }
        }
      }
      for (int u = 1; u <= n; u++) {
        if (s_[st[u]] == 0) {
          if (lab[u] <= d) {
            return false;
          }
          lab[u] -= d;
        } else if (s_[st[u]] == 1) {
          lab[u] += d;
        }
      }
      for (int b = n + 1; b <= nx; b++) {
        if (st[b] == b && s_[b] != -1) {
          lab[b] += (s_[b] == 0 ? 1 : -1) * d * 2;
        }
      }
      q = std::queue<int>();
      for (int x = 1; x <= nx; x++) {
        if (st[x] == x && slack[x] != 0 && st[slack[x]] != x && e_delta(g[slack[x]][x]) == 0) {
          if (on_found_edge(g[slack[x]][x])) {
            return true;
          }
        }
      }
      for (int b = n + 1; b <= nx; b++) {
        if (st[b] == b && s_[b] == 1 && lab[b] == 0) {
          expand_blossom(b);
        }
      }
    }
  }

 public:
  explicit WeightedGeneralMatching(int n)
      : n(n),
        nx(n),
        g(2 * n + 1, std::vector<Edge>(2 * n + 1)),
        flo(2 * n + 1),
        flo_from(2 * n + 1, std::vector<int>(n + 1, 0)),
        lab(2 * n + 1, 0),
        match_(2 * n + 1, 0),
        slack(2 * n + 1, 0),
        st(2 * n + 1, 0),
        par(2 * n + 1, 0),
        s_(2 * n + 1, 0),
        aux_(2 * n + 1, 0),
        aux_clock(0) {
    for (int u = 1; u <= n; u++) {
      for (int v = 1; v <= n; v++) {
        g[u][v] = {u, v, 0};
      }
    }
  }

  void add_edge(int u, int v, int64_t w) { g[u + 1][v + 1].w = g[v + 1][u + 1].w = w; }

  int64_t solve() {
    nx = n;
    for (int u = 1; u <= n; u++) {
      st[u] = u;
      match_[u] = 0;
      flo[u].clear();
    }
    int64_t w_max = 0;
    for (int u = 1; u <= n; u++) {
      for (int v = 1; v <= n; v++) {
        flo_from[u][v] = (u == v ? u : 0);
        w_max = std::max(w_max, g[u][v].w);
      }
    }
    for (int u = 1; u <= n; u++) {
      lab[u] = w_max;
    }
    while (matching()) {
    }
    int64_t weight = 0;
    for (int u = 1; u <= n; u++) {
      if (match_[u] != 0 && match_[u] < u) {
        weight += g[u][match_[u]].w;
      }
    }
    return weight;
  }

  int partner(int u) const {
    int v = match_[u + 1];
    // A node "matched" through an absent (weight 0) edge is really unmatched.
    if (v == 0 || g[u + 1][v].w == 0) {
      return -1;
    }
    return v - 1;
  }
};

/*** Example Usage and Output:

Maximum matching weight: 12
Matched pairs: (1,2) (3,4)

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  //   w=5   w=6   w=5   w=6
  // 0-----1-----2-----3-----4
  //  \_____________________/
  //            w=5
  WeightedGeneralMatching m(5);
  m.add_edge(0, 1, 5);
  m.add_edge(1, 2, 6);
  m.add_edge(2, 3, 5);
  m.add_edge(3, 4, 6);
  m.add_edge(4, 0, 5);
  int64_t weight = m.solve();
  assert(weight == 12);  // Edges 1-2 and 3-4 with weights 6 + 6.
  assert(m.partner(1) == 2 && m.partner(2) == 1);
  assert(m.partner(3) == 4 && m.partner(4) == 3);
  assert(m.partner(0) == -1);  // Node 0 is left unmatched.
  cout << "Maximum matching weight: " << weight << "\n";
  cout << "Matched pairs:";
  for (int u = 0; u < 5; u++) {
    int v = m.partner(u);
    if (u < v) {
      cout << " (" << u << "," << v << ")";
    }
  }
  cout << "\n";

  //   w=3   w=3   w=5
  // 0-----1-----2-----3
  //  \_________/
  //      w=3
  WeightedGeneralMatching t(4);
  t.add_edge(0, 1, 3);
  t.add_edge(1, 2, 3);
  t.add_edge(0, 2, 3);
  t.add_edge(2, 3, 5);
  assert(t.solve() == 8);  // Edges 0-1 and 2-3 with weights 3 + 5.
  return 0;
}
