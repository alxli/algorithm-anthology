/*

4.1 - Mathematics Toolbox

Useful math definitions. Excludes geometry (see next chapter).

*/

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

/* Definitions for Common Floating Point Constants */

const double PI = acos(-1.0), E = exp(1.0), root2 = sqrt(2.0);
const double phi = (1.0 + sqrt(5.0)) / 2.0; //golden ratio

//Sketchy but working defintions of +infinity, -infinity and quiet NaN
//A better way is using functions of std::numeric_limits<T> from <limits>
//See main() for identities involving the following special values.
const double posinf = 1.0/0.0, neginf = -1.0/0.0, NaN = -(0.0/0.0);


/*

Epsilon Comparisons

The range of values comparing EQ() to X is made to be [X - eps, X + eps].
For values to compare LT() and GT() x, they must fall outside of the range.

e.g. if eps = 1e-7, then EQ(1e-8, 2e-8) is true and LT(1e-8, 2e-8) is false.

*/

const double eps = 1e-7;
#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define NE(a, b) (fabs((a) - (b)) > eps)  /* not equal to */
#define LT(a, b) ((a) < (b) - eps)        /* less than */
#define GT(a, b) ((a) > (b) + eps)        /* greater than */
#define LE(a, b) ((a) <= (b) + eps)       /* less than or equal to */
#define GE(a, b) ((a) >= (b) - eps)       /* greater than or equal to */


/*

Sign Function:

Returns: -1 (if x < 0), 0 (if x = 0), or 1 (if x > 0)
Warning: Does not handle the sign of +/-NaN (see signbit() in C++11)

*/

template<class T> int sgn(const T & x) {
  return (T(0) < x) - (x < T(0));
}


/*

Modulo Operation - Euclidean Definition

The % operator in C/C++ returns the remainder of division (which may be
a positive or negative result) The true Euclidean definition of modulo,
however, defines the remainder to be always nonnegative. For positive
operators, % and mod are the same. But for negative operands, they differ.
The result here is consistent with the Euclidean division algorithm.

e.g. -21 % 4 == -1 because -21 / 4 == -5 and 4 * -5 + (-1) == -21
      however, -21 mod 4 is equal to 3 because -21 + 4 * 6 is 3.

*/

template<class T> T mod(const T & a, const T & b) {
  if (b < T(0)) return a - (b * (T)ceil((double)a / b));
  return a - (b * (T)floor((double)a / b));
}


/*

Floating Point Rounding Functions

floor() in <cmath> asymmetrically rounds down, towards -infinity,
while ceil() in <cmath> asymmetrically rounds up, towards +infinity.
The following are common alternative ways to round.

*/

//symmetric round down, bias: towards zero (same as trunc() in C++11)
template<class Double> Double floor0(const Double & x) {
  Double res = floor(fabs(x));
  return (x < 0.0) ? -res : res;
}

//symmetric round up, bias: away from zero
template<class Double> Double ceil0(const Double & x) {
  Double res = ceil(fabs(x));
  return (x < 0.0) ? -res : res;
}

//round half up, bias: towards +infinity
template<class Double> Double roundhalfup(const Double & x) {
  return floor(x + 0.5);
}

//round half up, bias: towards -infinity
template<class Double> Double roundhalfdown(const Double & x) {
  return ceil(x - 0.5);
}

//symmetric round half down, bias: towards zero
template<class Double> Double roundhalfdown0(const Double & x) {
  Double res = roundhalfdown(fabs(x));
  return (x < 0.0) ? -res : res;
}

//symmetric round half up, bias: away from zero
template<class Double> Double roundhalfup0(const Double & x) {
  Double res = roundhalfup(fabs(x));
  return (x < 0.0) ? -res : res;
}

//round half even (banker's rounding), bias: none
template<class Double>
Double roundhalfeven(const Double & x, const Double & eps = 1e-7) {
  if (x < 0.0) return -roundhalfeven(-x, eps);
  Double ipart;
  modf(x, &ipart);
  if (x - (ipart + 0.5) < eps)
    return (fmod(ipart, 2.0) < eps) ? ipart : ceil0(ipart + 0.5);
  return roundhalfup0(x);
}

//round alternating up/down for ties, bias: none for sequential calls
template<class Double> Double roundalternate(const Double & x) {
  static bool up = true;
  return (up = !up) ? roundhalfup(x) : roundhalfdown(x);
}

//symmetric round alternate, bias: none for sequential calls
template<class Double> Double roundalternate0(const Double & x) {
  static bool up = true;
  return (up = !up) ? roundhalfup0(x) : roundhalfdown0(x);
}

//round randomly for tie-breaking, bias: generator's bias
template<class Double> Double roundrandom(const Double & x) {
  return (rand() % 2 == 0) ? roundhalfup0(x) : roundhalfdown0(x);
}

//round x to N digits after the decimal using the specified round function
//e.g. roundplaces(-1.23456, 3, roundhalfdown0<double>) returns -1.235
template<class Double, class Function>
double roundplaces(const Double & x, unsigned int N, Function f) {
  return f(x * pow(10, N)) / pow(10, N);
}


/*

Error Function (erf() in C++11)
Adapted from: http://www.johndcook.com/blog/cpp_erf/

*/

double erf(double x) {
  static const double a1 =  0.254829592, a2 = -0.284496736, a3 = 1.421413741;
  static const double a4 = -1.453152027, a5 =  1.061405429, p  = 0.3275911;
  int sign = (x < 0.0) ? -1 : 1;
  x = fabs(x);
  double t = 1.0/(1.0 + p*x);
  return sign*(1.0 - ((((a5*t + a4)*t + a3)*t + a2)*t + a1)*t*exp(-x*x));
}


/*

Gamma and Log-Gamma Functions (tgamma() and lgamma() in C++11)
Adapted from: http://www.johndcook.com/blog/cpp_gamma/

*/

double lgamma(double x);

double tgamma(double x) {
  if (x <= 0.0) return NaN; //x must be positive
  static const double gamma = 0.577215664901532860606512090;
  if (x < 1e-3) return 1.0/(x*(1.0 + gamma*x));
  if (x < 12.0) {
    double y = x;
    int n = 0;
    bool arg_was_less_than_one = (y < 1.0);
    if (arg_was_less_than_one) y += 1.0;
    else y -= (n = static_cast<int>(floor(y)) - 1);
    static const double p[] = {
      -1.71618513886549492533811E+0, 2.47656508055759199108314E+1,
      -3.79804256470945635097577E+2, 6.29331155312818442661052E+2,
       8.66966202790413211295064E+2,-3.14512729688483675254357E+4,
      -3.61444134186911729807069E+4, 6.64561438202405440627855E+4
    };
    static const double q[] = {
      -3.08402300119738975254353E+1, 3.15350626979604161529144E+2,
      -1.01515636749021914166146E+3,-3.10777167157231109440444E+3,
       2.25381184209801510330112E+4, 4.75584627752788110767815E+3,
      -1.34659959864969306392456E+5,-1.15132259675553483497211E+5
    };
    double num = 0.0, den = 1.0, z = y - 1;
    for (int i = 0; i < 8; i++) {
      num = (num + p[i])*z;
      den = den*z + q[i];
    }
    double result = num/den + 1.0;
    if (arg_was_less_than_one) result /= (y - 1.0);
    else for (int i = 0; i < n; i++) result *= y++;
    return result;
  }
  return (x > 171.624) ? DBL_MAX*2.0 : exp(lgamma(x));
}

double lgamma(double x) {
  if (x <= 0.0) return NaN; //x must be positive
  if (x < 12.0) return log(fabs(tgamma(x)));
  static const double c[8] = {
    1.0/12.0, -1.0/360.0, 1.0/1260.0, -1.0/1680.0, 1.0/1188.0,
    -691.0/360360.0, 1.0/156.0, -3617.0/122400.0
  };
  double z = 1.0/(x*x), sum = c[7];
  for (int i = 6; i >= 0; i--) sum = sum*z + c[i];
  static const double halflog2pi = 0.91893853320467274178032973640562;
  return (x - 0.5)*log(x) - x + halflog2pi + sum/x;
}


/*

Base Conversion - O(N) on the number of digits

Given the digits of an integer x in base a, returns x's digits in base b.
Precondition: the base-10 value of x must be able to fit within an unsigned
long long. In other worst, the value of x must be between 0 and 2^64 - 1.

Note: vector[0] stores the most significant digit in all usages below.

e.g. if x = {1, 2, 3} and a = 5 (i.e. x = 123 in base 5 = 38 in base 10),
then convert_base(x, 5, 3) returns {1, 1, 0, 2} (1102 in base 2).

*/

std::vector<int> convert_base(const std::vector<int> & x, int a, int b) {
  unsigned long long base10 = 0;
  for (int i = 0; i < x.size(); i++)
    base10 += x[i] * pow(a, x.size() - i - 1);
  int N = ceil(log(base10 + 1) / log(b));
  std::vector<int> baseb;
  for (int i = 1; i <= N; i++)
    baseb.push_back(int(base10/pow(b, N - i)) % b);
  return baseb;
}

//returns digits of a number in base b
std::vector<int> base_digits(int x, int b = 10) {
  std::vector<int> baseb;
  while (x != 0) {
    baseb.push_back(x % b);
    x /= b;
  }
  std::reverse(baseb.begin(), baseb.end());
  return baseb;
}


/*

Integer to Roman Numerals Conversion

Given an integer x, this function returns the Roman numeral representation
of x as a C++ string. More 'M's are appended to the front of the resulting
string if x is greater than 1000. e.g. to_roman(1234) returns "CCXXXIV"

*/

std::string to_roman(int x) {
  static std::string h[] = {"","C","CC","CCC","CD","D","DC","DCC","DCCC","CM"};
  static std::string t[] = {"","X","XX","XXX","XL","L","LX","LXX","LXXX","XC"};
  static std::string o[] = {"","I","II","III","IV","V","VI","VII","VIII","IX"};
  return std::string(x/1000, 'M') + h[(x %= 1000)/100] + t[x/10%10] + o[x%10];
}


#include <cassert>
#include <cstdio>
#include <iostream>
using namespace std;

int main() {
  cout << "PI: " << PI << "\n";
  cout << "E: " << E << "\n";
  cout << "sqrt(2): " << root2 << "\n";
  cout << "Golden ratio: " << phi << "\n";

  //testing some properties of posinf, neginf, and NaN:
  double x = -1234.567890; //any value will work
  assert((posinf > x) && (neginf < x) && (posinf == -neginf));
  assert((posinf + x == posinf) && (posinf - x == posinf));
  assert((neginf + x == neginf) && (neginf - x == neginf));
  assert((posinf + posinf == posinf) && (neginf - posinf == neginf));
  assert((NaN != x) && (NaN != NaN) && (NaN != posinf) && (NaN != neginf));
  assert(!(NaN < x) && !(NaN > x) && !(NaN <= x) && !(NaN >= x));
  assert(isnan(0.0*posinf) && isnan(0.0*neginf) && isnan(posinf/neginf));
  assert(isnan(NaN) && isnan(-NaN) && isnan(NaN*x + x - x/-NaN));
  assert(isnan(neginf-neginf) && isnan(posinf-posinf) && isnan(posinf+neginf));

  //epsilon comparisons
  assert(EQ(1e-8, 2e-8) && !LT(1e-8, 2e-8)); //above, eps is defined as 1e-7

  assert(sgn(-1.234) == -1 && sgn(0.0) == 0 && sgn(5678) == 1);
  assert(mod(21, 4) == 1 && mod(-21, 4) == 3);
  
  return 0;
}
