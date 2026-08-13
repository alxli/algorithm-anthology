/*

A straight line in two dimensions, represented by coefficients of $ax + by + c = 0$ and templated on
their type `T`. Coefficients are canonicalized by default so proportional valid coefficient vectors
have the same representation, up to floating-point rounding.

The template flag `CANONICALIZE` controls whether construction normalizes the coefficients and
defaults to `true`. When enabled, `EXACT = true` divides coefficients by their Euclidean GCD and
fixes their sign, while `EXACT = false` scales a nonvertical line to $b = 1$ and a vertical line to
$a = 1$. `EXACT` defaults to `true` for built-in integral types; set it explicitly for custom exact
types such as `BigInt` and `Rational`. Exact canonicalization requires `%`, comparisons, and basic
arithmetic, and predicates remain exact for integral `T`. Floating-point and `Modular` coefficients
use the scaling form.

- `TLine<T>(a, b, c)` constructs a line from its coefficients, while `TLine<T>(p, q)` constructs the
  line through distinct points `p` and `q`. For floating-point `T`, `TLine<T>(slope, p)` constructs
  the line of the given slope through `p`. The default constructor produces an invalid line.
- `canonicalized()` returns a canonical copy. `operator==` compares stored coefficients exactly,
  while `EQ(l1, l2)` canonicalizes raw lines if necessary and uses `EPS` for floating-point
  coefficients.
- `valid()`, `horizontal()`, and `vertical()` classify the line.
- `slope()`, `x(y)`, and `y(x)` return `fp_t`, which is `T` for floating-point coefficients and
  `double` otherwise. An undefined result is returned as `NaN`.
- `contains(p)`, `is_parallel(l)`, and `is_perpendicular(l)` test geometric relationships.
- `parallel(p)` and `perpendicular(p)` return the corresponding line through point `p`.
- `operator<<` outputs the line in the form `ax+by+c=0`.
- `LineI`, `LineL`, `LineD`, and `LineLD` use `int`, `int64_t`, `double`, and `long double`
  coefficients, respectively. `Line` aliases `LineD`.

Overflow warning: the exact predicates form products of coefficients, which grow like the squared
coordinate magnitude. With `LineI` these may overflow once coordinates reach a few tens of
thousands, so use `LineL` for larger integer coordinates.

Time Complexity:
- O(log C) per integral canonicalizing constructor or call to `canonicalize()`, where $C$ is the
  largest absolute coefficient.
- O(1) per call to all other operations.

Space Complexity:
- O(1) for storage of the line.
- O(1) auxiliary for all operations.

*/

#include <cmath>
#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>
#include <utility>

const double EPS = 1e-9;
const double M_NAN = std::numeric_limits<double>::quiet_NaN();

// Epsilon-aware for floating-point coordinates; exact for int and for coordinate types like Modular
// or Rational, which therefore compose for all of the predicates below.
template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) == C(b) || std::fabs(C(a) - C(b)) <= EPS;
  return C(a) == C(b);
}

// SFINAE guard: valid only when Pt exposes numeric .x/.y members, so templated point constructors
// don't hijack scalar-argument calls.
template<typename Pt>
using if_point = decltype(std::declval<const Pt &>().x, std::declval<const Pt &>().y, void());

template<typename T, bool CANONICALIZE = true, bool EXACT = std::is_integral<T>::value>
struct TLine {
  // Slope and solve operations preserve floating-point coordinates and otherwise promote to double.
  using fp_t = std::conditional_t<std::is_floating_point<T>::value, T, double>;

  T a, b, c;

  void canonicalize() {
    if constexpr (EXACT) {
      auto gcd = [](T x, T y) {
        if (x < T{0}) x = -x;
        if (y < T{0}) y = -y;
        while (y != T{0}) {
          x %= y;
          std::swap(x, y);
        }
        return x;
      };
      T g = gcd(a, gcd(b, c));
      if (g != 0) {
        a /= g;
        b /= g;
        c /= g;
      }
      if (a < 0 || (a == 0 && b < 0)) {
        a = -a;
        b = -b;
        c = -c;
      }
    } else if (!EQ(b, T{0})) {
      a /= b;
      c /= b;
      b = T{1};
    } else if (!EQ(a, T{0})) {
      c /= a;
      a = T{1};
      b = T{0};
    }
  }

  TLine() : a(0), b(0), c(0) {}  // Invalid or uninitialized line.

  TLine(T a, T b, T c) : a(a), b(b), c(c) {
    if constexpr (CANONICALIZE) canonicalize();
  }

  // Line through two points. Coefficients are exact for integral points (overflow warning for c).
  template<typename Pt, typename = if_point<Pt>>
  TLine(const Pt &p, const Pt &q) : a(q.y - p.y), b(p.x - q.x), c(-(a * p.x + b * p.y)) {
    if constexpr (CANONICALIZE) canonicalize();
  }

  // Line of a given slope through a point. Intended for floating-point T (a generic slope
  // produces non-integer coefficients), so this constructor is disabled for integer T.
  template<
      class Pt, typename = if_point<Pt>, typename U = T,
      class = std::enable_if_t<std::is_floating_point<U>::value>>
  TLine(fp_t slope, const Pt &p) : a(-slope), b(1), c(slope * p.x - p.y) {
    if constexpr (CANONICALIZE) canonicalize();
  }

  TLine<T, true, EXACT> canonicalized() const { return TLine<T, true, EXACT>(a, b, c); }

  bool operator==(const TLine &l) const { return a == l.a && b == l.b && c == l.c; }
  bool operator!=(const TLine &l) const { return !(*this == l); }

  friend bool EQ(const TLine &l1, const TLine &l2) {
    if constexpr (CANONICALIZE) {
      return EQ(l1.a, l2.a) && EQ(l1.b, l2.b) && EQ(l1.c, l2.c);
    }
    return EQ(l1.canonicalized(), l2.canonicalized());
  }

  // Returns whether the line is a valid line (not both a and b zero).
  bool valid() const { return !(EQ(a, 0) && EQ(b, 0)); }
  bool horizontal() const { return valid() && EQ(a, 0); }
  bool vertical() const { return valid() && EQ(b, 0); }

  // Slope -a/b, or NaN if the line is vertical or invalid.
  fp_t slope() const { return (!valid() || EQ(b, 0)) ? M_NAN : -fp_t(a) / b; }

  // Solve for x at a given y (NaN if horizontal or invalid).
  fp_t x(fp_t y) const { return (!valid() || EQ(a, 0)) ? M_NAN : -(fp_t(b) * y + c) / a; }

  // Solve for y at a given x (NaN if vertical or invalid).
  fp_t y(fp_t x) const { return (!valid() || EQ(b, 0)) ? M_NAN : -(fp_t(a) * x + c) / b; }

  // Whether point p lies on the line. Exact for integer T and integer p.
  template<typename Pt>
  bool contains(const Pt &p) const {
    return valid() && EQ(a * p.x + b * p.y + c, 0);  // Overflow warning.
  }

  // Parallel iff the normals are parallel (cross == 0); perpendicular iff normals are perpendicular
  // (dot == 0). Both exact for integer T. Overflow warning.
  bool is_parallel(const TLine &l) const { return valid() && l.valid() && EQ(a * l.b, l.a * b); }

  bool is_perpendicular(const TLine &l) const {
    return valid() && l.valid() && EQ(a * l.a, -(b * l.b));
  }

  // Parallel/perpendicular line through point p. Exact for integer T and integer p.
  template<typename Pt, typename = if_point<Pt>>
  TLine parallel(const Pt &p) const {
    return TLine(a, b, -(a * p.x + b * p.y));  // Overflow warning.
  }

  template<typename Pt, typename = if_point<Pt>>
  TLine perpendicular(const Pt &p) const {
    return TLine(-b, a, b * p.x - a * p.y);  // Overflow warning.
  }

  friend std::ostream &operator<<(std::ostream &out, const TLine &l) {
    auto clean = [](T v) {
      if constexpr (std::is_floating_point_v<T>) {
        return std::fabs(v) < EPS ? T{0} : v;
      }
      return v;
    };
    auto flags = out.flags();
    out << clean(l.a) << "x" << std::showpos << clean(l.b) << "y" << clean(l.c) << "=0";
    out.flags(flags);
    return out;
  }
};

using LineI = TLine<int>;
using LineL = TLine<int64_t>;
using LineD = TLine<double>;
using LineLD = TLine<long double>;
using Line = LineD;  // Default line type is double.

// Can compose with numerical types from chapter 6:
// using LineB = TLine<BigInt, true, true>;
// using LineR = TLine<Rational<int64_t>, true, true>;

/*** Example Usage ***/

#include <cassert>

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
};

int main() {
  Line invalid;
  assert(!invalid.contains(Point(0, 0)) && !invalid.is_parallel(invalid));

  // Floating-point line.
  Line l(2, -5, -8);
  Line para = Line(2, -5, -8).parallel(Point(-6, -2));
  Line perp = Line(2, -5, -8).perpendicular(Point(-6, -2));
  assert(l.is_parallel(para) && l.is_perpendicular(perp));
  assert(l.slope() == 0.4);
  assert(EQ(para, Line(-0.4, 1, -0.4)));
  assert(EQ(perp, Line(2.5, 1, 17)));
  assert(l.contains(Point(1.5, -1)));  // 2(1.5) - 5(-1) - 8 = 0.

  // Integer line through integer points - exact canonical coefficients and predicates.
  LineI il(PointI(0, 0), PointI(2, 1));  // a=1, b=-2, c=0  ->  x - 2y = 0
  assert(il.a == 1 && il.b == -2 && il.c == 0);
  assert(il.contains(PointI(4, 2)));  // exact integer check
  assert(!il.contains(PointI(4, 3)));
  assert(il.is_parallel(LineI(PointI(1, 1), PointI(3, 2))));        // both slope 1/2
  assert(il.is_perpendicular(LineI(PointI(0, 0), PointI(1, -2))));  // dot of normals = 0
  assert(il.slope() == 0.5);                                        // fp_t result

  LineI reduced(2, -4, -6);
  assert(reduced == LineI(1, -2, -3));

  using RawLine = TLine<double, false>;
  RawLine raw(2, -5, -8), scaled(-0.4, 1, 1.6);
  assert(raw != scaled);    // Stored coefficients differ.
  assert(EQ(raw, scaled));  // Canonicalized copies represent the same line.
  RawLine canonical = raw;
  canonical.canonicalize();
  assert(canonical == scaled);
  assert(EQ(raw.canonicalized(), Line(-0.4, 1, 1.6)));
  return 0;
}
