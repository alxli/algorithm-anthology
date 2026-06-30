/*

Given a set $P$ of distinct two-dimensional points, a Delaunay triangulation of $P$ is a
triangulation of the convex hull of $P$ such that no point of $P$ lies strictly inside the
circumcircle of any triangle in the triangulation. The Delaunay triangulation is not necessarily
unique when four or more points are cocircular.

This implementation produces one valid triangulation using the Guibas-Stolfi divide-and-conquer
algorithm with a quad-edge data structure. The point set is split in half, each half is triangulated
recursively, and the halves are stitched together from the bottom up by a sequence of cross edges,
with circumcircle tests deciding each connecting edge and deleting invalidated ones.

- `delaunay_triangulation(pts)` sorts `pts` lexicographically and removes duplicate points in place
  (so the input vector is modified), then returns the triangles of one Delaunay triangulation as a
  vector of counterclockwise-oriented vertex triples (`std::tuple`) of the input point type. If
  fewer than three non-collinear unique points remain, the result is empty.

The point type must provide exact lexicographic `operator<`; sorting and duplicate removal requires
this strict ordering rather than epsilon equality.

All arithmetic uses the point's own coordinate type, so integer inputs yield an exact triangulation.
The in-circle test is a degree-4 polynomial in the coordinates and dominates the overflow budget: a
64-bit integer coordinate type stays exact up to $|C| \approx 1.4 \times 10^4$, where $C$ is the
coordinate magnitude. For floating-point coordinates the predicates are subject to the usual
rounding error.

Time Complexity:
- O(n log n) per call, where $n$ is the number of input points.

Space Complexity:
- O(n) for the quad-edge structure and returned triangulation.

*/

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

const double EPS = 1e-9;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return std::fabs(C(a) - C(b)) <= static_cast<C>(EPS);
  return C(a) == C(b);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool LT(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) < C(b) - static_cast<C>(EPS);
  return C(a) < C(b);
}

template<typename T>
struct Point {
  T x, y;
  Point(T x = 0, T y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  friend bool EQ(const Point &a, const Point &b) { return EQ(a.x, b.x) && EQ(a.y, b.y); }
  bool operator<(const Point &p) const { return std::tie(x, y) < std::tie(p.x, p.y); }
};

template<typename T>
T cross(const Point<T> &a, const Point<T> &b, const Point<T> &o) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);  // Overflow warning!
}

template<typename T>
T incircle(const Point<T> &a, const Point<T> &b, const Point<T> &c, const Point<T> &d) {
  T adx = a.x - d.x, ady = a.y - d.y;
  T bdx = b.x - d.x, bdy = b.y - d.y;
  T cdx = c.x - d.x, cdy = c.y - d.y;
  T abdet = adx * bdy - bdx * ady;  // Overflow warning!
  T bcdet = bdx * cdy - cdx * bdy;
  T cadet = cdx * ady - adx * cdy;
  T alift = adx * adx + ady * ady;
  T blift = bdx * bdx + bdy * bdy;
  T clift = cdx * cdx + cdy * cdy;
  return alift * bcdet + blift * cadet + clift * abdet;
}

template<typename T>
struct Edge {
  Point<T> origin;
  Edge *rot, *onext;
  bool primal, deleted, used;

  Edge *rev() const { return rot->rot; }
  Edge *lnext() const { return rot->rev()->onext->rot; }
  Edge *oprev() const { return rot->onext->rot; }
  Point<T> dest() const { return rev()->origin; }
};

template<typename T>
struct QuadEdgePool {
  std::vector<Edge<T> *> edges;

  QuadEdgePool() = default;

  ~QuadEdgePool() {
    for (Edge<T> *e : edges) {
      delete e;
    }
  }

  QuadEdgePool(const QuadEdgePool &) = delete;
  QuadEdgePool &operator=(const QuadEdgePool &) = delete;

  Edge<T> *make_edge(const Point<T> &u, const Point<T> &v) {
    Edge<T> *e1 = new Edge<T>, *e2 = new Edge<T>, *e3 = new Edge<T>, *e4 = new Edge<T>;
    edges.push_back(e1);
    edges.push_back(e2);
    edges.push_back(e3);
    edges.push_back(e4);
    e1->origin = u;
    e2->origin = v;
    e1->rot = e3;
    e2->rot = e4;
    e3->rot = e2;
    e4->rot = e1;
    e1->onext = e1;
    e2->onext = e2;
    e3->onext = e4;
    e4->onext = e3;
    e1->primal = e2->primal = true;
    e3->primal = e4->primal = false;
    e1->deleted = e2->deleted = e3->deleted = e4->deleted = false;
    e1->used = e2->used = e3->used = e4->used = false;
    return e1;
  }

  void delete_edge(Edge<T> *e) {
    splice(e, e->oprev());
    splice(e->rev(), e->rev()->oprev());
    e->deleted = e->rot->deleted = e->rev()->deleted = e->rev()->rot->deleted = true;
  }

  Edge<T> *connect(Edge<T> *a, Edge<T> *b) {
    Edge<T> *e = make_edge(a->dest(), b->origin);
    splice(e, a->lnext());
    splice(e->rev(), b);
    return e;
  }

  static void splice(Edge<T> *a, Edge<T> *b) {
    std::swap(a->onext->rot->onext, b->onext->rot->onext);
    std::swap(a->onext, b->onext);
  }
};

template<typename T>
bool left_of(const Point<T> &p, Edge<T> *e) {
  return LT(0, cross(e->origin, e->dest(), p));
}

template<typename T>
bool right_of(const Point<T> &p, Edge<T> *e) {
  return LT(cross(e->origin, e->dest(), p), 0);
}

template<typename T>
bool in_circle(const Point<T> &a, const Point<T> &b, const Point<T> &c, const Point<T> &d) {
  return LT(0, incircle(a, b, c, d));
}

template<typename T>
std::pair<Edge<T> *, Edge<T> *> build_triangulation(
    std::vector<Point<T>> &p, int lo, int hi, QuadEdgePool<T> &pool
) {
  if (hi - lo == 1) {
    Edge<T> *a = pool.make_edge(p[lo], p[hi]);
    return {a, a->rev()};
  }
  if (hi - lo == 2) {
    Edge<T> *a = pool.make_edge(p[lo], p[lo + 1]);
    Edge<T> *b = pool.make_edge(p[lo + 1], p[hi]);
    pool.splice(a->rev(), b);
    int side = LT(0, cross(p[lo], p[lo + 1], p[hi]))
                   ? 1
                   : (LT(cross(p[lo], p[lo + 1], p[hi]), 0) ? -1 : 0);
    if (side == 0) {
      return {a, b->rev()};
    }
    Edge<T> *c = pool.connect(b, a);
    if (side == 1) {
      return {a, b->rev()};
    }
    return {c->rev(), c};
  }
  int mid = lo + (hi - lo) / 2;
  auto [ldo, ldi] = build_triangulation(p, lo, mid, pool);
  auto [rdi, rdo] = build_triangulation(p, mid + 1, hi, pool);
  while (true) {
    if (left_of(rdi->origin, ldi)) {
      ldi = ldi->lnext();
    } else if (right_of(ldi->origin, rdi)) {
      rdi = rdi->rev()->onext;
    } else {
      break;
    }
  }
  Edge<T> *base = pool.connect(rdi->rev(), ldi);
  if (ldi->origin == ldo->origin) {
    ldo = base->rev();
  }
  if (rdi->origin == rdo->origin) {
    rdo = base;
  }
  while (true) {
    Edge<T> *lcand = base->rev()->onext;
    if (right_of(lcand->dest(), base)) {
      while (in_circle(base->dest(), base->origin, lcand->dest(), lcand->onext->dest())) {
        Edge<T> *next = lcand->onext;
        pool.delete_edge(lcand);
        lcand = next;
      }
    }
    Edge<T> *rcand = base->oprev();
    if (right_of(rcand->dest(), base)) {
      while (in_circle(base->dest(), base->origin, rcand->dest(), rcand->oprev()->dest())) {
        Edge<T> *prev = rcand->oprev();
        pool.delete_edge(rcand);
        rcand = prev;
      }
    }
    bool lvalid = right_of(lcand->dest(), base), rvalid = right_of(rcand->dest(), base);
    if (!lvalid && !rvalid) {
      break;
    }
    if (!lvalid ||
        (rvalid && in_circle(lcand->dest(), lcand->origin, rcand->origin, rcand->dest()))) {
      base = pool.connect(rcand, base->rev());
    } else {
      base = pool.connect(base->rev(), lcand->rev());
    }
  }
  return {ldo, rdo};
}

template<typename Pt>
auto delaunay_triangulation(std::vector<Pt> &p) {
  using T = decltype(Pt::x);
  using Triangle = std::tuple<Pt, Pt, Pt>;
  // Rotates triangle vertices so that the lexicographically smallest comes first, preserving the
  // cyclic (counterclockwise) order, to give each triangle a canonical, comparable representation.
  auto canonical = [](const Pt &a, const Pt &b, const Pt &c) -> Triangle {
    if (b < a && b < c) {
      return {b, c, a};
    }
    if (c < a && c < b) {
      return {c, a, b};
    }
    return {a, b, c};
  };
  std::sort(p.begin(), p.end());
  auto same_point = [](const Pt &a, const Pt &b) { return !(a < b) && !(b < a); };
  p.erase(std::unique(p.begin(), p.end(), same_point), p.end());
  std::vector<Triangle> res;
  if (p.size() < 3) {
    return res;
  }
  QuadEdgePool<T> pool;
  build_triangulation(p, 0, static_cast<int>(p.size()) - 1, pool);
  for (Edge<T> *start : pool.edges) {
    if (!start->primal || start->deleted || start->used) {
      continue;
    }
    std::vector<Pt> face;
    Edge<T> *curr = start;
    do {
      curr->used = true;
      face.push_back(curr->origin);
      curr = curr->lnext();
    } while (curr != start);
    if (face.size() == 3 && LT(0, cross(face[0], face[1], face[2]))) {
      res.push_back(canonical(face[0], face[1], face[2]));
    }
  }
  std::sort(res.begin(), res.end());
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

using PointL = Point<int64_t>;
using PointD = Point<double>;

int main() {
  vector<PointD> v{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
  vector<tuple<PointD, PointD, PointD>> t{
      {PointD{-1, 3}, PointD{0, 0}, PointD{1, 2}},
      {PointD{-1, 3}, PointD{1, 2}, PointD{1, 3}},
      {PointD{0, 0}, PointD{2, 1}, PointD{1, 2}},
      {PointD{1, 2}, PointD{2, 1}, PointD{1, 3}}
  };
  assert(delaunay_triangulation(v) == t);

  // Integer-coordinate inputs are triangulated using exact integer arithmetic.
  vector<PointL> iv{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
  vector<tuple<PointL, PointL, PointL>> ti{
      {PointL{-1, 3}, PointL{0, 0}, PointL{1, 2}},
      {PointL{-1, 3}, PointL{1, 2}, PointL{1, 3}},
      {PointL{0, 0}, PointL{2, 1}, PointL{1, 2}},
      {PointL{1, 2}, PointL{2, 1}, PointL{1, 3}}
  };
  assert(delaunay_triangulation(iv) == ti);
  return 0;
}
