/*

Given a set $P$ of distinct two-dimensional points, a Delaunay triangulation of $P$ is a
triangulation of the convex hull of P such that no point of P lies strictly inside the circumcircle
of any triangle in the triangulation. The Delaunay triangulation is not necessarily unique when four
or more points are cocircular.

This implementation produces one valid triangulation using the Guibas-Stolfi divide-and-conquer
algorithm with a quad-edge data structure. The point set is split in half, each half is triangulated
recursively, and the halves are stitched together from the bottom up by a sequence of cross edges,
with circumcircle tests deciding each connecting edge and deleting invalidated ones. Input points
are copied to `Point` with `double` coordinates, sorted lexicographically, and duplicates are
removed.

- `delaunay_triangulation(lo, hi)` returns the triangles of one Delaunay triangulation for a range
  [`lo`, `hi`) of points. The input iterator value type may be any point type with numeric `.x` and
  `.y` members. The returned triangles use `Point` with `double` coordinates. Duplicate points are
  ignored. If fewer than three non-collinear unique points are available, the result is empty. Note
  that all predicates are evaluated using floating-point arithmetic, so results are subject to
  numerical error on nearly collinear or nearly cocircular inputs.

Time Complexity:
- O(n log n) per call to `delaunay_triangulation(lo, hi)`, where $n$ is the number of input points.

Space Complexity:
- O(n) heap space for the quad-edge structure and returned triangulation.

*/

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define GT(a, b) ((a) > (b) + EPS)

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

double cross(const Point &a, const Point &b, const Point &o = Point(0, 0)) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

long double incircle(const Point &a, const Point &b, const Point &c, const Point &d) {
  long double adx = a.x - d.x, ady = a.y - d.y;
  long double bdx = b.x - d.x, bdy = b.y - d.y;
  long double cdx = c.x - d.x, cdy = c.y - d.y;
  long double abdet = adx * bdy - bdx * ady;
  long double bcdet = bdx * cdy - cdx * bdy;
  long double cadet = cdx * ady - adx * cdy;
  long double alift = adx * adx + ady * ady;
  long double blift = bdx * bdx + bdy * bdy;
  long double clift = cdx * cdx + cdy * cdy;
  return alift * bcdet + blift * cadet + clift * abdet;
}

struct Edge {
  Point origin;
  Edge *rot, *onext;
  bool primal, deleted, used;

  Edge *rev() const { return rot->rot; }
  Edge *lnext() const { return rot->rev()->onext->rot; }
  Edge *oprev() const { return rot->onext->rot; }
  Point dest() const { return rev()->origin; }
};

struct QuadEdgePool {
  std::vector<Edge *> edges;

  QuadEdgePool() = default;

  ~QuadEdgePool() {
    for (Edge *e : edges) {
      delete e;
    }
  }

  QuadEdgePool(const QuadEdgePool &) = delete;
  QuadEdgePool &operator=(const QuadEdgePool &) = delete;

  Edge *make_edge(const Point &from, const Point &to) {
    Edge *e1 = new Edge, *e2 = new Edge, *e3 = new Edge, *e4 = new Edge;
    edges.push_back(e1);
    edges.push_back(e2);
    edges.push_back(e3);
    edges.push_back(e4);
    e1->origin = from;
    e2->origin = to;
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

  void delete_edge(Edge *e) {
    splice(e, e->oprev());
    splice(e->rev(), e->rev()->oprev());
    e->deleted = e->rot->deleted = e->rev()->deleted = e->rev()->rot->deleted = true;
  }

  Edge *connect(Edge *a, Edge *b) {
    Edge *e = make_edge(a->dest(), b->origin);
    splice(e, a->lnext());
    splice(e->rev(), b);
    return e;
  }

  static void splice(Edge *a, Edge *b) {
    std::swap(a->onext->rot->onext, b->onext->rot->onext);
    std::swap(a->onext, b->onext);
  }
};

bool left_of(const Point &p, Edge *e) {
  return GT(cross(e->origin, e->dest(), p), 0);
}

bool right_of(const Point &p, Edge *e) {
  return LT(cross(e->origin, e->dest(), p), 0);
}

bool in_circle(const Point &a, const Point &b, const Point &c, const Point &d) {
  return incircle(a, b, c, d) > EPS;
}

std::pair<Edge *, Edge *> build_triangulation(
    std::vector<Point> &p, int lo, int hi, QuadEdgePool &pool
) {
  if (hi - lo == 1) {
    Edge *a = pool.make_edge(p[lo], p[hi]);
    return {a, a->rev()};
  }
  if (hi - lo == 2) {
    Edge *a = pool.make_edge(p[lo], p[lo + 1]);
    Edge *b = pool.make_edge(p[lo + 1], p[hi]);
    pool.splice(a->rev(), b);
    int side = GT(cross(p[lo], p[lo + 1], p[hi]), 0)
                   ? 1
                   : (LT(cross(p[lo], p[lo + 1], p[hi]), 0) ? -1 : 0);
    if (side == 0) {
      return {a, b->rev()};
    }
    Edge *c = pool.connect(b, a);
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
  Edge *base = pool.connect(rdi->rev(), ldi);
  if (ldi->origin == ldo->origin) {
    ldo = base->rev();
  }
  if (rdi->origin == rdo->origin) {
    rdo = base;
  }
  while (true) {
    Edge *lcand = base->rev()->onext;
    if (right_of(lcand->dest(), base)) {
      while (in_circle(base->dest(), base->origin, lcand->dest(), lcand->onext->dest())) {
        Edge *next = lcand->onext;
        pool.delete_edge(lcand);
        lcand = next;
      }
    }
    Edge *rcand = base->oprev();
    if (right_of(rcand->dest(), base)) {
      while (in_circle(base->dest(), base->origin, rcand->dest(), rcand->oprev()->dest())) {
        Edge *prev = rcand->oprev();
        pool.delete_edge(rcand);
        rcand = prev;
      }
    }
    bool lvalid = right_of(lcand->dest(), base);
    bool rvalid = right_of(rcand->dest(), base);
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

struct Triangle {
  Point a, b, c;

  Triangle(const Point &a, const Point &b, const Point &c) : a(a), b(b), c(c) {
    if (b < a && b < c) {
      this->a = b;
      this->b = c;
      this->c = a;
    } else if (c < a && c < b) {
      this->a = c;
      this->b = a;
      this->c = b;
    }
  }

  bool operator==(const Triangle &t) const { return a == t.a && b == t.b && c == t.c; }

  bool operator<(const Triangle &t) const {
    return a != t.a ? a < t.a : (b != t.b ? b < t.b : c < t.c);
  }
};

// Accepts any point type with numeric .x/.y; inputs are copied to double Points.
template<class It>
std::vector<Triangle> delaunay_triangulation(It lo, It hi) {
  std::vector<Point> p;
  for (It it = lo; it != hi; ++it) {
    p.emplace_back(it->x, it->y);
  }
  std::sort(p.begin(), p.end());
  p.erase(std::unique(p.begin(), p.end()), p.end());
  if (p.size() < 3) {
    return std::vector<Triangle>();
  }
  QuadEdgePool pool;
  build_triangulation(p, 0, p.size() - 1, pool);
  std::vector<Triangle> res;
  for (Edge *start : pool.edges) {
    if (!start->primal || start->deleted || start->used) {
      continue;
    }
    std::vector<Point> face;
    Edge *curr = start;
    do {
      curr->used = true;
      face.push_back(curr->origin);
      curr = curr->lnext();
    } while (curr != start);
    if (face.size() == 3 && GT(cross(face[0], face[1], face[2]), 0)) {
      res.emplace_back(face[0], face[1], face[2]);
    }
  }
  std::sort(res.begin(), res.end());
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
};

int main() {
  vector<Point> v{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
  vector<Triangle> t{
      Triangle(Point(-1, 3), Point(0, 0), Point(1, 2)),
      Triangle(Point(-1, 3), Point(1, 2), Point(1, 3)),
      Triangle(Point(0, 0), Point(2, 1), Point(1, 2)),
      Triangle(Point(1, 2), Point(2, 1), Point(1, 3))
  };
  assert(delaunay_triangulation(v.begin(), v.end()) == t);

  // Integer-coordinate inputs are accepted (triangulation computed in double).
  vector<PointI> iv{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
  assert(delaunay_triangulation(iv.begin(), iv.end()) == t);
  return 0;
}
