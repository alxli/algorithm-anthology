/*

4.6.1 - Real Root Finding (Bisection)

Real roots can be found via binary searching, a.k.a the bisection
method. If two x-coordinates evaluate to y-coordinates that have
opposite signs, a root must exist between them. For a polynomial
function, at most 1 root lies between adjacent local extrema.
Since local extrema exist where the derivative equals 0, we can
break root-finding into the subproblem of finding the roots of
the derivative. Recursively solve for local extrema until we get
to a base case of degree 0. For each set of local extrema found,
binary search between pairs of extrema for a root. This method is
easy, robust, and allows us to find the root to an arbitrary level
of accuracy. We're limited only by the precision of the arithmetic.

Complexity: For a degree N polynomial, repeatedly differentiating
it will take N + (N-1) + ... + 1 = O(N^2) operations. At each step
we binary search the number of times equal to the current degree.
If we want to make roots precise to eps=10^-P, each binary search
will take O(log P). Thus the overall complexity is O(N^2 log P).

*/

#include <cmath>   /* fabsl(), powl() */
#include <limits>  /* std::numeric_limits<>::quiet_NaN() */
#include <utility> /* std::pair<> */
#include <vector>

typedef long double Double;
typedef std::vector<std::pair<Double, int> > poly;

const Double epsa = 1e-11; //required precision of roots in absolute error
const Double epsr = 1e-15; //required precision of roots in relative error
const Double eps0 = 1e-17; //x is considered a root if fabs(eval(x))<=eps0
const Double inf = 1e20;   //[-inf, inf] is the range of roots to consider
const Double NaN = std::numeric_limits<Double>::quiet_NaN();

Double eval(const poly & p, Double x) {
  Double res = 0;
  for (int i = 0; i < (int)p.size(); i++)
    res += p[i].first * powl(x, p[i].second);
  return res;
}

Double find_root(const poly & p, Double x1, Double x2) {
  Double y1 = eval(p, x1), y2 = eval(p, x2);
  if (fabsl(y1) <= eps0) return x1;
  bool neg1 = (y1 < 0), neg2 = (y2 < 0);
  if (fabsl(y2) <= eps0 || neg1 == neg2) return NaN;
  while (x2 - x1 > epsa && x1*(1 + epsr) < x2 && x2*(1 + epsr) > x1) {
    Double x = (x1 + x2) / 2;
    ((eval(p, x) < 0) == neg1 ? x1 : x2) = x;
  }
  return x1;
}

std::vector<Double> find_all_roots(const poly & p) {
  poly dif;
  for (int i = 0; i < (int)p.size(); i++)
    if (p[i].second > 0)
      dif.push_back(std::make_pair(p[i].first * p[i].second, p[i].second - 1));
  if (dif.empty()) return std::vector<Double>();
  std::vector<Double> res, r = find_all_roots(dif);
  r.insert(r.begin(), -inf);
  r.push_back(inf);
  for (int i = 0; i < (int)r.size() - 1; i++) {
    Double root = find_root(p, r[i], r[i + 1]);
    if (root != root) continue; //NaN, not found
    if (res.empty() || root != res.back())
      res.push_back(root);
  }
  return res;
}

/*** Example Usage (http://wcipeg.com/problem/rootsolve) ***/

#include <iostream>
using namespace std;

int main() {
  int n, d;
  Double c;
  poly p;
  cin >> n;
  for (int i = 0; i < n; i++) {
    cin >> c >> d;
    p.push_back(make_pair(c, d));
  }
  vector<Double> sol = find_all_roots(p);
  if (sol.empty()) {
    cout << "NO REAL ROOTS\n";
  } else {
    cout.precision(9);
    for (int i = 0; i < (int)sol.size(); i++)
      cout << fixed << sol[i] << "\n";
  }
  return 0;
}
