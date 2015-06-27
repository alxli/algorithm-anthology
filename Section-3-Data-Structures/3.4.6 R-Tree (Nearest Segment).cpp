/*

3.4.6 - R-Tree for Nearest Neighbouring Line Segment Queries

Description: R-trees are tree data structures used for spatial
access methods, i.e., for indexing multi-dimensional information
such as geographical coordinates, rectangles or polygons. The
following implementation supports querying of the nearing line
segment to a point after a tree of line segments have been built.

Time Complexity: O(N log N) for build(), where N is the number of
points in the tree. nearest_neighbor_id() is O(log(N)) on average.

Space Complexity: O(N) on the number of points.

*/

#include <algorithm> /* nth_element(), max(), min(), swap() */
#include <cfloat>    /* DBL_MAX */
#include <climits>   /* INT_MIN, INT_MAX */
#include <vector>

struct segment { int x1, y1, x2, y2; };

class r_tree {

  static inline bool cmp_x(const segment & a, const segment & b) {
    return a.x1 + a.x2 < b.x1 + b.x2;
  }

  static inline bool cmp_y(const segment & a, const segment & b) {
    return a.y1 + a.y2 < b.y1 + b.y2;
  }

  std::vector<segment> s;
  std::vector<int> minx, maxx, miny, maxy;

  void build(int lo, int hi, bool div_x, segment s[]) {
    if (lo >= hi) return;
    int mid = (lo + hi) >> 1;
    std::nth_element(s + lo, s + mid, s + hi, div_x ? cmp_x : cmp_y);
    this->s[mid] = s[mid];
    for (int i = lo; i < hi; i++) {
      minx[mid] = std::min(minx[mid], std::min(s[i].x1, s[i].x2));
      miny[mid] = std::min(miny[mid], std::min(s[i].y1, s[i].y2));
      maxx[mid] = std::max(maxx[mid], std::max(s[i].x1, s[i].x2));
      maxy[mid] = std::max(maxy[mid], std::max(s[i].y1, s[i].y2));
    }
    build(lo, mid, !div_x, s);
    build(mid + 1, hi, !div_x, s);
  }

  double min_dist;
  int min_dist_id, x, y;

  void nearest_neighbor(int lo, int hi, bool div_x) {
    if (lo >= hi) return;
    int mid = (lo + hi) >> 1;
    double pdist = point_to_segment_squared(x, y, s[mid]);
    if (min_dist > pdist) {
      min_dist = pdist;
      min_dist_id = mid;
    }
    long long delta = div_x ? 2*x - s[mid].x1 - s[mid].x2 :
                              2*y - s[mid].y1 - s[mid].y2;
    if (delta <= 0) {
      nearest_neighbor(lo, mid, !div_x);
      if (mid + 1 < hi) {
        int mid1 = (mid + hi + 1) >> 1;
        long long dist = div_x ? seg_dist(x, minx[mid1], maxx[mid1]) :
                                 seg_dist(y, miny[mid1], maxy[mid1]);
        if (dist*dist < min_dist) nearest_neighbor(mid + 1, hi, !div_x);
      }
    } else {
      nearest_neighbor(mid + 1, hi, !div_x);
      if (lo < mid) {
        int mid1 = (lo + mid) >> 1;
        long long dist = div_x ? seg_dist(x, minx[mid1], maxx[mid1]) :
                                 seg_dist(y, miny[mid1], maxy[mid1]);
        if (dist*dist < min_dist) nearest_neighbor(lo, mid, !div_x);
      }
    }
  }

  static double point_to_segment_squared(int x, int y, const segment & s) {
    long long dx = s.x2 - s.x1, dy = s.y2 - s.y1;
    long long px = x - s.x1, py = y - s.y1;
    long long square_dist = dx*dx + dy*dy;
    long long dot_product = dx*px + dy*py;
    if (dot_product <= 0 || square_dist == 0) return px*px + py*py;
    if (dot_product >= square_dist)
      return (px - dx)*(px - dx) + (py - dy)*(py - dy);
    double q = (double)dot_product/square_dist;
    return (px - q*dx)*(px - q*dx) + (py - q*dy)*(py - q*dy);
  }

  static inline int seg_dist(int v, int lo, int hi) {
    return v <= lo ? lo - v : (v >= hi ? v - hi : 0);
  }

 public:
  r_tree(int N, segment s[]) {
    this->s.resize(N);
    minx.assign(N, INT_MAX);
    maxx.assign(N, INT_MIN);
    miny.assign(N, INT_MAX);
    maxy.assign(N, INT_MIN);
    build(0, N, true, s);
  }

  int nearest_neighbor_id(int x, int y) {
    min_dist = DBL_MAX;
    this->x = x; this->y = y;
    nearest_neighbor(0, s.size(), true);
    return min_dist_id;
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  segment s[4];
  s[0] = (segment){0, 0, 0, 4};
  s[1] = (segment){0, 4, 4, 4};
  s[2] = (segment){4, 4, 4, 0};
  s[3] = (segment){4, 0, 0, 0};
  r_tree t(4, s);
  int id = t.nearest_neighbor_id(-1, 2);
  cout << s[id].x1 << " " << s[id].y1 << " " <<
          s[id].x2 << " " << s[id].y2 << "\n"; //0 0 0 4
  return 0;
}
