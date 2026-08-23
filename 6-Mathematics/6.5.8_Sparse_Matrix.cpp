/*

Provides a sparse matrix type together with arithmetic and elimination routines. A sparse matrix
stores only nonzero entries, which is useful when the matrix is large but most values are zero.

The matrix keeps both row maps and column maps in sync: `row(i)` can iterate all nonzeros in row
`i`, while `col(j)` can iterate all nonzeros in column `j`. That bidirectional storage is especially
useful for sparse elimination or graph-like matrix operations, where both row updates and column
pivot lookups are common.

The class treats `T{}` as additive zero and requires `value == T{}` to be a valid zero test.

- `SparseMatrix<T>(rows, cols)` constructs a `rows` by `cols` sparse matrix.
- `num_rows()` returns the number of rows.
- `num_cols()` returns the number of columns.
- `nonzeros()` returns the number of stored nonzero entries.
- `get(i, j)` returns the value at row `i`, column `j`, or $0$ if the entry is not stored.
- `set(i, j, value)` assigns an entry. Assigning zero erases it from both maps.
- `add(i, j, delta)` adds `delta` to an entry. If the result becomes zero, the entry is erased.
- `row(i)` returns a map from column index to value for the nonzero entries in row `i`.
- `col(j)` returns a map from row index to value for the nonzero entries in column `j`.
- `swap_rows(i, j)` swaps two rows while keeping the column maps synchronized.
- `transpose()` transposes the matrix in place.
- `a * x` returns the matrix-vector product with vector `x`.
- `a + b` and `a - b` return the sum and difference of two matrices with identical dimensions.
- `a * b` returns the matrix product, requiring `a.num_cols()` to equal `b.num_rows()`. Only pairs
  of stored entries are ever multiplied.
- `a * k` returns `a` scaled by the value `k`. Scaling by zero yields an empty matrix.
- Operators `+=`, `-=`, and `*=` assign the corresponding result back into the left operand. In all
  of these operators, entries that cancel to zero are not stored.

Time Complexity:
- O(log d) per call to `get()`, `set()`, and `add()`, where $d$ is the number of nonzeros in the
  touched row or column.
- O(1) per call to `transpose()` and O(z) per matrix-vector product, where $z$ is the number of
  stored nonzero entries.
- O((r_i + r_j)*log d) per call to `swap_rows(i, j)`, where $r_i$ and $r_j$ are the sizes of the two
  rows.
- O((z_a + z_b)*log d) per call to `operator+` and `operator-`, where $z_a$ and $z_b$ are the
  operand nonzero counts.
- O(z*log d) per call to scalar `operator*`, where $z$ is the number of stored nonzero entries.
- O(f*log d) per call to `operator*`, where $f$ is the number of scalar products accumulated, that
  is, the sum of `b.row(k).size()` over every stored entry `a[i][k]`.
- O(1) per call to `num_rows()`, `num_cols()`, `nonzeros()`, `row()`, and `col()`.

Space Complexity:
- O(z) storage.
- O(1) auxiliary for `get()`, `set()`, `add()`, `row()`, `col()`, and `transpose()`.
- O(r_i + r_j) auxiliary for `swap_rows(i, j)`.
- O(m) for the vector returned by matrix-vector multiplication, where $m$ is the number of rows.
- O(c) auxiliary for `operator*`, where $c$ is the largest number of nonzeros in one result row.

*/

#include <cassert>
#include <map>
#include <unordered_set>
#include <utility>
#include <vector>

template<typename T>
class SparseMatrix {
  int rows = 0, cols = 0, stored = 0;
  std::vector<std::map<int, T>> rvals, cvals;

  bool is_zero(const T &value) const { return value == T{}; }

 public:
  SparseMatrix(int rows, int cols) : rows(rows), cols(cols) {
    assert(rows >= 0 && cols >= 0);
    rvals.resize(rows);
    cvals.resize(cols);
  }

  int num_rows() const { return rows; }
  int num_cols() const { return cols; }
  int nonzeros() const { return stored; }

  T get(int i, int j) const {
    auto it = rvals[i].find(j);
    return it == rvals[i].end() ? T{} : it->second;
  }

  void set(int i, int j, const T &value) {
    auto it = rvals[i].find(j);
    if (is_zero(value)) {
      if (it != rvals[i].end()) {
        rvals[i].erase(it);
        cvals[j].erase(i);
        stored--;
      }
      return;
    }
    if (it == rvals[i].end()) {
      stored++;
    }
    rvals[i][j] = value;
    cvals[j][i] = value;
  }

  void add(int i, int j, const T &delta) {
    if (is_zero(delta)) {
      return;
    }
    set(i, j, get(i, j) + delta);
  }

  const std::map<int, T> &row(int i) const { return rvals[i]; }
  const std::map<int, T> &col(int j) const { return cvals[j]; }

  void swap_rows(int i, int j) {
    if (i == j) {
      return;
    }
    std::unordered_set<int> touched;
    for (const auto &[col, value] : rvals[i]) {
      touched.insert(col);
    }
    for (const auto &[col, value] : rvals[j]) {
      touched.insert(col);
    }
    for (int col : touched) {
      T x = get(i, col), y = get(j, col);
      set(i, col, y);
      set(j, col, x);
    }
  }

  void transpose() {
    std::swap(rows, cols);
    std::swap(rvals, cvals);
  }

  std::vector<T> operator*(const std::vector<T> &x) const {
    assert(static_cast<int>(x.size()) == cols);
    std::vector<T> res(rows);
    for (int i = 0; i < rows; i++) {
      for (const auto &[j, value] : rvals[i]) {
        res[i] += value * x[j];
      }
    }
    return res;
  }

  SparseMatrix operator+(const SparseMatrix &b) const {
    assert(rows == b.rows && cols == b.cols);
    SparseMatrix res(rows, cols);
    for (int i = 0; i < rows; i++) {
      for (const auto &[j, value] : rvals[i]) {
        res.set(i, j, value);
      }
      for (const auto &[j, value] : b.rvals[i]) {
        res.add(i, j, value);  // Cancellation to zero erases the entry.
      }
    }
    return res;
  }

  SparseMatrix operator-(const SparseMatrix &b) const {
    assert(rows == b.rows && cols == b.cols);
    SparseMatrix res(rows, cols);
    for (int i = 0; i < rows; i++) {
      for (const auto &[j, value] : rvals[i]) {
        res.set(i, j, value);
      }
      for (const auto &[j, value] : b.rvals[i]) {
        res.add(i, j, T{} - value);
      }
    }
    return res;
  }

  // Scaling by zero cancels every entry, so the result stores nothing at all.
  SparseMatrix operator*(const T &k) const {
    SparseMatrix res(rows, cols);
    if (is_zero(k)) {
      return res;
    }
    for (int i = 0; i < rows; i++) {
      for (const auto &[j, value] : rvals[i]) {
        res.set(i, j, value * k);
      }
    }
    return res;
  }

  // Row-by-row product: each stored a[i][k] scatters row k of b into an accumulator for row i, so
  // only nonzero-by-nonzero pairs are ever multiplied.
  SparseMatrix operator*(const SparseMatrix &b) const {
    assert(cols == b.rows);
    SparseMatrix res(rows, b.cols);
    for (int i = 0; i < rows; i++) {
      std::map<int, T> acc;
      for (const auto &[k, aik] : rvals[i]) {
        for (const auto &[j, bkj] : b.rvals[k]) {
          acc[j] = acc[j] + aik * bkj;
        }
      }
      for (const auto &[j, value] : acc) {
        res.set(i, j, value);
      }
    }
    return res;
  }

  SparseMatrix &operator+=(const SparseMatrix &b) { return *this = *this + b; }
  SparseMatrix &operator-=(const SparseMatrix &b) { return *this = *this - b; }
  SparseMatrix &operator*=(const SparseMatrix &b) { return *this = *this * b; }
  SparseMatrix &operator*=(const T &k) { return *this = *this * k; }
};

/*

The elimination routines require field-like arithmetic because they divide by pivots, so use types
such as `double`, rational numbers, or modular integers. For floating-point matrices, replace
`is_zero()` with an epsilon comparison if small roundoff values should be erased. At each pivot
column, sparse elimination chooses the available row with the fewest stored entries to reduce
fill-in, although an unfriendly matrix can still become dense.

- `row_reduce(a, limit)` converts columns $[0, `limit`)$ of `a` to sparse row echelon form,
  returning the rank found in those columns.
- `det(a)` returns the determinant of a square sparse matrix.
- `sparse_rank(a)` returns the rank of a sparse matrix.
- `solve_system(a, b, &x)` solves the system $ax = b$, returning $0$ for one solution, $-1$ for no
  solution, or $-2$ for infinitely many solutions. When one solution exists, `x` is populated.

Time Complexity:
- O(f * log d) for sparse elimination, where $f$ is the number of entry updates performed after
  fill-in and $d$ is a touched row or column size. In the worst case this is still cubic.
- O(f * log d) per call to `det()`, `sparse_rank()`, and `solve_system()`.

Space Complexity:
- O(z + m) auxiliary for elimination, in addition to the copied matrix used by `det()` and
  `sparse_rank()` or the augmented matrix used by `solve_system()`.

*/

template<typename T>
int row_reduce(SparseMatrix<T> &a, int limit) {
  assert(0 <= limit && limit <= a.num_cols());
  int r = 0;
  for (int c = 0; c < limit && r < a.num_rows(); c++) {
    int pivot = -1;
    for (const auto &[i, value] : a.col(c)) {
      if (i >= r && (pivot == -1 || a.row(i).size() < a.row(pivot).size())) {
        pivot = i;
      }
    }
    if (pivot == -1) {
      continue;
    }
    a.swap_rows(r, pivot);
    T inv_pivot = T{1} / a.get(r, c);
    std::vector<std::pair<int, T>> pivot_row(a.row(r).begin(), a.row(r).end());
    std::vector<int> touched_rows;
    for (const auto &[i, value] : a.col(c)) {
      if (i > r) {
        touched_rows.push_back(i);
      }
    }
    for (int i : touched_rows) {
      T coeff = -a.get(i, c) * inv_pivot;
      for (const auto &[j, value] : pivot_row) {
        if (j != c) {
          a.add(i, j, coeff * value);
        }
      }
      a.set(i, c, T{});
    }
    r++;
  }
  return r;
}

template<typename T>
T det(SparseMatrix<T> a) {
  assert(a.num_rows() == a.num_cols());
  int swaps = 0, r = 0, n = a.num_rows();
  T det = 1;
  for (int c = 0; c < n && r < n; c++) {
    int pivot = -1;
    for (const auto &[i, value] : a.col(c)) {
      if (i >= r && (pivot == -1 || a.row(i).size() < a.row(pivot).size())) {
        pivot = i;
      }
    }
    if (pivot == -1) {
      return T{};
    }
    if (pivot != r) {
      a.swap_rows(r, pivot);
      swaps++;
    }
    T pivot_value = a.get(r, c);
    det *= pivot_value;
    T inv_pivot = T{1} / pivot_value;
    std::vector<std::pair<int, T>> pivot_row(a.row(r).begin(), a.row(r).end());
    std::vector<int> touched_rows;
    for (const auto &[i, value] : a.col(c)) {
      if (i > r) {
        touched_rows.push_back(i);
      }
    }
    for (int i : touched_rows) {
      T coeff = -a.get(i, c) * inv_pivot;
      for (const auto &[j, value] : pivot_row) {
        if (j != c) {
          a.add(i, j, coeff * value);
        }
      }
      a.set(i, c, T{});
    }
    r++;
  }
  return (r == n ? (swaps % 2 == 0 ? det : -det) : T{});
}

template<typename T>
int sparse_rank(SparseMatrix<T> a) {
  return row_reduce(a, a.num_cols());
}

template<typename T>
int solve_system(const SparseMatrix<T> &a, const std::vector<T> &b, std::vector<T> *x) {
  if (x == nullptr || a.num_rows() != static_cast<int>(b.size())) {
    return -1;
  }
  int rows = a.num_rows(), cols = a.num_cols();
  SparseMatrix<T> aug(rows, cols + 1);
  for (int i = 0; i < rows; i++) {
    for (const auto &[j, value] : a.row(i)) {
      aug.set(i, j, value);
    }
    aug.set(i, cols, b[i]);
  }
  row_reduce(aug, cols);
  std::vector<int> pivot_col;
  for (int i = 0; i < rows; i++) {
    int lead = -1;
    for (const auto &[j, value] : aug.row(i)) {
      if (j < cols) {
        lead = j;
        break;
      }
    }
    if (lead == -1) {
      if (aug.get(i, cols) != T{}) {
        return -1;
      }
    } else {
      pivot_col.push_back(lead);
    }
  }
  if (static_cast<int>(pivot_col.size()) < cols) {
    return -2;
  }
  x->assign(cols, T{});
  for (int row = static_cast<int>(pivot_col.size()) - 1; row >= 0; row--) {
    int c = pivot_col[row];
    T rhs = aug.get(row, cols);
    for (const auto &[j, value] : aug.row(row)) {
      if (j > c && j < cols) {
        rhs -= value * (*x)[j];
      }
    }
    (*x)[c] = rhs / aug.get(row, c);
  }
  return 0;
}

/*** Example Usage ***/

#include <cassert>
#include <cstdint>
using namespace std;

int main() {
  SparseMatrix<int64_t> a(3, 4);
  a.set(0, 1, 5);
  a.set(2, 3, 7);
  a.add(0, 1, -2);
  a.add(1, 2, 4);

  assert(a.num_rows() == 3 && a.num_cols() == 4);
  assert(a.nonzeros() == 3);
  assert(a.get(0, 1) == 3);
  assert(a.get(0, 0) == 0);
  assert(a.row(0).begin()->first == 1);
  assert(a.col(2).begin()->first == 1);

  vector<int64_t> x{10, 20, 30, 40};
  assert((a * x == vector<int64_t>{60, 120, 280}));

  a.add(0, 1, -3);  // Entries that become zero are removed from both row and column maps.
  assert(a.get(0, 1) == 0);
  assert(a.nonzeros() == 2);
  assert(a.row(0).empty());
  assert(a.col(1).empty());

  a.transpose();
  assert(a.num_rows() == 4 && a.num_cols() == 3);
  assert(a.get(3, 2) == 7);
  assert(a.get(2, 1) == 4);

  //  p = [1 2]   q = [0 3]   p + q = [1 5]   p * q = [4 3]
  //      [0 1]       [2 0]           [2 1]           [2 0]
  SparseMatrix<int64_t> p(2, 2), q(2, 2);
  p.set(0, 0, 1);
  p.set(0, 1, 2);
  p.set(1, 1, 1);
  q.set(0, 1, 3);
  q.set(1, 0, 2);
  SparseMatrix<int64_t> sum = p + q;
  assert(sum.get(0, 0) == 1 && sum.get(0, 1) == 5);
  assert(sum.get(1, 0) == 2 && sum.get(1, 1) == 1);
  SparseMatrix<int64_t> product = p * q;
  assert(product.get(0, 0) == 4 && product.get(0, 1) == 3);
  assert(product.get(1, 0) == 2 && product.get(1, 1) == 0);
  assert(product.nonzeros() == 3);  // The zero entry is never stored.

  // Cancelling entries are dropped rather than stored as explicit zeros.
  SparseMatrix<int64_t> negated(2, 2);
  negated.set(0, 0, -1);
  negated.set(0, 1, -2);
  negated.set(1, 1, -1);
  assert((p + negated).nonzeros() == 0);
  assert((p - p).nonzeros() == 0);

  SparseMatrix<int64_t> difference = p - q;
  assert(difference.get(0, 0) == 1 && difference.get(0, 1) == -1);
  assert(difference.get(1, 0) == -2 && difference.get(1, 1) == 1);

  SparseMatrix<int64_t> scaled = p * 3;
  assert(scaled.get(0, 0) == 3 && scaled.get(0, 1) == 6 && scaled.get(1, 1) == 3);
  assert((p * 0LL).nonzeros() == 0);

  SparseMatrix<int64_t> acc = p;
  acc += q;
  assert(acc.get(0, 1) == 5 && acc.get(1, 0) == 2);
  acc -= q;
  acc *= 2;
  assert(acc.get(0, 0) == 2 && acc.get(0, 1) == 4 && acc.get(1, 1) == 2);

  SparseMatrix<double> d(3, 3);
  d.set(0, 0, 2);
  d.set(0, 1, 1);
  d.set(1, 1, 3);
  d.set(2, 0, 1);
  d.set(2, 2, 4);
  double determinant = det(d);
  assert(determinant > 24 - 1e-9 && determinant < 24 + 1e-9);
  assert(sparse_rank(d) == 3);

  SparseMatrix<double> sys(3, 3);
  sys.set(0, 0, -1);
  sys.set(0, 1, 2);
  sys.set(0, 2, 5);
  sys.set(1, 0, 1);
  sys.set(1, 2, -6);
  sys.set(2, 0, -4);
  sys.set(2, 1, 2);
  sys.set(2, 2, 2);
  vector<double> rhs{3, 1, -2}, sol;
  assert(solve_system(sys, rhs, &sol) == 0);
  for (int i = 0; i < sys.num_rows(); i++) {
    double sum = 0;
    for (const auto &[j, value] : sys.row(i)) {
      sum += value * sol[j];
    }
    assert(sum > rhs[i] - 1e-9 && sum < rhs[i] + 1e-9);
  }
  return 0;
}
