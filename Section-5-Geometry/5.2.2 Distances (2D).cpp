#include <cmath>     /* fabs(), sqrt() */
#include <utility>   /* std::pair */

typedef std::pair<double, double> point;
#define x first
#define y second

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps)  /* equal to */
#define GE(a, b) ((a) >= (b) - eps)        /* greater than or equal to */
#define LE(a, b) ((a) <= (b) + eps)        /* less than or equal to */

double norm(const point & a) { return a.x * a.x + a.y * a.y; }
double abs(const point & a) { return sqrt(norm(a)); }

//distance and squared distance from point a to point b
double dist(const point & a, const point & b) {
  return abs(point(b.x - a.x, b.y - a.y));
}

double dist2(const point & a, const point & b) {
  return norm(point(b.x - a.x, b.y - a.y));
}

//minimum distance from point p to line l denoted by ax + by + c = 0
double dist(const point & p,
            const double & a, const double & b, const double & c) {
  return fabs(a * p.x + b * p.y + c) / abs(point(a, b));
}

//minimum distance from point p to line segment ab
double dist(const point & p, const point & a, const point & b) {
  if (a == b) return dist(p, a);
  point ab(b.x - a.x, b.y - a.y), ap(p.x - a.x, p.y - a.y);
  double n = norm(ab), d = ab.x * ap.x + ab.y * ap.y;
  if (LE(d, 0) || EQ(n, 0)) return abs(ap);
  if (GE(d, n)) return abs(point(ap.x - ab.x, ap.y - ab.y));
  return abs(point(ap.x - ab.x * (d / n), ap.y - ab.y * (d / n)));
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {

  return 0;
}
