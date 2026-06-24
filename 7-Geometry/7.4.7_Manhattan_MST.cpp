/*

Given points in the plane, produce a small set of candidate edges guaranteed to contain a minimum
spanning tree under Manhattan distance $|x_1 - x_2| + |y_1 - y_2|$. The plane is swept in four
rotations/reflections; in each sweep, dominance by $x + y$ identifies the only nearby candidates
that can matter. Run Kruskal on the returned edges to obtain the MST.

- `manhattan_mst_edges(points)` returns O(n) candidate edges $(w, u, v)$.
- `manhattan_mst_weight(points)` is a small Kruskal wrapper that returns the MST weight.

Time Complexity:
- O(n log n), where $n$ is the number of points.

Space Complexity:
- O(n) auxiliary.

*/

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <map>
#include <numeric>
#include <vector>

struct Point {
  int64_t x, y;
  Point(int64_t x = 0, int64_t y = 0) : x(x), y(y) {}
};

std::vector<std::tuple<int64_t, int, int>> manhattan_mst_edges(std::vector<Point> p) {
  int n = static_cast<int>(p.size());
  std::vector<int> id(n);
  std::iota(id.begin(), id.end(), 0);
  std::vector<std::tuple<int64_t, int, int>> edges;
  for (int rot = 0; rot < 4; rot++) {
    std::sort(id.begin(), id.end(), [&](int i, int j) {
      return p[i].x + p[i].y < p[j].x + p[j].y;
    });
    std::map<int64_t, int> sweep;
    for (int i : id) {
      for (auto it = sweep.lower_bound(-p[i].y); it != sweep.end();) {
        int j = it->second;
        int64_t dx = p[i].x - p[j].x, dy = p[i].y - p[j].y;
        if (dy > dx) {
          break;
        }
        edges.emplace_back(dx + dy, i, j);
        it = sweep.erase(it);
      }
      sweep[-p[i].y] = i;
    }
    for (Point &q : p) {
      if (rot & 1) {
        q.x = -q.x;
      } else {
        std::swap(q.x, q.y);
      }
    }
  }
  return edges;
}

struct DSU {
  std::vector<int> parent, rank;

  explicit DSU(int n) : parent(n), rank(n, 0) { std::iota(parent.begin(), parent.end(), 0); }
  int find(int u) { return parent[u] == u ? u : parent[u] = find(parent[u]); }

  bool unite(int u, int v) {
    u = find(u);
    v = find(v);
    if (u == v) {
      return false;
    }
    if (rank[u] < rank[v]) {
      std::swap(u, v);
    }
    parent[v] = u;
    rank[u] += rank[u] == rank[v];
    return true;
  }
};

int64_t manhattan_mst_weight(const std::vector<Point> &p) {
  auto edges = manhattan_mst_edges(p);
  std::sort(edges.begin(), edges.end());
  DSU dsu(static_cast<int>(p.size()));
  int64_t res = 0;
  for (auto [w, u, v] : edges) {
    if (dsu.unite(static_cast<int>(u), static_cast<int>(v))) {
      res += w;
    }
  }
  return res;
}

/*** Example Usage ***/

using namespace std;

int main() {
  assert(manhattan_mst_weight({Point(5, -7)}) == 0);
  assert(manhattan_mst_weight({Point(0, 0), Point(3, -4)}) == 7);

  vector<Point> p{{0, 0}, {2, 1}, {3, 4}, {-1, 2}, {5, 0}};
  assert(manhattan_mst_weight(p) == 14);
  auto edges = manhattan_mst_edges(p);
  assert(edges.size() <= 4 * p.size());
  return 0;
}
