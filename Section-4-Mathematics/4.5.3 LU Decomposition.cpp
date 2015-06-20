/*

4.5.3 - LU Decomposition

The LU (lower upper) decomposition of a matrix is a
factorization of a matrix as the product of a lower
triangular matrix and an upper triangular matrix.
With the LU decomposition, we can solve many problems
including calculate the determinant of the matrix,
solving a systems of linear equations, and finding
the inverse of a matrix.

Note: in the following implementation, each call to
det() and inverse() recomputes the lu decomposition.
You do not have to do this, and can reuse the same
precomputed decomposition for multiply uses.

Complexity: O(N^3) for lu_decompose().

*/

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

static const double eps = 1e-10;
typedef std::vector<double> vd;
typedef std::vector<vd> vvd;

/*

LU decomposition with Gauss-Jordan elimination.
This is generalized for rectangular matrices.
Since the resulting L and U matrices have all
mutually exclusive 0's (except when i == j), we
can merge into a single lu matrix to save memory.

Optionally determine the permutation vector p.
If an array p is passed, p[i] will be populated
such that p[i] is the only column of the i-th
row of the permutation matrix that is equal to 1.

Returns: a merged lower/upper triangular matrix:
         m[i][j] = l[u][j] for i > j
         m[i][j] = u[i][j] for i <= j

Note that l[i][i] = 1 for all i.

*/

vvd lu_decompose(vvd a, int * detsign = 0, int * p = 0) {
  int n = a.size(), m = a[0].size();
  int sign = 1;
  if (p != 0)
    for (int i = 0; i < n; i++) p[i] = i;
  for (int r = 0, c = 0; r < n && c < m; r++, c++) {
    int pr = r;
    for (int i = r + 1; i < n; i++)
      if (fabs(a[i][c]) > fabs(a[pr][c]))
        pr = i;
    if (fabs(a[pr][c]) <= eps) {
      r--;
      continue;
    }
    if (pr != r) {
      if (p != 0) std::swap(p[r], p[pr]);
      sign = -sign;
      for (int i = 0; i < m; i++)
        std::swap(a[r][i], a[pr][i]);
    }
    for (int s = r + 1; s < n; s++) {
      a[s][c] /= a[r][c];
      for (int d = c + 1; d < m; d++)
        a[s][d] -= a[s][c] * a[r][d];
    }
  }
  if (detsign != 0) *detsign = sign;
  return a;
}

/*

Extract a value for the lower and upper parts given
indices (i, j) and a merged LU decomposition matrix.

*/

double getl(const vvd & lu, int i, int j) {
  if (i > j) return lu[i][j];
  return i < j ? 0.0 : 1.0;
}

double getu(const vvd & lu, int i, int j) {
  return i <= j ? lu[i][j] : 0.0;
}


/*

Finds the determinant in an additional O(n) time.
Precondition: A is square matrix.

*/

double det(const vvd & a) {
  int n = a.size(), detsign;
  assert(!a.empty() && n == a[0].size());
  vvd lu = lu_decompose(a, &detsign);
  double det = 1;
  for (int i = 0; i < n; i++)
    det *= lu[i][i];
  return detsign < 0 ? -det : det;
}


/*

Solves a system of linear equations using lu
decomposition and forward/backwards substitution.

Precondition: A must be n*n and B must be n*m.
Returns an n*m matrix X such that A*X = B.

*/

vvd solve_system(const vvd & a, const vvd & b) {
  int n = b.size(), m = b[0].size();
  assert(!a.empty() && n == a.size() && n == a[0].size());
  int detsign, p[a.size()];
  vvd lu = lu_decompose(a, &detsign, p);
  //forward substitute for Y in L*Y = B
  vvd y(n, vd(m));
  for (int j = 0; j < m; j++) {
    y[0][j] = b[p[0]][j] / getl(lu, 0, 0);
    for (int i = 1; i < n; i++) {
      double s = 0;
      for (int k = 0; k < i; k++)
        s += getl(lu, i, k) * y[k][j];
      y[i][j] = (b[p[i]][j] - s) / getl(lu, i, i);
    }
  }
  //backward substitute for X in U*X = Y
  vvd x(n, vd(m));
  for (int j = 0; j < m; j++) {
    x[n - 1][j] = y[n - 1][j] / getu(lu, n-1, n-1);
    for (int i = n - 2; i >= 0; i--) {
      double s = 0;
      for (int k = i + 1; k < n; k++)
        s += getu(lu, i, k) * x[k][j];
      x[i][j] = (y[i][j] - s) / getu(lu, i, i);
    }
  }
  return x;
}


/*

Find the inverse A^-1 of a matrix A. The inverse
of a matrix satisfies A * A^-1 = I, where I is the
identity matrix (for all pairs (i, j), I[i][j] = 1
iff i == j, otherwise I[i][j] = 0). The inverse of
a matrix exists if and only if det(a) is not 0.

We're lazy, so just generate the identity matrix I
and call solve_system().

Precondition: A is a square and det(A) != 0.

*/

vvd inverse(const vvd & a) {
  int n = a.size();
  assert(!a.empty() && n == a[0].size());
  vvd I(n, vd(n));
  for (int i = 0; i < n; i++) I[i][i] = 1;
  return solve_system(a, I);
}


/*** Example Usage ***/

#include <cstdio>
#include <iostream>
using namespace std;

void print(const vvd & m) {
  cout << "[";
  for (int i = 0; i < m.size(); i++) {
    cout << (i > 0 ? ",[" : "[");
    for (int j = 0; j < m[0].size(); j++)
      cout << (j > 0 ? "," : "") << m[i][j];
    cout << "]";
  }
  cout << "]\n";
}

void printlu(const vvd & lu) {
  printf("L:\n");
  for (int i = 0; i < lu.size(); i++) {
    for (int j = 0; j < lu[0].size(); j++)
      printf("%10.5lf ", getl(lu, i, j));
    printf("\n");
  }
  printf("U:\n");
  for (int i = 0; i < lu.size(); i++) {
    for (int j = 0; j < lu[0].size(); j++)
      printf("%10.5lf ", getu(lu, i, j));
    printf("\n");
  }
}

int main() {
  { //determinant of 3x3
    const int n = 3;
    double a[n][n] = {{1,3,5},{2,4,7},{1,1,0}};
    vvd v(n);
    for (int i = 0; i < n; i++)
      v[i] = vector<double>(a[i], a[i] + n);
    printlu(lu_decompose(v));
    cout << "determinant: " << det(v) << "\n"; //4
  }

  { //determinant of 4x4
    const int n = 4;
    double a[n][n] = {{11,9,24,2},{1,5,2,6},{3,17,18,1},{2,5,7,1}};
    vvd v(n);
    for (int i = 0; i < n; i++)
      v[i] = vector<double>(a[i], a[i] + n);
    printlu(lu_decompose(v));
    cout << "determinant: " << det(v) << "\n"; //284
  }

  { //solve for [x, y] in x + 3y = 4 && 2x + 3y = 6
    const int n = 2;
    double a[n][n] = {{1,3},{2,3}};
    double b[n] = {4, 6};
    vvd va(n), vb(n);
    for (int i = 0; i < n; i++) {
      va[i] = vector<double>(a[i], a[i] + n);
      vb[i] = vector<double>(1, b[i]);
    }
    vvd x = solve_system(va, vb);
    for (int i = 0; i < n; i++) {
      assert(fabs(a[i][0]*x[0][0] + a[i][1]*x[1][0] - b[i]) < eps);
    }
  }

  { //find inverse by solving a system
    const int n = 2;
    double a[n][n] = {{2,3},{1,2}};
    vvd v(n);
    for (int i = 0; i < n; i++)
      v[i] = vector<double>(a[i], a[i] + n);
    print(inverse(v)); //[[2,-3],[-1,2]]
  }
  return 0;
}
