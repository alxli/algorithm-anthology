/*

Given a set of points with integer coordinates, compute the maximum number of points that lie on
one line. For every anchor point, normalize the direction vector to every other point by dividing
by `gcd(abs(dx), abs(dy))` and choosing a canonical sign; equal normalized directions from the same
anchor are collinear with it.

- `max_collinear_points(points)` returns the largest number of collinear points. Duplicate points
  are counted as separate input points.

Time Complexity:
- O(n^2 log n) per call, where $n$ is the number of points.

Space Complexity:
- O(n) auxiliary.

*/

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <map>
#include <numeric>
#include <utility>
#include <vector>

struct Point {
  int64_t x, y;
  Point(int64_t x = 0, int64_t y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
};

int max_collinear_points(const std::vector<Point> &p) {
  int n = static_cast<int>(p.size()), ans = std::min(n, 1);
  for (int i = 0; i < n; i++) {
    std::map<std::pair<int64_t, int64_t>, int> cnt;
    int duplicates = 0, best = 0;
    for (int j = 0; j < n; j++) {
      if (i == j) {
        continue;
      }
      int64_t dx = p[j].x - p[i].x, dy = p[j].y - p[i].y;
      if (dx == 0 && dy == 0) {
        duplicates++;
        continue;
      }
      int64_t g = std::gcd(std::abs(dx), std::abs(dy));
      dx /= g;
      dy /= g;
      if (dx < 0 || (dx == 0 && dy < 0)) {
        dx = -dx;
        dy = -dy;
      }
      best = std::max(best, ++cnt[{dx, dy}]);
    }
    ans = std::max(ans, best + duplicates + 1);
  }
  return ans;
}

/*** Example Usage ***/

using namespace std;

int main() {
  vector<Point> p{{0, 0}, {1, 1}, {2, 2}, {3, 5}, {4, 6}, {3, 3}};
  assert(max_collinear_points(p) == 4);
  p.push_back(Point(1, 1));
  assert(max_collinear_points(p) == 5);
  assert(max_collinear_points({}) == 0);
  assert(max_collinear_points({Point(2, 3)}) == 1);
  return 0;
}
