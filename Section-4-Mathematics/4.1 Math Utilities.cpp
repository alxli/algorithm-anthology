/*

Common mathematic constants and functions, many of which are substitutes for
features which are not available in standard C++, or may not be available on
compilers that do not support C++11 or later.

Time Complexity:
- O(1) for all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

#ifndef M_PI
  const double M_PI = acos(-1.0);
#endif
#ifndef M_E
  const double M_E = exp(1.0);
#endif
const double M_PHI = (1.0 + sqrt(5.0))/2.0;
const double M_INF = std::numeric_limits<double>::infinity();
const double M_NAN = std::numeric_limits<double>::quiet_NaN();

#ifndef isnan
  #define isnan(x) ((x) != (x))
#endif

/*

Epsilon Comparisons

EQ(), NE(), LT(), GT(), LE(), and GE() relationally compares two values x and y
accounting for absolute error. For any x, the range of values considered equal
barring absolute error is [x - EPS, x + EPS]. Values outside of this range are
considered not equal (strictly less or strictly greater).

rEQ() returns whether x and y are equal barring relative error. For any x, the
range of values considered equal is [x*(1 - EPS), x*(1 + EPS)].

*/

const double EPS = 1e-9;

#define EQ(x, y) (fabs((x) - (y)) <= EPS)
#define NE(x, y) (fabs((x) - (y)) > EPS)
#define LT(x, y) ((x) < (y) - EPS)
#define GT(x, y) ((x) > (y) + EPS)
#define LE(x, y) ((x) <= (y) + EPS)
#define GE(x, y) ((x) >= (y) - EPS)
#define rEQ(x, y) (fabs((x) - (y)) <= EPS*fabs(x))

/*

Sign Functions

- sgn(x) returns -1 (if x < 0), 0 (if x == 0), or 1 (if x > 0). Unlike signbit()
  or copysign(), this does not handle the sign of NaN.
- signbit_(x) is analogous to std::signbit() in C++11 and later, returning
  whether the sign bit of the floating point number is set to true. If so, then
  x is considered "negative." Note that this works as expected on +0.0, -0.0,
  Inf, -Inf, NaN, as well as -NaN. Warning: This assumes that the sign bit is
  the leading (most significant) bit in the internal representation of the IEEE
  floating point value.
- copysign_(x, y) is analogous to std::copysign() in C++11 and later, returning
  a number with the magnitude of x but the sign of y.

*/

template<class T>
int sgn(const T &x) {
  return (T(0) < x) - (x < T(0));
}

template<class Double>
bool signbit_(Double x) {
  return (((unsigned char *)&x)[sizeof(x) - 1] >> (CHAR_BIT - 1)) & 1;
}

template<class Double>
Double copysign_(Double x, Double y) {
  return signbit_(y) ? -fabs(x) : fabs(x);
}

/*

Rounding Functions

- floor0(x) returns x rounded down, symmetrically towards zero. This function is
  analogous to trunc() in C++11 and later.
- ceil0(x) returns x rounded up, symmetrically away from zero. This function is
  analogous to round() in C++11 and later.
- round_half_up(x) returns x rounded half up, towards positive infinity.
- round_half_down(x) returns x rounded half down, towards negative infinity.
- round_half_to0(x) returns x rounded half down, symmetrically towards zero.
- round_half_from0(x) returns x rounded half up, symmetrically away from zero.
- round_half_even(x) returns x rounded half to even, using banker's rounding.
- round_half_alternate(x) returns x rounded, where ties are broken by
  alternating rounds towards positive and negative infinity.
- round_half_alternate0(x) returns x rounded, where ties are broken by
  alternating symmetric rounds towards and away from zero.
- round_half_random(x) returns x rounded, where ties are broken randomly.
- round_n_places(x, n, f) returns x rounded to n digits after the decimal, using
  the specified rounding function f(x).

*/

template<class Double>
Double floor0(const Double &x) {
  Double res = floor(fabs(x));
  return (x < 0.0) ? -res : res;
}

template<class Double>
Double ceil0(const Double &x) {
  Double res = ceil(fabs(x));
  return (x < 0.0) ? -res : res;
}

template<class Double>
Double round_half_up(const Double &x) {
  return floor(x + 0.5);
}

template<class Double>
Double round_half_down(const Double &x) {
  return ceil(x - 0.5);
}

template<class Double>
Double round_half_to0(const Double &x) {
  Double res = round_half_down(fabs(x));
  return (x < 0.0) ? -res : res;
}

template<class Double>
Double round_half_from0(const Double &x) {
  Double res = round_half_up(fabs(x));
  return (x < 0.0) ? -res : res;
}

template<class Double>
Double round_half_even(const Double &x, const Double &eps = 1e-9) {
  if (x < 0.0) {
    return -round_half_even(-x, eps);
  }
  Double ipart;
  modf(x, &ipart);
  if (x - (ipart + 0.5) < eps) {
    return (fmod(ipart, 2.0) < eps) ? ipart : ceil0(ipart + 0.5);
  }
  return round_half_from0(x);
}

template<class Double>
Double round_half_alternate(const Double &x) {
  static bool up = true;
  return (up = !up) ? round_half_up(x) : round_half_down(x);
}

template<class Double>
Double round_half_alternate0(const Double &x) {
  static bool up = true;
  return (up = !up) ? round_half_from0(x) : round_half_to0(x);
}

template<class Double>
Double round_half_random(const Double &x) {
  return (rand() % 2 == 0) ? round_half_from0(x) : round_half_to0(x);
}

template<class Double, class RoundingFunction>
Double round_n_places(const Double &x, unsigned int n, RoundingFunction f) {
  return f(x*pow(10, n)) / pow(10, n);
}

/*

Error Function

- erf_(x) returns the error encountered in integrating the normal distribution.
  Its value is 2/sqrt(pi)*(integral of e^(-t^2) dt from 0 to x). This function
  is analogous to erf(x) in C++11 and later.
- erfc_(x) returns the error function complement, that is, 1 - erf_(x). This
  function is analogous to erfc(x) in C++11 and later.

*/

#define ERF_EPS 1e-14

double erfc_(double x);

double erf_(double x) {
  if (signbit_(x)) {
    return -erf_(-x);
  }
  if (fabs(x) > 2.2) {
    return 1.0 - erfc_(x);
  }
  double sum = x, term = x, xx = x*x;
  int j = 1;
  do {
    term *= xx / j;
    sum -= term/(2*(j++) + 1);
    term *= xx / j;
    sum += term/(2*(j++) + 1);
  } while (fabs(term) > sum*ERF_EPS);
  return 2/sqrt(M_PI) * sum;
}

double erfc_(double x) {
  if (fabs(x) < 2.2) {
    return 1.0 - erf_(x);
  }
  if (signbit_(x)) {
    return 2.0 - erfc_(-x);
  }
  double a = 1, b = x, c = x, d = x*x + 0.5, q1, q2 = 0, n = 1.0, t;
  do {
    t = a*n + b*x;
    a = b;
    b = t;
    t = c*n + d*x;
    c = d;
    d = t;
    n += 0.5;
    q1 = q2;
    q2 = b / d;
  } while (fabs(q1 - q2) > q2*ERF_EPS);
  return 1/sqrt(M_PI) * exp(-x*x) * q2;
}

#undef ERF_EPS

/*

Gamma Functions

- tgamma_(x) returns the gamma function of x. Unlike the tgamma() function in
  C++11 and later, this version only supports positive x, returning NaN if x is
  less than or equal to 0.
- lgamma_(x) returns the natural logarithm of the absolute value of the gamma
  function of x. Unlike the lgamma() function in C++11 and later, this version
  only supports positive x, returning NaN if x is less than or equal to 0.

*/

double lgamma_(double x);

double tgamma_(double x) {
  if (x <= 0) {
    return M_NAN;
  }
  if (x < 1e-3) {
    return 1.0 / (x*(1.0 + 0.57721566490153286060651209*x));
  }
  if (x < 12) {
    double y = x;
    int n = 0;
    bool arg_was_less_than_one = (y < 1);
    if (arg_was_less_than_one) {
      y += 1;
    } else {
      n = (int)floor(y) - 1;
      y -= n;
    }
    static const double p[] = {
        -1.71618513886549492533811e+0, 2.47656508055759199108314e+1,
        -3.79804256470945635097577e+2, 6.29331155312818442661052e+2,
        8.66966202790413211295064e+2, -3.14512729688483675254357e+4,
        -3.61444134186911729807069e+4, 6.64561438202405440627855e+4};
    static const double q[] = {
        -3.08402300119738975254353e+1, 3.15350626979604161529144e+2,
        -1.01515636749021914166146e+3, -3.10777167157231109440444e+3,
        2.25381184209801510330112e+4, 4.75584627752788110767815e+3,
        -1.34659959864969306392456e+5, -1.15132259675553483497211e+5};
    double num = 0, den = 1, z = y - 1;
    for (int i = 0; i < 8; i++) {
      num = (num + p[i])*z;
      den = den*z + q[i];
    }
    double result = num/den + 1;
    if (arg_was_less_than_one) {
      result /= (y - 1);
    } else {
      for (int i = 0; i < n; i++) {
        result *= y++;
      }
    }
    return result;
  }
  return (x > 171.624) ? 2*DBL_MAX : exp(lgamma(x));
}

double lgamma_(double x) {
  if (x <= 0) {
    return M_NAN;
  }
  if (x < 12) {
    return log(fabs(tgamma_(x)));
  }
  static const double c[8] = {
    1.0/12, -1.0/360, 1.0/1260, -1.0/1680, 1.0/1188, -691.0/360360, 1.0/156,
    -3617.0/122400
  };
  double z = 1.0/(x*x), sum = c[7];
  for (int i = 6; i >= 0; i--) {
    sum = sum*z + c[i];
  }
  return (x - 0.5)*log(x) - x + 0.91893853320467274178032973640562 + sum/x;
}

/*

Base Conversion

- Given an integer in base a as a vector d of digits (where d[0] is the least
  significant digit), convert_base(d, a, b) returns a vector of the integer's
  digits when converted base b (again with index 0 storing the least significant
  digit). The actual value of the entire integer to be converted must be able to
  fit within an unsigned 64-bit integer for intermediate storage.
- convert_digits(x, b) returns the digits of the unsigned integer x in base b,
  where index 0 of the result stores the least significant digit.
- to_roman(x) returns the Roman numeral representation of the unsigned integer x
  as a C++ string.

*/

std::vector<int> convert_base(const std::vector<int> &d, int a, int b) {
  unsigned long long x = 0, power = 1;
  for (int i = 0; i < (int)d.size(); i++) {
    x += d[i]*power;
    power *= a;
  }
  int n = ceil(log(x + 1)/log(b));
  std::vector<int> res;
  for (int i = 0; i < n; i++) {
    res.push_back(x % b);
    x /= b;
  }
  return res;
}

std::vector<int> convert_base(unsigned int x, int b = 10) {
  std::vector<int> res;
  while (x != 0) {
    res.push_back(x % b);
    x /= b;
  }
  return res;
}

std::string to_roman(unsigned int x) {
  static const std::string h[] =
      {"", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM"};
  static const std::string t[] =
      {"", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC"};
  static const std::string o[] =
      {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX"};
  std::string prefix(x / 1000, 'M');
  x %= 1000;
  return prefix + h[x/100] + t[x/10 % 10] + o[x % 10];
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>

int main() {
  assert(EQ(M_PI, 3.14159265359));
  assert(EQ(M_E, 2.718281828459));
  assert(EQ(M_PHI, 1.61803398875));

  double x = -12345.6789;
  assert((-M_INF < x) && (x < M_INF));
  assert((M_INF + x == M_INF) && (M_INF - x == M_INF));
  assert((M_INF + M_INF == M_INF) && (-M_INF - M_INF == -M_INF));
  assert((M_NAN != x) && (M_NAN != M_INF) && (M_NAN != M_NAN));
  assert(!(M_NAN < x) && !(M_NAN > x) && !(M_NAN <= x) && !(M_NAN >= x));
  assert(isnan(0.0*M_INF) && isnan(0.0*-M_INF) && isnan(M_INF/-M_INF));
  assert(isnan(M_NAN) && isnan(-M_NAN) && isnan(M_INF - M_INF));

  assert(sgn(x) == -1 && sgn(0.0) == 0 && sgn(5678) == 1);
  assert(signbit_(x) && !signbit_(0.0) && signbit_(-0.0));
  assert(!signbit_(M_INF) && signbit_(-M_INF));
  assert(!signbit_(M_NAN) && signbit_(-M_NAN));
  assert(copysign(1.0, +2.0) == +1.0 && copysign(M_INF, -2.0) == -M_INF);
  assert(copysign(1.0, -2.0) == -1.0 && std::signbit(copysign(M_NAN, -2.0)));

  assert(EQ(floor0(1.5), 1.0) && EQ(ceil0(1.5), 2.0));
  assert(EQ(floor0(-1.5), -1.0) && EQ(ceil0(-1.5), -2.0));
  assert(EQ(round_half_up(+1.5), +2) && EQ(round_half_down(+1.5), +1));
  assert(EQ(round_half_up(-1.5), -1) && EQ(round_half_down(-1.5), -2));
  assert(EQ(round_half_to0(+1.5), +1) && EQ(round_half_from0(+1.5), +2));
  assert(EQ(round_half_to0(-1.5), -1) && EQ(round_half_from0(-1.5), -2));
  assert(EQ(round_half_even(+1.5), +2) && EQ(round_half_even(-1.5), -2));
  assert(NE(round_half_alternate(+1.5), round_half_alternate(+1.5)));
  assert(NE(round_half_alternate0(-1.5), round_half_alternate0(-1.5)));
  assert(EQ(round_n_places(-1.23456, 3, round_half_to0<double>), -1.235));

  assert(EQ(erf_(1.0), 0.8427007929) && EQ(erf_(-1.0), -0.8427007929));
  assert(EQ(tgamma_(0.5), 1.7724538509) && EQ(tgamma_(1.0), 1.0));
  assert(EQ(lgamma_(0.5), 0.5723649429) && EQ(lgamma_(1.0), 0.0));

  int digits[] = {6, 5, 4, 3, 2, 1};
  std::vector<int> base20 = convert_base(123456, 20);
  assert(convert_base(base20, 20, 10) == std::vector<int>(digits, digits + 6));
  assert(to_roman(1234) == "MCCXXXIV");
  assert(to_roman(5678) == "MMMMMDCLXXVIII");
  return 0;
}
