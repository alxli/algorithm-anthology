/*

5.1.1 - 2D Point Class

This class is very similar to std::complex, except it uses epsilon
comparisons and also supports other operations such as reflection
and rotation. All operations are O(1) in space and time.

*/

#include <cmath>   /* atan(), fabs(), sqrt() */
#include <ostream>
#include <utility> /* std::pair */

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define LT(a, b) ((a) < (b) - eps)        /* less than */

struct point {

  double x, y;

  point() : x(0), y(0) {}
  point(const point & p) : x(p.x), y(p.y) {}
  point(const std::pair<double, double> & p) : x(p.first), y(p.second) {}
  point(const double & a, const double & b) : x(a), y(b) {}

  bool operator < (const point & p) const {
    return EQ(x, p.x) ? LT(y, p.y) : LT(x, p.x);
  }

  bool operator > (const point & p) const {
    return EQ(x, p.x) ? LT(p.y, y) : LT(p.x, x);
  }

  bool operator == (const point & p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator != (const point & p) const { return !(*this == p); }
  bool operator <= (const point & p) const { return !(*this > p); }
  bool operator >= (const point & p) const { return !(*this < p); }
  point operator + (const point & p) const { return point(x + p.x, y + p.y); }
  point operator - (const point & p) const { return point(x - p.x, y - p.y); }
  point operator + (const double & v) const { return point(x + v, y + v); }
  point operator - (const double & v) const { return point(x - v, y - v); }
  point operator * (const double & v) const { return point(x * v, y * v); }
  point operator / (const double & v) const { return point(x / v, y / v); }
  point & operator += (const point & p) { x += p.x; y += p.y; return *this; }
  point & operator -= (const point & p) { x -= p.x; y -= p.y; return *this; }
  point & operator += (const double & v) { x += v; y += v; return *this; }
  point & operator -= (const double & v) { x -= v; y -= v; return *this; }
  point & operator *= (const double & v) { x *= v; y *= v; return *this; }
  point & operator /= (const double & v) { x /= v; y /= v; return *this; }
  friend point operator + (const double & v, const point & p) { return p + v; }
  friend point operator * (const double & v, const point & p) { return p * v; }

  double norm() const { return x * x + y * y; }
  double abs() const { return sqrt(x * x + y * y); }
  double arg() const { return atan2(y, x); }
  double dot(const point & p) const { return x * p.x + y * p.y; }
  double cross(const point & p) const { return x * p.y - y * p.x; }
  double proj(const point & p) const { return dot(p) / p.abs(); } //onto p
  point rot90() const { return point(-y, x); }

  //proportional unit vector of (x, y) such that x^2 + y^2 = 1
  point normalize() const {
    return (EQ(x, 0) && EQ(y, 0)) ? point(0, 0) : (point(x, y) / abs());
  }

  //rotate t radians CW about origin
  point rotateCW(const double & t) const {
    return point(x * cos(t) + y * sin(t), y * cos(t) - x * sin(t));
  }

  //rotate t radians CCW about origin
  point rotateCCW(const double & t) const {
    return point(x * cos(t) - y * sin(t), x * sin(t) + y * cos(t));
  }

  //rotate t radians CW about point p
  point rotateCW(const point & p, const double & t) const {
    return (*this - p).rotateCW(t) + p;
  }

  //rotate t radians CCW about point p
  point rotateCCW(const point & p, const double & t) const {
    return (*this - p).rotateCCW(t) + p;
  }

  //reflect across point p
  point reflect(const point & p) const {
    return point(2 * p.x - x, 2 * p.y - y);
  }

  //reflect across the line containing points p and q
  point reflect(const point & p, const point & q) const {
    if (p == q) return reflect(p);
    point r(*this - p), s = q - p;
    r = point(r.x * s.x + r.y * s.y, r.x * s.y - r.y * s.x) / s.norm();
    r = point(r.x * s.x - r.y * s.y, r.x * s.y + r.y * s.x) + p;
    return r;
  }

  friend double norm(const point & p) { return p.norm(); }
  friend double abs(const point & p) { return p.abs(); }
  friend double arg(const point & p) { return p.arg(); }
  friend double dot(const point & p, const point & q) { return p.dot(q); }
  friend double cross(const point & p, const point & q) { return p.cross(q); }
  friend double proj(const point & p, const point & q) { return p.proj(q); }
  friend point rot90(const point & p) { return p.rot90(); }
  friend point normalize(const point & p) { return p.normalize(); }
  friend point rotateCW(const point & p, const double & t) { return p.rotateCW(t); }
  friend point rotateCCW(const point & p, const double & t) { return p.rotateCCW(t); }
  friend point rotateCW(const point & p, const point & q, const double & t) { return p.rotateCW(q, t); }
  friend point rotateCCW(const point & p, const point & q, const double & t) { return p.rotateCCW(q, t); }
  friend point reflect(const point & p, const point & q) { return p.reflect(q); }
  friend point reflect(const point & p, const point & a, const point & b) { return p.reflect(a, b); }

  friend std::ostream & operator << (std::ostream & out, const point & p) {
    out << "(";
    out << (fabs(p.x) < eps ? 0 : p.x) << ",";
    out << (fabs(p.y) < eps ? 0 : p.y) << ")";
    return out;
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

const double PI = acos(-1);

int main() {
  point p(-10, 3);
  cout << (p + point(-3, 9) * 6 / 2 - point(-1, 1)) << "\n"; //(-18,29)
  cout << p.norm() << "\n";                                  //109
  cout << p.abs() << "\n";                                   //10.4403
  cout << p.arg() << "\n";                                   //2.85014
  cout << p.dot(point(3, 10)) << "\n";                       //0
  cout << p.cross(point(10, -3)) << "\n";                    //0
  cout << p.proj(point(-10, 0)) << "\n";                     //10
  cout << p.rot90() << "\n";                                 //(-3,-10)
  cout << p.normalize().abs() << "\n";                       //1
  cout << p.rotateCW(point(1, 1), PI / 2) << "\n";           //(3,12)
  cout << p.rotateCCW(point(2, 2), PI / 2) << "\n";          //(1,-10)
  cout << p.reflect(point(0, 0)) << "\n";                    //(10,-3)
  cout << p.reflect(point(-2, 0), point(5, 0)) << "\n";      //(-10,-3)
  return 0;
}
