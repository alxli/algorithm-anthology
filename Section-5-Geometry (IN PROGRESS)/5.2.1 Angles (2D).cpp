/*

5.2.1 - Angles (2D)

Angle calculations in 2 dimensions. All returned angles are in radians,
except for reduce_deg(). If x is an angle in radians, then you may use
x * DEG to convert x to degrees, and vice versa to radians with x * RAD.

All operations are O(1) in time and space.

*/

#include <cmath>     /* acos(), fabs(), sqrt(), atan2() */
#include <utility>   /* std::pair */

typedef std::pair<double, double> point;
#define x first
#define y second

const double PI = acos(-1.0), RAD = 180 / PI, DEG = PI / 180;

double abs(const point & a) { return sqrt(a.x * a.x + a.y * a.y); }

//reduce angles to the range [0, 360) degrees. e.g. reduce_deg(-630) = 90
double reduce_deg(const double & t) {
  if (t < -360) return reduce_deg(fmod(t, 360));
  if (t < 0) return t + 360;
  return t >= 360 ? fmod(t, 360) : t;
}

//reduce angles to the range [0, 2*pi) radians. e.g. reduce_rad(720.5) = 0.5
double reduce_rad(const double & t) {
  if (t < -2 * PI) return reduce_rad(fmod(t, 2 * PI));
  if (t < 0) return t + 2 * PI;
  return t >= 2 * PI ? fmod(t, 2 * PI) : t;
}

//like std::polar(), but returns a point instead of an std::complex
point polar_point(const double & r, const double & theta) {
  return point(r * cos(theta), r * sin(theta));
}

//angle of segment (0, 0) to p, relative (CCW) to the +'ve x-axis in radians
double polar_angle(const point & p) {
  double t = atan2(p.y, p.x);
  return t < 0 ? t + 2 * PI : t;
}

//smallest angle formed by points aob (angle is at point o) in radians
double angle(const point & a, const point & o, const point & b) {
  point u(o.x - a.x, o.y - a.y), v(o.x - b.x, o.y - b.y);
  return acos((u.x * v.x + u.y * v.y) / (abs(u) * abs(v)));
}

//angle of line segment ab relative (CCW) to the +'ve x-axis in radians
double angle_between(const point & a, const point & b) {
  double t = atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
  return t < 0 ? t + 2 * PI : t;
}

//Given the A, B values of two lines in Ax + By + C = 0 form, finds the
//minimum angle in radians between the two lines in the range [0, PI/2]
double angle_between(const double & a1, const double & b1,
                     const double & a2, const double & b2) {
  double t = atan2(a1 * b2 - a2 * b1, a1 * a2 + b1 * b2);
  if (t < 0) t += PI; //force angle to be positive
  if (t > PI / 2) t = PI - t; //force angle to be <= 90 degrees
  return t;
}

//magnitude of the 3D cross product with Z component implicitly equal to 0
//the answer assumes the origin (0, 0) is instead shifted to point o.
//this is equal to 2x the signed area of the triangle from these 3 points.
double cross(const point & o, const point & a, const point & b) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

//does the path a->o->b form:
// -1 ==> a left turn on the plane?
//  0 ==> a single straight line segment? (i.e. are a,o,b collinear?) or
// +1 ==> a right turn on the plane?
int turn(const point & a, const point & o, const point & b) {
  double c = cross(o, a, b);
  return c < 0 ? -1 : (c > 0 ? 1 : 0);
}

/*** Example Usage ***/

#include <cassert>
#define pt point
#define EQ(a, b) (fabs((a) - (b)) <= 1e-9)

int main() {
  assert(EQ(123,    reduce_deg(-(8 * 360) + 123)));
  assert(EQ(1.2345, reduce_rad(2 * PI * 8 + 1.2345)));
  point p = polar_point(4, PI), q = polar_point(4, -PI / 2);
  assert(EQ(p.x, -4) && EQ(p.y, 0));
  assert(EQ(q.x, 0) && EQ(q.y, -4));
  assert(EQ(45,  polar_angle(pt(5, 5)) * RAD));
  assert(EQ(135, polar_angle(pt(-4, 4)) * RAD));
  assert(EQ(90,  angle(pt(5, 0), pt(0, 5), pt(-5, 0)) * RAD));
  assert(EQ(225, angle_between(pt(0, 5), pt(5, -5)) * RAD));
  assert(EQ(90,  angle_between(-1, 1, -1, -1) * RAD)); //y=x and y=-x
  assert(-1 == cross(pt(0, 0), pt(0, 1), pt(1, 0)));
  assert(+1 == turn(pt(0, 1), pt(0, 0), pt(-5, -5)));
  return 0;
}
