/*

4.5.2 - Determinant (Gauss's Method)

The following are ways to compute the determinant of a
matrix directly using Gaussian elimination. See the
following section for a generalized solution using LU
decompositions. Since the determinant can get very large,
look out for overflows and floating-point inaccuracies.
Bignums are recommended for maximal correctness.

Complexity: O(N^3), except for the adjustment for
overflow in the integer det() function.

Precondition: All input matrices must be square.

*/

#include <algorithm> /* std::swap() */
#include <cassert>
#include <cmath>     /* fabs() */
#include <map>
#include <vector>

static const double eps = 1e-10;
typedef std::vector<std::vector<int> > vvi;
typedef std::vector<std::vector<double> > vvd;

double det(vvd a) {
  int n = a.size();
  assert(!a.empty() && n == a[0].size());
  double res = 1;
  std::vector<bool> used(n, false);
  for (int i = 0; i < n; i++) {
    int p;
    for (p = 0; p < n; p++)
      if (!used[p] && fabs(a[p][i]) > eps)
        break;
    if (p >= n) return 0;
    res *= a[p][i];
    used[p] = true;
    double z = 1 / a[p][i];
    for (int j = 0; j < n; j++) a[p][j] *= z;
    for (int j = 0; j < n; j++) {
      if (j == p) continue;
      z = a[j][i];
      for (int k = 0; k < n; k++)
        a[j][k] -= z * a[p][k];
    }
  }
  return res;
}

/*

Determinant of Integer Matrix

This is prone to overflow, so it is recommended you use your
own bigint class instead of long long. At the end of this
function, the final answer is found as a product of powers.
You have two choices: change the "#if 0" to "#if 1" and use
the naive method to compute this product and risk overflow,
or keep it as "#if 0" and try to make the situation better
through prime factorization (less efficient). Note that
even in the prime factorization method, overflow may happen
if the final answer is too big for a long long.

*/

//C++98 doesn't have an abs() for long long
template<class T> inline T _abs(const T & x) {
  return x < 0 ? -x : x;
}

long long det(const vvi & a) {
  int n = a.size();
  assert(!a.empty() && n == a[0].size());
  long long b[n][n], det = 1;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++) b[i][j] = a[i][j];
  int sign = 1, exponent[n];
  for (int i = 0; i < n; i++) {
    exponent[i] = 0;
    int k = i;
    for (int j = i + 1; j < n; j++) {
      if (b[k][i] == 0 || (b[j][i] != 0 && _abs(b[k][i]) > _abs(b[j][i])))
        k = j;
    }
    if (b[k][i] == 0) return 0;
    if (i != k) {
      sign = -sign;
      for (int j = 0; j < n; j++)
        std::swap(b[i][j], b[k][j]);
    }
    exponent[i]++;
    for (int j = i + 1; j < n; j++)
      if (b[j][i] != 0) {
        for (int p = i + 1; p < n; ++p)
          b[j][p] = b[j][p] * b[i][i] - b[i][p] * b[j][i];
        exponent[i]--;
      }
  }

#if 0
  for (int i = 0; i < n; i++)
    for (; exponent[i] > 0; exponent[i]--)
      det *= b[i][i];
  for (int i = 0; i < n; i++)
    for (; exponent[i] < 0; exponent[i]++)
      det /= b[i][i];
#else
  std::map<long long, int> m;
  for (int i = 0; i < n; i++) {
    long long x = b[i][i];
    for (long long d = 2; ; d++) {
      long long power = 0, quo = x / d, rem = x - quo * d;
      if (d > quo || (d == quo && rem > 0)) break;
      for (; rem == 0; rem = x - quo * d) {
        power++;
        x = quo;
        quo = x / d;
      }
      if (power > 0) m[d] += power * exponent[i];
    }
    if (x > 1) m[x] += exponent[i];
  }
  std::map<long long, int>::iterator it;
  for (it = m.begin(); it != m.end(); ++it)
    for (int i = 0; i < it->second; i++)
      det *= it->first;
#endif

  return sign < 0 ? -det : det;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  const int n = 3;
  int a[n][n] = {{6,1,1},{4,-2,5},{2,8,7}};
  vvi v1(n);
  vvd v2(n);
  for (int i = 0; i < n; i++) {
    v1[i] = vector<int>(a[i], a[i] + n);
    v2[i] = vector<double>(a[i], a[i] + n);
  }
  int d1 = det(v1);
  int d2 = (int)det(v2);
  assert(d1 == d2 && d2 == -306);
  return 0;
}
