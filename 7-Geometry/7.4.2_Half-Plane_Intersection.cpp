/*

Given a list of directed lines, compute the convex polygon contained in every left half-plane.
Half-plane intersection sorts the lines by angle, keeps only the tightest representative among
parallel lines, and maintains a deque of lines whose pairwise intersections form the current
boundary.

- `half_plane_intersection(planes)` returns the polygon cut out by the half-planes in
  counter-clockwise order. Each half-plane is represented by `HalfPlane(p, q)`, meaning the closed
  region to the left of the directed line `p` $\to$ `q`.
- If the intersection is empty, the function returns an empty vector.
- If the true intersection is unbounded, the returned polygon is not meaningful. Add explicit
  bounding-box half-planes when a bounded polygon is required.

Time Complexity:
- O(n log n), where $n$ is the number of half-planes.

Space Complexity:
- O(n) auxiliary.

*/

#include <algorithm>
#include <cmath>
#include <deque>
#include <type_traits>
#include <vector>

const double EPS = 1e-9;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  return std::is_integral_v<C> ? C(a) == C(b) : std::fabs(C(a) - C(b)) <= static_cast<C>(EPS);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool LT(T a, U b) {
  return std::is_integral_v<C> ? C(a) < C(b) : C(a) < C(b) - static_cast<C>(EPS);
}

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
  Point operator+(const Point &p) const { return {x + p.x, y + p.y}; }
  Point operator-(const Point &p) const { return {x - p.x, y - p.y}; }
  Point operator*(double k) const { return {x * k, y * k}; }
  double cross(const Point &p) const { return x * p.y - y * p.x; }
};

struct HalfPlane {
  Point p, dir;
  double ang;

  HalfPlane() : p(), dir(), ang(0) {}
  HalfPlane(Point p, Point q) : p(p), dir(q - p), ang(atan2(dir.y, dir.x)) {}

  bool out(const Point &q) const { return LT(dir.cross(q - p), 0); }
  Point intersection(const HalfPlane &h) const {
    double t = h.dir.cross(p - h.p) / dir.cross(h.dir);
    return p + dir * t;
  }
  bool operator<(const HalfPlane &h) const {
    if (!EQ(ang, h.ang)) {
      return ang < h.ang;
    }
    return dir.cross(h.p - p) < 0;  // tighter half-plane first among parallel lines
  }
};

std::vector<Point> half_plane_intersection(std::vector<HalfPlane> planes) {
  std::sort(planes.begin(), planes.end());
  std::vector<HalfPlane> unique;
  for (const HalfPlane &h : planes) {
    if (!unique.empty() && EQ(unique.back().dir.cross(h.dir), 0)) {
      continue;
    }
    unique.push_back(h);
  }
  std::deque<HalfPlane> dq;
  auto bad_back = [&](const HalfPlane &h) {
    return dq.size() >= 2 && h.out(dq[dq.size() - 2].intersection(dq.back()));
  };
  auto bad_front = [&](const HalfPlane &h) {
    return dq.size() >= 2 && h.out(dq[0].intersection(dq[1]));
  };
  for (const HalfPlane &h : unique) {
    while (bad_back(h)) {
      dq.pop_back();
    }
    while (bad_front(h)) {
      dq.pop_front();
    }
    dq.push_back(h);
  }
  while (dq.size() >= 3 && dq.front().out(dq[dq.size() - 2].intersection(dq.back()))) {
    dq.pop_back();
  }
  while (dq.size() >= 3 && dq.back().out(dq[0].intersection(dq[1]))) {
    dq.pop_front();
  }
  if (dq.size() < 3) {
    return {};
  }
  std::vector<Point> res;
  for (int i = 0; i < static_cast<int>(dq.size()); i++) {
    if (EQ(dq[i].dir.cross(dq[(i + 1) % dq.size()].dir), 0)) {
      return {};
    }
    res.push_back(dq[i].intersection(dq[(i + 1) % dq.size()]));
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

double polygon_area(const vector<Point> &p) {
  double area = 0;
  for (int i = 0, j = static_cast<int>(p.size()) - 1; i < static_cast<int>(p.size()); j = i++) {
    area += p[j].cross(p[i]);
  }
  return fabs(area) / 2.0;
}

int main() {
  vector<HalfPlane> box{
      HalfPlane(Point(0, 0), Point(4, 0)),
      HalfPlane(Point(4, 0), Point(4, 4)),
      HalfPlane(Point(4, 4), Point(0, 4)),
      HalfPlane(Point(0, 4), Point(0, 0)),
  };
  auto square = half_plane_intersection(box);
  assert(square.size() == 4);
  assert(EQ(polygon_area(square), 16));

  box.push_back(HalfPlane(Point(2, 0), Point(2, 4)));  // x <= 2
  auto rect = half_plane_intersection(box);
  assert(rect.size() == 4);
  assert(EQ(polygon_area(rect), 8));

  vector<HalfPlane> empty{
      HalfPlane(Point(0, 0), Point(1, 0)),  // y >= 0
      HalfPlane(Point(1, 1), Point(0, 1)),  // y <= 1
      HalfPlane(Point(0, 2), Point(1, 2)),  // y >= 2
  };
  assert(half_plane_intersection(empty).empty());
  return 0;
}
