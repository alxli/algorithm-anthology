/*

Given a list of points in two dimensions, find the circle with smallest area
which contains all the given points using a randomized algorithm.

- minimum_enclosing_circle(lo, hi) returns the minimum enclosing circle given a
  range [lo, hi) of points, where lo and hi must be RandomAccessIterators. The
  input range will be shuffled after the function call, though this is only to
  avoid the worst-case running time and is not necessary for correctness.

Time Complexity:
- O(n) on average per call to minimum_enclosing_circle(lo, hi), where n is the
  distance between lo and hi.

Space Complexity:
- O(1) auxiliary.

*/

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LE(a, b) ((a) <= (b) + EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double sqnorm(const point &a) { return a.x*a.x + a.y*a.y; }
double norm(const point &a) { return sqrt(sqnorm(a)); }

struct circle {
  double h, k, r;

  circle() : h(0), k(0), r(0) {}
  circle(double h, double k, double r) : h(h), k(k), r(fabs(r)) {}

  // Circle with the line segment ab as a diameter.
  circle(const point &a, const point &b) {
    h = (a.x + b.x)/2.0;
    k = (a.y + b.y)/2.0;
    r = norm(point(a.x - h, a.y - k));
  }

  // Circumcircle of three points.
  circle(const point &a, const point &b, const point &c) {
    double an = sqnorm(point(b.x - c.x, b.y - c.y));
    double bn = sqnorm(point(a.x - c.x, a.y - c.y));
    double cn = sqnorm(point(a.x - b.x, a.y - b.y));
    double wa = an*(bn + cn - an);
    double wb = bn*(an + cn - bn);
    double wc = cn*(an + bn - cn);
    double w = wa + wb + wc;
    if (EQ(w, 0)) {
      throw std::runtime_error("No circumcircle from collinear points.");
    }
    h = (wa*a.x + wb*b.x + wc*c.x)/w;
    k = (wa*a.y + wb*b.y + wc*c.y)/w;
    r = norm(point(a.x - h, a.y - k));
  }

  bool contains(const point &p) const {
    return LE(sqnorm(point(p.x - h, p.y - k)), r*r);
  }
};

template<class It>
circle minimum_enclosing_circle(It lo, It hi) {
  if (lo == hi) {
    return circle(0, 0, 0);
  }
  if (lo + 1 == hi) {
    return circle(lo->x, lo->y, 0);
  }
  std::random_shuffle(lo, hi);
  circle res(*lo, *(lo + 1));
  for (It i = lo + 2; i != hi; ++i) {
    if (res.contains(*i)) {
      continue;
    }
    res = circle(*lo, *i);
    for (It j = lo + 1; j != i; ++j) {
      if (res.contains(*j)) {
        continue;
      }
      res = circle(*i, *j);
      for (It k = lo; k != j; ++k) {
        if (!res.contains(*k)) {
          res = circle(*i, *j, *k);
        }
      }
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  vector<point> v;
  v.push_back(point(0, 0));
  v.push_back(point(0, 1));
  v.push_back(point(1, 0));
  v.push_back(point(1, 1));
  circle res = minimum_enclosing_circle(v.begin(), v.end());
  assert(EQ(res.h, 0.5) && EQ(res.k, 0.5) && EQ(res.r, 1/sqrt(2)));
  return 0;
}
