/*

A straight line in two dimensions, templated on the coordinate type `T` (default `double`). The line
$ax + by + c = 0$ is stored unnormalized, so a line through integer points $(p_x, p_y)$ and
$(q_x, q_y)$ keeps the integer coefficients: $a = q_y - p_y$, $b = p_x - q_x$,
$c = -(a \cdot p_x + b \cdot p_y)$. Storage and the exact predicates `contains()`, `is_parallel()`,
and `is_perpendicular()` therefore stay exact for integral type `T`. The inherently-fractional
operations `slope()`, `x()`, and `y()` return `fp_t`, which is `double` when `T` is integral and `T`
itself otherwise.

Because coefficients are not normalized to a canonical form, `operator==` compares lines by
proportionality (whether they are the same geometric line) rather than by coefficient equality.

Operations include `horizontal()`, `vertical()`, `slope()`, evaluating $y$ at some $x$ with `y()`
(and vice versa with `x()`), checking if a point falls on the line with `contains()`, checking if
another line is parallel or perpendicular with `is_parallel()` or `is_perpendicular()`, and finding
the `parallel()` or `perpendicular()` line through a point.

Overflow warning: the exact predicates form products of coefficients (e.g. cross terms in
`is_parallel()`/`is_perpendicular()` and `a*p.x + b*p.y` in `contains()`), which grow like the
squared coordinate magnitude. For integral type `T`, use `LineL` (`TLine<int64_t>`) once
coordinates exceed $\sim 46000$, or the 32-bit products overflow.

Type aliases:
- `LineI = TLine<int>`: exact integer-coefficient lines (small values only; see overflow warning)
- `LineL = TLine<int64_t>`: exact integer-coefficient lines for large coordinates
- `LineD = TLine<double>`: standard floating-point
- `LineLD = TLine<long double>`: extra precision
- `Line = LineD`: default line type is double

Time Complexity:
- O(1) per call to the constructor and all other operations.

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
  if constexpr (std::is_floating_point_v<C>) return std::fabs(C(a) - C(b)) <= static_cast<C>(EPS);
  return C(a) == C(b);
}

// SFINAE guard: valid only when Pt exposes numeric .x/.y members, so templated point constructors
// don't hijack scalar-argument calls.
template<typename Pt>
using if_point = decltype(std::declval<const Pt &>().x, std::declval<const Pt &>().y, void());

template<typename T>
struct TLine {
  // fp_t is T for floating-point types, double for integral types.
  using fp_t = std::conditional_t<std::is_integral<T>::value, double, T>;

  T a, b, c;

  TLine() : a(0), b(0), c(0) {}  // Invalid or uninitialized line.
  TLine(T a, T b, T c) : a(a), b(b), c(c) {}

  // Line through two points. Coefficients are exact (type T) for integer points.
  template<typename Pt, typename = if_point<Pt>>
  TLine(const Pt &p, const Pt &q) : a(q.y - p.y), b(p.x - q.x), c(-(a * p.x + b * p.y)) {}

  // Line of a given slope through a point. Intended for floating-point T (a generic slope
  // produces non-integer coefficients), so this constructor is disabled for integer T.
  template<
      class Pt, typename = if_point<Pt>, typename U = T,
      class = std::enable_if_t<std::is_floating_point<U>::value>>
  TLine(fp_t slope, const Pt &p) : a(-slope), b(1), c(slope * p.x - p.y) {}

  // Two lines are equal iff their coefficient vectors are proportional (the same line).
  bool operator==(const TLine &l) const {
    return EQ(a * l.b, l.a * b) && EQ(b * l.c, l.b * c) && EQ(a * l.c, l.a * c);
  }

  bool operator!=(const TLine &l) const { return !(*this == l); }

  // Returns whether the line is a valid line (not both a and b zero).
  bool valid() const { return !(EQ(a, 0) && EQ(b, 0)); }
  bool horizontal() const { return valid() && EQ(a, 0); }
  bool vertical() const { return valid() && EQ(b, 0); }

  // Slope -a/b, or NaN if the line is vertical or invalid.
  fp_t slope() const { return (!valid() || EQ(b, 0)) ? M_NAN : -(fp_t)a / b; }

  // Solve for x at a given y (NaN if horizontal or invalid).
  fp_t x(fp_t y) const { return (!valid() || EQ(a, 0)) ? M_NAN : -((fp_t)b * y + c) / a; }

  // Solve for y at a given x (NaN if vertical or invalid).
  fp_t y(fp_t x) const { return (!valid() || EQ(b, 0)) ? M_NAN : -((fp_t)a * x + c) / b; }

  // Whether point p lies on the line. Exact for integer T and integer p.
  template<typename Pt>
  bool contains(const Pt &p) const {
    return EQ(a * p.x + b * p.y + c, 0);
  }

  // Parallel iff the normals are parallel (cross == 0); perpendicular iff normals are
  // perpendicular (dot == 0). Both exact for integer T.
  bool is_parallel(const TLine &l) const { return EQ(a * l.b, l.a * b); }
  bool is_perpendicular(const TLine &l) const { return EQ(a * l.a, -(b * l.b)); }

  // Parallel/perpendicular line through point p. Exact for integer T and integer p.
  template<typename Pt, typename = if_point<Pt>>
  TLine parallel(const Pt &p) const {
    return TLine(a, b, -(a * p.x + b * p.y));
  }

  template<typename Pt, typename = if_point<Pt>>
  TLine perpendicular(const Pt &p) const {
    return TLine(-b, a, b * p.x - a * p.y);
  }

  friend std::ostream &operator<<(std::ostream &out, const TLine &l) {
    auto clean = [](T v) -> double {
      return fabs(static_cast<double>(v)) < EPS ? 0 : static_cast<double>(v);
    };
    return out << clean(l.a) << "x" << std::showpos << clean(l.b) << "y" << clean(l.c) << "=0"
               << std::noshowpos;
  }
};

using LineI = TLine<int>;
using LineL = TLine<int64_t>;
using LineD = TLine<double>;
using LineLD = TLine<long double>;
using Line = LineD;  // Default line type is double.

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
  // Floating-point line.
  Line l(2, -5, -8);
  Line para = Line(2, -5, -8).parallel(Point(-6, -2));
  Line perp = Line(2, -5, -8).perpendicular(Point(-6, -2));
  assert(l.is_parallel(para) && l.is_perpendicular(perp));
  assert(l.slope() == 0.4);
  // operator== is proportionality-based: para is the same line as -0.4x + y - 0.4 = 0.
  assert(para == Line(-0.4, 1, -0.4));
  assert(perp == Line(2.5, 1, 17));
  assert(l.contains(Point(1.5, -1)));  // 2(1.5) - 5(-1) - 8 = 0.

  // Integer line through integer points - exact coefficients and predicates.
  LineI il(PointI(0, 0), PointI(2, 1));  // a=1, b=-2, c=0  ->  x - 2y = 0
  assert(il.a == 1 && il.b == -2 && il.c == 0);
  assert(il.contains(PointI(4, 2)));  // exact integer check
  assert(!il.contains(PointI(4, 3)));
  assert(il.is_parallel(LineI(PointI(1, 1), PointI(3, 2))));        // both slope 1/2
  assert(il.is_perpendicular(LineI(PointI(0, 0), PointI(1, -2))));  // dot of normals = 0
  assert(il.slope() == 0.5);                                        // fp_t result
  return 0;
}
