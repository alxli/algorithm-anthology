/*

Maintain a dynamic collection of points in a bounded two-dimensional region while supporting
rectangle reporting, radius reporting, and nearest-neighbor queries. A quadtree recursively divides
space into at most four disjoint regions. Each leaf stores a small bucket of points, and queries
prune regions using their spatial bounds rather than decomposing each coordinate independently.

The same implementation supports integral and floating-point coordinates. A leaf stops splitting at
depth `MAX_DEPTH` or when rounding leaves neither coordinate interval with an interior midpoint; its
bucket may then grow beyond `BUCKET_SIZE`. These guards make duplicate and arbitrarily close
floating-point coordinates safe without giving integral and floating-point trees different logic.

Unlike the static 2D range tree and k-d tree, this structure supports online insertions and
erasures. Its spatial pruning also handles disks and nearest neighbors naturally, but its query cost
depends on the point distribution and requested region. Use a 2D segment or Fenwick tree instead for
guaranteed polylogarithmic rectangle aggregates over grid cells.

- `Quadtree<T, BUCKET_SIZE, MAX_DEPTH>(x1, y1, x2, y2)` constructs an empty tree covering the
  half-open region $[`x1`, `x2`) \times [`y1`, `y2`)$. Both coordinate intervals must be nonempty.
- `size()` returns the number of stored points, counting duplicates.
- `insert(x, y)` inserts one occurrence of (`x`, `y`), which must lie inside the covered region.
- `erase(x, y)` erases one occurrence of (`x`, `y`) and returns whether one existed. Floating-point
  coordinates are compared exactly.
- `query(x1, y1, x2, y2)` returns every stored point in the closed rectangle
  $[`x1`, `x2`] \times [`y1`, `y2`]$, including duplicate occurrences.
- `query_radius(x, y, radius)` returns every stored point at Euclidean distance at most `radius`
  from (`x`, `y`). `radius` must be nonnegative.
- `nearest(x, y, can_equal = true)` returns a stored point closest to (`x`, `y`) by Euclidean
  distance. This may equal (`x`, `y`) only if `can_equal` is `true`; at least one eligible point
  must exist.
- `k_nearest(x, y, k, can_equal = true)` returns the `k` closest stored points in nondecreasing
  order of Euclidean distance, counting duplicates. Points equal to (`x`, `y`) are eligible only if
  `can_equal` is `true`; $k$ must be positive and at most the number of eligible points.

Overflow warning: Coordinate differences used to split the covered region must fit in `int64_t` for
integral types or `long double` otherwise. The same applies to squared differences and their sums in
radius and nearest-neighbor queries.

Time Complexity:
- O(1) per call to the constructor and `size()`.
- O(B*D) amortized per call to `insert()`, where $B$ is `BUCKET_SIZE` and $D$ is `MAX_DEPTH`.
- O(D + b) per call to `erase()`, where $b$ is the size of the terminal bucket and can be O(n).
- O(v + m) per call to `query()` and `query_radius()`, where $v$ is the number of visited nodes and
  $m$ is the number of reported points; this is O(n*D) in the worst case.
- O(v + b) per call to `nearest()`, where $v$ is the number of visited nodes and $b$ is the total
  number of inspected bucket entries; this is O(n*D) in the worst case.
- O(v + b log k) per call to `k_nearest()`, with the same definitions of $v$ and $b$; this is
  O(n*(D + log k)) in the worst case.

Space Complexity:
- O(n*D) for storage after inserting $n$ points.
- O(D) auxiliary stack space for `insert()`, `erase()`, and all queries.
- O(k) auxiliary for `k_nearest()`.
- O(m) for the vector returned by a reporting query, where $m$ is the number of reported points.

*/

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <queue>
#include <type_traits>
#include <utility>
#include <vector>

template<typename T, int BUCKET_SIZE = 8, int MAX_DEPTH = 64>
class Quadtree {
  static_assert(std::is_arithmetic_v<T> && !std::is_same_v<T, bool>);
  static_assert(BUCKET_SIZE > 0 && MAX_DEPTH >= 0);

  using Dist = std::conditional_t<std::is_integral_v<T>, int64_t, long double>;
  using Point = std::pair<T, T>;
  using Candidate = std::pair<Dist, Point>;

  struct Node {
    T x1, y1, x2, y2;
    int size;
    std::vector<Point> points;
    std::array<std::unique_ptr<Node>, 4> children;

    Node(const T &x1, const T &y1, const T &x2, const T &y2)
        : x1(x1), y1(y1), x2(x2), y2(y2), size(0) {}
  };

  std::unique_ptr<Node> root;

  static T midpoint(const T &lo, const T &hi) {
    return T(Dist(lo) + (Dist(hi) - Dist(lo)) / Dist(2));
  }

  static bool is_leaf(const Node *n) {
    return std::all_of(n->children.begin(), n->children.end(), [](const auto &child) {
      return child == nullptr;
    });
  }

  static int child_index(const Node *n, const Point &p) {
    T mid_x = midpoint(n->x1, n->x2), mid_y = midpoint(n->y1, n->y2);
    bool split_x = n->x1 < mid_x && mid_x < n->x2;
    bool split_y = n->y1 < mid_y && mid_y < n->y2;
    int index = split_x && !(p.first < mid_x) ? 1 : 0;
    return index + (split_y && !(p.second < mid_y) ? 2 : 0);
  }

  static std::unique_ptr<Node> make_child(const Node *n, int index) {
    T mid_x = midpoint(n->x1, n->x2), mid_y = midpoint(n->y1, n->y2);
    bool split_x = n->x1 < mid_x && mid_x < n->x2;
    bool split_y = n->y1 < mid_y && mid_y < n->y2;
    T x1 = split_x && (index & 1) != 0 ? mid_x : n->x1;
    T x2 = split_x && (index & 1) == 0 ? mid_x : n->x2;
    T y1 = split_y && (index & 2) != 0 ? mid_y : n->y1;
    T y2 = split_y && (index & 2) == 0 ? mid_y : n->y2;
    return std::make_unique<Node>(x1, y1, x2, y2);
  }

  static bool can_split(const Node *n, int depth) {
    if (depth >= MAX_DEPTH) {
      return false;
    }
    T mid_x = midpoint(n->x1, n->x2), mid_y = midpoint(n->y1, n->y2);
    return (n->x1 < mid_x && mid_x < n->x2) || (n->y1 < mid_y && mid_y < n->y2);
  }

  static void collect(const Node *n, std::vector<Point> &res) {
    if (is_leaf(n)) {
      res.insert(res.end(), n->points.begin(), n->points.end());
      return;
    }
    for (const auto &child : n->children) {
      if (child != nullptr) {
        collect(child.get(), res);
      }
    }
  }

  void insert_child(Node *n, const Point &p, int depth) {
    int index = child_index(n, p);
    if (n->children[index] == nullptr) {
      n->children[index] = make_child(n, index);
    }
    insert(n->children[index].get(), p, depth);
  }

  void insert(Node *n, const Point &p, int depth) {
    n->size++;
    if (!is_leaf(n)) {
      insert_child(n, p, depth + 1);
      return;
    }
    n->points.push_back(p);
    if (static_cast<int>(n->points.size()) <= BUCKET_SIZE || !can_split(n, depth)) {
      return;
    }
    std::vector<Point> points = std::move(n->points);
    for (const Point &point : points) {
      insert_child(n, point, depth + 1);
    }
  }

  bool erase(Node *n, const Point &p) {
    if (is_leaf(n)) {
      auto it = std::find(n->points.begin(), n->points.end(), p);
      if (it == n->points.end()) {
        return false;
      }
      n->points.erase(it);
      n->size--;
      return true;
    }
    int index = child_index(n, p);
    if (n->children[index] == nullptr || !erase(n->children[index].get(), p)) {
      return false;
    }
    n->size--;
    if (n->children[index]->size == 0) {
      n->children[index].reset();
    }
    if (n->size <= BUCKET_SIZE) {
      n->points.reserve(n->size);
      collect(n, n->points);
      for (auto &child : n->children) {
        child.reset();
      }
    }
    return true;
  }

  static bool disjoint(const Node *n, const T &x1, const T &y1, const T &x2, const T &y2) {
    return x2 < n->x1 || !(x1 < n->x2) || y2 < n->y1 || !(y1 < n->y2);
  }

  static bool covered(const Node *n, const T &x1, const T &y1, const T &x2, const T &y2) {
    return !(n->x1 < x1) && !(x2 < n->x2) && !(n->y1 < y1) && !(y2 < n->y2);
  }

  void query(
      const Node *n, const T &x1, const T &y1, const T &x2, const T &y2, std::vector<Point> &res
  ) const {
    if (disjoint(n, x1, y1, x2, y2)) {
      return;
    }
    if (covered(n, x1, y1, x2, y2)) {
      collect(n, res);
      return;
    }
    if (is_leaf(n)) {
      for (const Point &p : n->points) {
        if (!(p.first < x1) && !(x2 < p.first) && !(p.second < y1) && !(y2 < p.second)) {
          res.push_back(p);
        }
      }
      return;
    }
    for (const auto &child : n->children) {
      if (child != nullptr) {
        query(child.get(), x1, y1, x2, y2, res);
      }
    }
  }

  static Dist box_dist(const Node *n, const T &x, const T &y) {
    Dist dx = 0, dy = 0;
    if (x < n->x1) {
      dx = Dist(n->x1) - Dist(x);
    } else if (!(x < n->x2)) {
      dx = Dist(x) - Dist(n->x2);
    }
    if (y < n->y1) {
      dy = Dist(n->y1) - Dist(y);
    } else if (!(y < n->y2)) {
      dy = Dist(y) - Dist(n->y2);
    }
    return dx * dx + dy * dy;  // Overflow warning.
  }

  static Dist point_dist(const Point &p, const T &x, const T &y) {
    Dist dx = Dist(x) - Dist(p.first), dy = Dist(y) - Dist(p.second);
    return dx * dx + dy * dy;  // Overflow warning.
  }

  void query_radius(
      const Node *n, const T &x, const T &y, Dist radius_sq, std::vector<Point> &res
  ) const {
    if (radius_sq < box_dist(n, x, y)) {
      return;
    }
    if (is_leaf(n)) {
      for (const Point &p : n->points) {
        if (point_dist(p, x, y) <= radius_sq) {
          res.push_back(p);
        }
      }
      return;
    }
    for (const auto &child : n->children) {
      if (child != nullptr) {
        query_radius(child.get(), x, y, radius_sq, res);
      }
    }
  }

  void k_nearest(
      const Node *n, const T &x, const T &y, int k, bool can_equal,
      std::priority_queue<Candidate> &best
  ) const {
    if (static_cast<int>(best.size()) == k && best.top().first <= box_dist(n, x, y)) {
      return;
    }
    if (is_leaf(n)) {
      for (const Point &p : n->points) {
        Dist dist = point_dist(p, x, y);
        if (!can_equal && dist == 0) {
          continue;
        }
        if (static_cast<int>(best.size()) < k) {
          best.emplace(dist, p);
        } else if (dist < best.top().first) {
          best.pop();
          best.emplace(dist, p);
        }
      }
      return;
    }
    std::array<std::pair<Dist, const Node *>, 4> order;
    int count = 0;
    for (const auto &child : n->children) {
      if (child != nullptr) {
        order[count++] = {box_dist(child.get(), x, y), child.get()};
      }
    }
    for (int i = 1; i < count; i++) {
      auto entry = order[i];
      int j = i;
      for (; j > 0 && entry.first < order[j - 1].first; j--) {
        order[j] = order[j - 1];
      }
      order[j] = entry;
    }
    for (int i = 0; i < count; i++) {
      auto [dist, child] = order[i];
      if (static_cast<int>(best.size()) == k && best.top().first <= dist) {
        break;
      }
      k_nearest(child, x, y, k, can_equal, best);
    }
  }

 public:
  Quadtree(const T &x1, const T &y1, const T &x2, const T &y2) {
    assert(x1 < x2 && y1 < y2);
    root = std::make_unique<Node>(x1, y1, x2, y2);
  }

  Quadtree(const Quadtree &) = delete;
  Quadtree &operator=(const Quadtree &) = delete;

  int size() const { return root->size; }

  void insert(const T &x, const T &y) {
    assert(!(x < root->x1) && x < root->x2 && !(y < root->y1) && y < root->y2);
    insert(root.get(), {x, y}, 0);
  }

  bool erase(const T &x, const T &y) { return erase(root.get(), {x, y}); }

  std::vector<Point> query(const T &x1, const T &y1, const T &x2, const T &y2) const {
    assert(!(x2 < x1) && !(y2 < y1));
    std::vector<Point> res;
    query(root.get(), x1, y1, x2, y2, res);
    return res;
  }

  std::vector<Point> query_radius(const T &x, const T &y, const T &radius) const {
    assert(!(radius < T{}));
    Dist radius_sq = Dist(radius) * Dist(radius);  // Overflow warning.
    std::vector<Point> res;
    query_radius(root.get(), x, y, radius_sq, res);
    return res;
  }

  Point nearest(const T &x, const T &y, bool can_equal = true) const {
    return k_nearest(x, y, 1, can_equal)[0];
  }

  std::vector<Point> k_nearest(const T &x, const T &y, int k, bool can_equal = true) const {
    assert(k > 0);
    std::priority_queue<Candidate> best;
    k_nearest(root.get(), x, y, k, can_equal, best);
    assert(static_cast<int>(best.size()) == k);
    std::vector<Point> res(best.size());
    for (int i = k - 1; i >= 0; i--) {
      res[i] = best.top().second;
      best.pop();
    }
    return res;
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  Quadtree<int, 2> t(0, 0, 16, 16);
  t.insert(1, 1);
  t.insert(4, 5);
  t.insert(7, 5);
  t.insert(12, 12);
  assert(t.size() == 4);

  auto got = t.query(0, 0, 7, 5);
  sort(got.begin(), got.end());
  assert((got == vector<pair<int, int>>{{1, 1}, {4, 5}, {7, 5}}));
  got = t.query_radius(5, 5, 2);
  sort(got.begin(), got.end());
  assert((got == vector<pair<int, int>>{{4, 5}, {7, 5}}));
  assert(t.nearest(6, 4) == (pair<int, int>{7, 5}));
  assert(t.nearest(7, 5, false) == (pair<int, int>{4, 5}));
  assert((t.k_nearest(6, 4, 3) == vector<pair<int, int>>{{7, 5}, {4, 5}, {1, 1}}));
  assert((t.k_nearest(7, 5, 2, false) == vector<pair<int, int>>{{4, 5}, {1, 1}}));
  assert(t.erase(4, 5));
  assert(!t.erase(4, 5));

  Quadtree<double, 1, 8> close(0.0, 0.0, 1.0, 1.0);
  close.insert(0.5, 0.5);
  close.insert(0.5, 0.5);
  close.insert(0.5000000000000001, 0.5);
  assert(close.size() == 3);
  assert(close.query(0.5, 0.5, 0.5, 0.5).size() == 2);
  assert(close.erase(0.5, 0.5));
  assert(close.size() == 2);
  return 0;
}
