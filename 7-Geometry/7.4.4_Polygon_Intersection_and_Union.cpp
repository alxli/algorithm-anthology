/*

Given two simple (non-self-intersecting) polygons, determine the areas of their intersection and
union using a sweep line algorithm and the inclusion-exclusion principle. Every vertex and every
pairwise edge intersection contributes its $x$-coordinate as a sweep boundary, so within each
vertical slab between consecutive boundaries the two borders cannot cross, and the slab's overlap is
accumulated exactly from trapezoid areas. The union then follows by inclusion-exclusion as the sum
of the individual areas minus the intersection.

- `intersection_area(a, b)` returns the intersection area of polygons `a` and `b`, whose vertices
  are listed in boundary order.
- `union_area(a, b)` returns the union area of polygons `a` and `b`.

Overflow warning: the segment-intersection tests form cross products in the input point's coordinate
type, which grow like the squared coordinate magnitude. For integral point types, use 64-bit
coordinates (e.g. `PointL` from 7.1.1) if necessary.

Time Complexity:
- O(n*m*(n + m)*log(n + m)) per call to `intersection_area()` and `union_area()`, where $n$ is the
  number of vertices in the first polygon and $m$ is the number of vertices in the second polygon
  (or O(N^3 log N) for $N = n + m$).

Space Complexity:
- O(n*m) auxiliary for `intersection_area()` and `union_area()`, where $n$ and $m$ are the
  respective polygon sizes.

*/

#include <algorithm>
#include <cmath>
#include <type_traits>
#include <utility>
#include <vector>

const double EPS = 1e-9;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) == C(b) || std::fabs(C(a) - C(b)) <= EPS;
  return C(a) == C(b);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool LT(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) < C(b) - EPS;
  return C(a) < C(b);
}

template<typename T, typename U>
bool LE(T a, U b) {
  return !LT(b, a);
}

template<typename Pt>
bool point_on_segment(const Pt &p, const Pt &a, const Pt &b) {
  // Overflow risk for integer Pt: these products are ~O(max_coord^2); use int64_t if necessary.
  return EQ((p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x), 0) &&
         LE(std::min(a.x, b.x), p.x) && LE(p.x, std::max(a.x, b.x)) &&
         LE(std::min(a.y, b.y), p.y) && LE(p.y, std::max(a.y, b.y));
}

// Specialized version of seg_intersection() from 7.2.3, simplified for include_boundary = true,
// returning -1 for no intersection, 0 for one intersection point, 1 for positive length overlap.
template<typename Pt>
int seg_intersection1(
    const Pt &a, const Pt &b, const Pt &c, const Pt &d, double *outx, double *outy
) {
  auto ab_x = b.x - a.x, ab_y = b.y - a.y;
  auto ac_x = c.x - a.x, ac_y = c.y - a.y;
  auto cd_x = d.x - c.x, cd_y = d.y - c.y;
  auto ab2 = ab_x * ab_x + ab_y * ab_y;
  auto cd2 = cd_x * cd_x + cd_y * cd_y;
  if (EQ(ab2, 0)) {
    if (point_on_segment(a, c, d)) {
      *outx = a.x;
      *outy = a.y;
      return 0;
    }
    return -1;
  }
  if (EQ(cd2, 0)) {
    if (point_on_segment(c, a, b)) {
      *outx = c.x;
      *outy = c.y;
      return 0;
    }
    return -1;
  }
  auto c1 = ab_x * cd_y - ab_y * cd_x;
  auto c2 = ac_x * ab_y - ac_y * ab_x;
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    Pt res1 = std::max(std::min(a, b), std::min(c, d));
    Pt res2 = std::min(std::max(a, b), std::max(c, d));
    if (!(res2 < res1)) {
      if (res1 == res2) {
        *outx = static_cast<double>(res1.x);
        *outy = static_cast<double>(res1.y);
        return 0;
      }
      return 1;
    }
    return -1;
  }
  if (EQ(c1, 0)) {
    return -1;
  }
  auto t_num = ac_x * cd_y - ac_y * cd_x;
  bool c1_pos = c1 > 0;
  bool t_ok = c1_pos ? (LE(0, t_num) && LE(t_num, c1)) : (LE(c1, t_num) && LE(t_num, 0));
  bool u_ok = c1_pos ? (LE(0, c2) && LE(c2, c1)) : (LE(c1, c2) && LE(c2, 0));
  if (t_ok && u_ok) {
    double t = static_cast<double>(t_num) / c1;
    *outx = static_cast<double>(a.x + t * ab_x);
    *outy = static_cast<double>(a.y + t * ab_y);
    return 0;
  }
  return -1;
}

// The two segments may use different point types (e.g. polygon edge vs. sweep line).
template<typename PtA, typename PtB>
int line_intersection1(
    const PtA &p1, const PtA &p2, const PtB &p3, const PtB &p4, double *outx, double *outy
) {
  double a1 = static_cast<double>(p2.y) - p1.y, b1 = static_cast<double>(p1.x) - p2.x;
  double c1 = -(a1 * p1.x + b1 * p1.y);
  double a2 = static_cast<double>(p4.y) - p3.y, b2 = static_cast<double>(p3.x) - p4.x;
  double c2 = -(a2 * p3.x + b2 * p3.y);
  double x = -(c1 * b2 - c2 * b1), y = -(a1 * c2 - a2 * c1);
  double det = a1 * b2 - a2 * b1;
  if (EQ(det, 0)) {
    return (EQ(x, 0) && EQ(y, 0)) ? 1 : -1;
  }
  if (outx != nullptr && outy != nullptr) {
    *outx = x / det;
    *outy = y / det;
  }
  return 0;
}

template<typename Pt>
double intersection_area(const std::vector<Pt> &a, const std::vector<Pt> &b) {
  if (a.empty() || b.empty()) {
    return 0;
  }
  struct SweepPoint {
    double x, y;
  };  // For line intersection with the sweep line.
  const std::vector<Pt> *polys[2] = {&a, &b};
  int orientation[2];
  for (int id = 0; id < 2; id++) {
    const auto &p = *polys[id];
    int n = static_cast<int>(p.size());
    double area = 0;
    for (int i = 0, j = n - 1; i < n; j = i++) {
      area += (static_cast<double>(p[j].x) - p[i].x) * (static_cast<double>(p[j].y) + p[i].y);
    }
    orientation[id] = (area < 0 ? 1 : (area > 0 ? -1 : 0));
  }
  std::vector<double> x_coords;
  for (const Pt &p : a) {
    x_coords.push_back(p.x);
  }
  for (const Pt &p : b) {
    x_coords.push_back(p.x);
  }
  int n = static_cast<int>(a.size()), m = static_cast<int>(b.size());
  for (int i1 = 0, j1 = n - 1; i1 < n; j1 = i1++) {
    for (int i2 = 0, j2 = m - 1; i2 < m; j2 = i2++) {
      double outx, outy;
      if (seg_intersection1(a[i1], a[j1], b[i2], b[j2], &outx, &outy) == 0) {
        x_coords.push_back(outx);
      }
    }
  }
  std::sort(x_coords.begin(), x_coords.end());
  x_coords.erase(
      std::unique(x_coords.begin(), x_coords.end(), [](double a, double b) { return EQ(a, b); }),
      x_coords.end()
  );
  double res = 0;
  for (int k = 0; k + 1 < static_cast<int>(x_coords.size()); k++) {
    double x = (x_coords[k] + x_coords[k + 1]) / 2;
    SweepPoint sweep0{x, 0}, sweep1{x, 1};
    std::vector<std::pair<double, int>> events;  // (y, mask delta)
    for (int id = 0; id < 2; id++) {
      const auto &p = *polys[id];
      int size = static_cast<int>(p.size());
      for (int j = 0, i = size - 1; j < size; i = j++) {
        double px, py;
        if (line_intersection1(p[j], p[i], sweep0, sweep1, &px, &py) == 0) {
          double y = py, x0 = p[i].x, x1 = p[j].x;
          if (x0 < x && x1 > x) {
            events.emplace_back(y, orientation[id] * (1 << id));
          } else if (x0 > x && x1 < x) {
            events.emplace_back(y, -orientation[id] * (1 << id));
          }
        }
      }
    }
    std::sort(events.begin(), events.end());
    double height = 0;
    int coverage[2] = {0, 0};
    for (int j = 0; j < static_cast<int>(events.size()); j++) {
      auto [y, mask_delta] = events[j];
      if (coverage[0] != 0 && coverage[1] != 0) {
        height += y - events[j - 1].first;
      }
      int bit = std::abs(mask_delta);
      int poly = (bit == 1 ? 0 : 1);
      coverage[poly] += mask_delta / bit;
    }
    res += height * (x_coords[k + 1] - x_coords[k]);
  }
  return res;
}

template<typename Pt>
double polygon_area(const std::vector<Pt> &p) {
  if (p.empty()) {
    return 0;
  }
  int n = static_cast<int>(p.size());
  double area = 0;
  for (int i = 0, j = n - 1; i < n; j = i++) {
    area += (static_cast<double>(p[j].x) - p[i].x) * (static_cast<double>(p[j].y) + p[i].y);
  }
  return fabs(area / 2.0);
}

template<typename Pt>
double union_area(const std::vector<Pt> &a, const std::vector<Pt> &b) {
  return polygon_area(a) + polygon_area(b) - intersection_area(a, b);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
  bool operator==(const PointI &o) const { return x == o.x && y == o.y; }
  bool operator!=(const PointI &p) const { return !(*this == p); }
  bool operator<(const PointI &p) const { return x != p.x ? x < p.x : y < p.y; }
};

int main() {
  vector<Point> p, s;
  // Irregular pentagon overlapping the square below in a triangle of area 1.5.
  p.emplace_back(1, 3);
  p.emplace_back(1, 2);
  p.emplace_back(2, 1);
  p.emplace_back(0, 0);
  p.emplace_back(-1, 3);
  // Square of area 12.5 in quadrant 2.
  s.emplace_back(0, 0);
  s.emplace_back(0, 3);
  s.emplace_back(-3, 3);
  s.emplace_back(-3, 0);
  assert(EQ(1.5, intersection_area(p, s)));
  assert(EQ(12.5, union_area(p, s)));

  // Integer-coordinate polygons are accepted (computation proceeds in double).
  vector<PointI> ip{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
  vector<PointI> is{{0, 0}, {0, 3}, {-3, 3}, {-3, 0}};
  assert(EQ(1.5, intersection_area(ip, is)));
  assert(EQ(12.5, union_area(ip, is)));
  return 0;
}
