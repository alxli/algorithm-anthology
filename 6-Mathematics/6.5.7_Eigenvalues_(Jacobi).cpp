/*

An eigenvector of a square matrix is a direction the matrix does not rotate, only rescales, and the
scale factor is its eigenvalue. For a real symmetric matrix, the spectral theorem guarantees that a
full set of real eigenvalues exists and that eigenvectors for distinct eigenvalues are orthogonal,
so the matrix is diagonalized by an orthogonal change of basis.

The Jacobi eigenvalue algorithm builds that basis one rotation at a time. It repeatedly picks the
largest off-diagonal entry and applies a rotation in the plane of that entry's row and column,
choosing the angle that sets the entry to zero. A rotation is a similarity transform, so it
preserves the eigenvalues, and it also preserves the sum of squares of all entries; since zeroing an
entry removes its square from the off-diagonal total and moves it onto the diagonal, every rotation
strictly reduces the off-diagonal weight. Earlier zeros are partially refilled by later rotations,
but the total decays geometrically, and the matrix converges to a diagonal one whose entries are the
eigenvalues. Accumulating the rotations gives the eigenvectors.

- `jacobi_eigen(a, eps = 1e-12, iterations = 100)` returns the pair (`values`, `vectors`) for a real
  symmetric $n$ by $n$ matrix `a`. The eigenvalues in `values` are sorted in decreasing order, and
  `vectors[i]` is a unit eigenvector for `values[i]`, so the eigenvectors are rows rather than
  columns. Each iteration rotates away the currently largest off-diagonal entry, stopping early once
  every such entry is smaller than `eps`; if the `iterations` limit is reached first, the current
  approximation is returned. The matrix must be symmetric, which is not checked; an unsymmetric one
  silently produces nonsense.

A general matrix instead needs the roots of the characteristic polynomial of section 6.5.6, found
including complex ones by the Ehrlich-Aberth iteration of section 5.4.5, though accuracy degrades
with degree since polynomial roots are far more sensitive to coefficients than eigenvalues are to
the matrix. When only the dominant eigenvalue is wanted, power iteration finds it with no
decomposition at all, by repeatedly multiplying a random vector and renormalizing.

Time Complexity:
- O(I*n^2) per call for at most $I$ iterations, where $n$ is the matrix dimension. Each iteration
  searches O(n^2) entries for the largest off-diagonal one and applies one O(n) rotation.

Space Complexity:
- O(n^2) auxiliary, and O(n^2) for the returned eigenvectors.

*/

#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>
#include <vector>

using Matrix = std::vector<std::vector<double>>;

std::pair<std::vector<double>, Matrix> jacobi_eigen(
    Matrix a, double eps = 1e-12, int iterations = 100
) {
  int n = static_cast<int>(a.size());
  Matrix vectors(n, std::vector<double>(n));
  for (int i = 0; i < n; i++) {
    vectors[i][i] = 1;
  }
  while (iterations--) {
    int p = 0, q = 1;
    double largest = 0;
    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        if (std::fabs(a[i][j]) > largest) {
          largest = std::fabs(a[i][j]);
          p = i;
          q = j;
        }
      }
    }
    if (n < 2 || largest < eps) {
      break;
    }
    // Choose the rotation angle that zeroes a[p][q], using the numerically safe form of
    // tan(theta) rather than computing the angle itself.
    double theta = (a[q][q] - a[p][p]) / (2 * a[p][q]);
    double t = (theta >= 0 ? 1 : -1) / (std::fabs(theta) + std::sqrt(theta * theta + 1));
    double c = 1 / std::sqrt(t * t + 1), s = t * c;
    for (int k = 0; k < n; k++) {
      double akp = a[k][p], akq = a[k][q];
      a[k][p] = c * akp - s * akq;
      a[k][q] = s * akp + c * akq;
    }
    for (int k = 0; k < n; k++) {
      double apk = a[p][k], aqk = a[q][k];
      a[p][k] = c * apk - s * aqk;
      a[q][k] = s * apk + c * aqk;
      double vpk = vectors[p][k], vqk = vectors[q][k];
      vectors[p][k] = c * vpk - s * vqk;
      vectors[q][k] = s * vpk + c * vqk;
    }
  }
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int x, int y) { return a[x][x] > a[y][y]; });
  std::vector<double> values(n);
  Matrix sorted(n);
  for (int i = 0; i < n; i++) {
    values[i] = a[order[i]][order[i]];
    sorted[i] = vectors[order[i]];
  }
  return {values, sorted};
}

/*** Example Usage ***/

#include <cassert>
#include <cmath>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  // A symmetric matrix with eigenvalues 3, 2, and 1.
  Matrix a{{2, 0, -1}, {0, 2, 0}, {-1, 0, 2}};
  auto [values, vectors] = jacobi_eigen(a);
  assert(EQ(values[0], 3) && EQ(values[1], 2) && EQ(values[2], 1));

  for (int i = 0; i < 3; i++) {
    double norm = 0;
    for (int k = 0; k < 3; k++) {
      norm += vectors[i][k] * vectors[i][k];
    }
    assert(EQ(norm, 1));  // Eigenvectors come back normalized.
    for (int r = 0; r < 3; r++) {
      double image = 0;
      for (int k = 0; k < 3; k++) {
        image += a[r][k] * vectors[i][k];
      }
      assert(EQ(image, values[i] * vectors[i][r]));  // The defining equation a*v = lambda*v.
    }
  }

  // A diagonal matrix is already solved, and a 1-by-1 matrix is its own eigenvalue.
  auto diagonal = jacobi_eigen(Matrix{{5, 0}, {0, -4}});
  assert(EQ(diagonal.first[0], 5) && EQ(diagonal.first[1], -4));
  assert(EQ(jacobi_eigen(Matrix{{7}}).first[0], 7));
  return 0;
}
