#include <cmath>     /* fabs(), sqrt() */
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

#include <iostream>
using namespace std;

void print(const point & p) {
  cout << "(" << (fabs(p.x) < 1e-9 ? 0 : p.x)
       << "," << (fabs(p.y) < 1e-9 ? 0 : p.y) << ")\n";
}

int main() {
  cout << reduce_deg(-(8 * 360) + 123) << "\n";                     //123
  cout << reduce_rad(2 * PI * 8 + 1.2345) << "\n";                  //1.2345
  print(polar_point(4, PI));                                        //(-4,0)
  print(polar_point(4, -PI/2));                                     //(0,-4)
  cout << polar_angle(point(5, 5)) * 180/PI << "\n";                //45
  cout << polar_angle(point(-4, 4)) * 180/PI << "\n";               //135
  cout << angle(point(5,0), point(0,5), point(-5,0)) * RAD << "\n"; //90
  cout << angle_between(point(0, 5), point(5, -5)) * RAD << "\n";   //225
  //y=x and y=-x
  cout << angle_between(-1, 1, -1, -1) * 180/PI << "\n";            //90
  cout << cross(point(0,0), point(0,1), point(1,0)) << "\n";        //-1
  cout << turn(point(0,1), point(0,0), point(-5,-5)) << "\n";       //1
  return 0;
}
