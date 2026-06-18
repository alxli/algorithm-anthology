/*

A sparse matrix stores only nonzero entries, which is useful when the matrix is large but most
values are zero. This implementation keeps both row maps and column maps in sync: `row(i)` can
iterate all nonzeros in row `i`, while `column(j)` can iterate all nonzeros in column `j`. That
bidirectional storage is especially useful for sparse elimination or graph-like matrix operations,
where both row updates and column pivot lookups are common.

- `SparseMatrix<T>(h, w)` constructs an `h` by `w` sparse matrix.
- `height()` returns the number of rows.
- `width()` returns the number of columns.
- `nonzeros()` returns the number of stored nonzero entries.
- `get(i, j)` returns the value at row `i`, column `j`, or 0 if the entry is not stored.
- `set(i, j, value)` assigns an entry. Assigning zero erases it from both maps.
- `add(i, j, delta)` adds `delta` to an entry. If the result becomes zero, the entry is erased.
- `row(i)` returns a map from column index to value for the nonzero entries in row `i`.
- `column(j)` returns a map from row index to value for the nonzero entries in column `j`.
- `swap_rows(i, j)` swaps two rows while keeping the column maps synchronized.
- `transpose()` transposes the matrix in place.
- `multiply_vector(x)` returns the matrix-vector product with vector `x`.
- `row_reduce(a, limit)` converts columns [0, `limit`) of `a` to sparse row echelon form, returning
  the rank found in those columns.
- `det(a)` returns the determinant of a square sparse matrix.
- `sparse_rank(a)` returns the rank of a sparse matrix.
- `solve_system(a, b, &x)` solves the system $ax = b$, returning 0 for one solution, $-1$ for no
  solution, or $-2$ for infinitely many solutions. When one solution exists, `x` is populated.

The class assumes that `T{}` is the additive zero and that `value == T{}` is a valid zero test. The
elimination routines also require field-like arithmetic because they divide by pivots, so use types
such as `double`, rational numbers, or modular integers. For floating-point matrices, replace
`is_zero()` with an epsilon comparison if small roundoff values should be erased.

Sparse elimination chooses, in each pivot column, the available pivot row with the fewest stored
entries. This heuristic can reduce fill-in, although sparse elimination can still become dense on
unfriendly inputs.

Time Complexity:
- O(log d) per call to `get()`, `set()`, and `add()`, where $d$ is the number of nonzeros in the
  touched row or column.
- O(n) per call to `transpose()` and `multiply_vector()`, where $n$ is the number of stored nonzero
  entries.
- Sparse elimination costs O(f * log d), where $f$ is the number of entry updates performed after
  fill-in and $d$ is a touched row or column size. In the worst case this is still cubic.
- O(f * log d) for `det()`, `sparse_rank()`, and `solve_system()`.
- O(1) per call to `height()`, `width()`, `nonzeros()`, `row()`, and `column()`.

Space Complexity:
- O(n) storage.
- O(1) auxiliary for `get()`, `set()`, `add()`, `row()`, `column()`, and `transpose()`.
- O(h) auxiliary heap space for `multiply_vector()`.

*/

#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <vector>

template<typename T>
class SparseMatrix {
  int h = 0, w = 0, stored = 0;
  std::vector<std::map<int, T>> rows, cols;

  bool is_zero(const T &value) const { return value == T{}; }

 public:
  SparseMatrix(int h_, int w_) : h(h_), w(w_), rows(h_), cols(w_) {}

  int height() const { return h; }
  int width() const { return w; }
  int nonzeros() const { return stored; }

  T get(int i, int j) const {
    auto it = rows[i].find(j);
    return it == rows[i].end() ? T{} : it->second;
  }

  void set(int i, int j, const T &value) {
    auto it = rows[i].find(j);
    if (is_zero(value)) {
      if (it != rows[i].end()) {
        rows[i].erase(it);
        cols[j].erase(i);
        stored--;
      }
      return;
    }
    if (it == rows[i].end()) {
      stored++;
    }
    rows[i][j] = value;
    cols[j][i] = value;
  }

  void add(int i, int j, const T &delta) {
    if (is_zero(delta)) {
      return;
    }
    set(i, j, get(i, j) + delta);
  }

  const std::map<int, T> &row(int i) const { return rows[i]; }
  const std::map<int, T> &column(int j) const { return cols[j]; }

  void swap_rows(int i, int j) {
    if (i == j) {
      return;
    }
    std::set<int> touched;
    for (const auto &[col, value] : rows[i]) {
      touched.insert(col);
    }
    for (const auto &[col, value] : rows[j]) {
      touched.insert(col);
    }
    for (int col : touched) {
      T x = get(i, col), y = get(j, col);
      set(i, col, y);
      set(j, col, x);
    }
  }

  void transpose() {
    std::swap(h, w);
    std::swap(rows, cols);
  }

  std::vector<T> multiply_vector(const std::vector<T> &x) const {
    assert(static_cast<int>(x.size()) == w);
    std::vector<T> res(h);
    for (int i = 0; i < h; i++) {
      for (const auto &[j, value] : rows[i]) {
        res[i] += value * x[j];
      }
    }
    return res;
  }
};

template<typename T>
int row_reduce(SparseMatrix<T> &a, int limit) {
  assert(0 <= limit && limit <= a.width());
  int r = 0;
  for (int c = 0; c < limit && r < a.height(); c++) {
    int pivot = -1;
    for (const auto &[i, value] : a.column(c)) {
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
    for (const auto &[i, value] : a.column(c)) {
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
  assert(a.height() == a.width());
  int swaps = 0, r = 0, n = a.height();
  T det = 1;
  for (int c = 0; c < n && r < n; c++) {
    int pivot = -1;
    for (const auto &[i, value] : a.column(c)) {
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
    for (const auto &[i, value] : a.column(c)) {
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
  return row_reduce(a, a.width());
}

template<typename T>
int solve_system(SparseMatrix<T> a, const std::vector<T> &b, std::vector<T> *x) {
  if (x == nullptr || a.height() != static_cast<int>(b.size())) {
    return -1;
  }
  int h = a.height(), w = a.width();
  SparseMatrix<T> aug(h, w + 1);
  for (int i = 0; i < h; i++) {
    for (const auto &[j, value] : a.row(i)) {
      aug.set(i, j, value);
    }
    aug.set(i, w, b[i]);
  }
  row_reduce(aug, w);
  std::vector<int> pivot_col;
  for (int i = 0; i < h; i++) {
    int lead = -1;
    for (const auto &[j, value] : aug.row(i)) {
      if (j < w) {
        lead = j;
        break;
      }
    }
    if (lead == -1) {
      if (aug.get(i, w) != T{}) {
        return -1;
      }
    } else {
      pivot_col.push_back(lead);
    }
  }
  if (static_cast<int>(pivot_col.size()) < w) {
    return -2;
  }
  x->assign(w, T{});
  for (int row = static_cast<int>(pivot_col.size()) - 1; row >= 0; row--) {
    int c = pivot_col[row];
    T rhs = aug.get(row, w);
    for (const auto &[j, value] : aug.row(row)) {
      if (j > c && j < w) {
        rhs -= value * (*x)[j];
      }
    }
    (*x)[c] = rhs / aug.get(row, c);
  }
  return 0;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  SparseMatrix<int64_t> a(3, 4);
  a.set(0, 1, 5);
  a.set(2, 3, 7);
  a.add(0, 1, -2);
  a.add(1, 2, 4);

  assert(a.height() == 3 && a.width() == 4);
  assert(a.nonzeros() == 3);
  assert(a.get(0, 1) == 3);
  assert(a.get(0, 0) == 0);
  assert(a.row(0).begin()->first == 1);
  assert(a.column(2).begin()->first == 1);

  vector<int64_t> x{10, 20, 30, 40};
  assert((a.multiply_vector(x) == vector<int64_t>{60, 120, 280}));

  a.add(0, 1, -3);  // Entries that become zero are removed from both row and column maps.
  assert(a.get(0, 1) == 0);
  assert(a.nonzeros() == 2);
  assert(a.row(0).empty());
  assert(a.column(1).empty());

  a.transpose();
  assert(a.height() == 4 && a.width() == 3);
  assert(a.get(3, 2) == 7);
  assert(a.get(2, 1) == 4);

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
  for (int i = 0; i < sys.height(); i++) {
    double sum = 0;
    for (const auto &[j, value] : sys.row(i)) {
      sum += value * sol[j];
    }
    assert(sum > rhs[i] - 1e-9 && sum < rhs[i] + 1e-9);
  }
  return 0;
}
