/*

Maintain a set of two-dimensional line segments while supporting queries for the
closest segment in the set to a given query point. This implementation uses
integer points and long doubles for intermediate calculations.

- r_tree(lo, hi) constructs a set from two RandomAccessIterators as a range
  [lo, hi) of segments.
- nearest(x, y) returns a segment in the set that contains some point which is
  as close or closer to (x, y) by Euclidean distance than any point on any
  other segment in the set.

Time Complexity:
- O(n log n) per call to the constructor, where n is the number of segments.
- O(log n) on average per call to nearest().

Space Complexity:
- O(n) for storage of the segments.
- O(log n) auxiliary stack space for nearest().

*/

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>

struct segment {
  int x1, y1, x2, y2;

  segment() : x1(0), y1(0), x2(0), y2(0) {}
  segment(int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}

  bool operator==(const segment &s) const {
    return (x1 == s.x1) && (y1 == s.y1) && (x2 == s.x2) && (y2 == s.y2);
  }
};

class r_tree {
  static inline bool cmp_x(const segment &a, const segment &b) {
    return a.x1 + a.x2 < b.x1 + b.x2;
  }

  static inline bool cmp_y(const segment &a, const segment &b) {
    return a.y1 + a.y2 < b.y1 + b.y2;
  }

  static inline int seg_dist(int v, int lo, int hi) {
    return (v <= lo) ? (lo - v) : (v >= hi ? v - hi : 0);
  }

  static long double point_to_segment_squared(int x, int y, const segment &s) {
    long long dx = s.x2 - s.x1, dy = s.y2 - s.y1;
    long long px = x - s.x1, py = y - s.y1;
    long long sqdist = dx*dx + dy*dy;
    long long dot = dx*px + dy*py;
    if (dot <= 0 || sqdist == 0) {
      return px*px + py*py;
    }
    if (dot >= sqdist) {
      return (px - dx)*(px - dx) + (py - dy)*(py - dy);
    }
    double q = (double)dot / sqdist;
    return (px - q*dx)*(px - q*dx) + (py - q*dy)*(py - q*dy);
  }

  std::vector<segment> tree;
  std::vector<int> minx, maxx, miny, maxy;

  void build(int lo, int hi, bool div_x) {
    if (lo >= hi) {
      return;
    }
    int mid = lo + (hi - lo)/2;
    std::nth_element(tree.begin() + lo, tree.begin() + mid, tree.begin() + hi,
                     div_x ? cmp_x : cmp_y);
    for (int i = lo; i < hi; i++) {
      minx[mid] = std::min(minx[mid], std::min(tree[i].x1, tree[i].x2));
      miny[mid] = std::min(miny[mid], std::min(tree[i].y1, tree[i].y2));
      maxx[mid] = std::max(maxx[mid], std::max(tree[i].x1, tree[i].x2));
      maxy[mid] = std::max(maxy[mid], std::max(tree[i].y1, tree[i].y2));
    }
    build(lo, mid, !div_x);
    build(mid + 1, hi, !div_x);
  }

  // Helper variables for nearest().
  double min_dist;
  int id;

  void nearest(int lo, int hi, int x, int y, bool div_x) {
    if (lo >= hi) {
      return;
    }
    int mid = lo + (hi - lo)/2;
    long double d = point_to_segment_squared(x, y, tree[mid]);
    if (min_dist > d) {
      min_dist = d;
      id = mid;
    }
    long long delta = div_x ? (2*x - tree[mid].x1 - tree[mid].x2)
                            : (2*y - tree[mid].y1 - tree[mid].y2);
    if (delta <= 0) {
      nearest(lo, mid, x, y, !div_x);
      if (mid + 1 < hi) {
        int mid1 = (mid + hi + 1)/2;
        long long dist = div_x ? seg_dist(x, minx[mid1], maxx[mid1])
                               : seg_dist(y, miny[mid1], maxy[mid1]);
        if (dist*dist < min_dist) {
          nearest(mid + 1, hi, x, y, !div_x);
        }
      }
    } else {
      nearest(mid + 1, hi, x, y, !div_x);
      if (lo < mid) {
        int mid1 = lo + (mid - lo)/2;
        long long dist = div_x ? seg_dist(x, minx[mid1], maxx[mid1]) :
                                 seg_dist(y, miny[mid1], maxy[mid1]);
        if (dist*dist < min_dist) {
          nearest(lo, mid, x, y, !div_x);
        }
      }
    }
  }

 public:
  template<class It>
  r_tree(It lo, It hi) : tree(lo, hi) {
    int n = std::distance(lo, hi);
    if (n <= 1) {
      throw std::runtime_error("R-tree must be have at least 2 segments.");
    }
    minx.assign(n, std::numeric_limits<int>::max());
    maxx.assign(n, std::numeric_limits<int>::min());
    miny.assign(n, std::numeric_limits<int>::max());
    maxy.assign(n, std::numeric_limits<int>::min());
    build(0, n, true);
  }

  segment nearest(int x, int y) {
    min_dist = std::numeric_limits<long double>::max();
    nearest(0, tree.size(), x, y, true);
    return tree[id];
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  segment s[4];
  s[0] = segment(0, 0, 0, 4);
  s[1] = segment(0, 4, 4, 4);
  s[2] = segment(4, 4, 4, 0);
  s[3] = segment(4, 0, 0, 0);
  r_tree t(s, s + 4);
  assert(t.nearest(-1, 2) == segment(0, 0, 0, 4));
  assert(t.nearest(100, 100) == segment(4, 4, 4, 0));
  return 0;
}
