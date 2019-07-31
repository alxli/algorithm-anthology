/*

Given a set of pairs (m, b) specifying lines of the form y = mx + b, process a
set of x-coordinate queries each asking to find the minimum y-value when any of
the given lines are evaluated at the specified x. For each add_line(m, b) call,
m must be the minimum m of all lines added so far. For each query(x) call, x
must be the maximum x of all queries made so far.

The following implementation is a concise, semi-dynamic version of the convex
hull optimization technique. It supports an an interlaced sequence of add_line()
and query() calls, as long as the preconditions of descending m and ascending x
are satisfied. As a result, it may be necessary to sort the lines and queries
before calling the functions. In that case, the overall time complexity will be
dominated by the sorting step.

Time Complexity:
- O(n) for any interlaced sequence of add_line() and query() calls, where n is
  the number of lines added. This is because the overall number of steps taken
  by add_line() and query() are respectively bounded by the number of lines.
  Thus a single call to either add_line() or query() will have an amortized O(1)
  running time.

Space Complexity:
- O(n) for storage of the lines.
- O(1) auxiliary for add_line() and query().

*/

#include <vector>

std::vector<long long> M, B;
int ptr = 0;

void add_line(long long m, long long b) {
  int len = M.size();
  while (len > 1 && (B[len - 2] - B[len - 1])*(m - M[len - 1]) >=
                    (B[len - 1] - b)*(M[len - 1] - M[len - 2])) {
    len--;
  }
  M.resize(len);
  B.resize(len);
  M.push_back(m);
  B.push_back(b);
}

long long query(long long x) {
  if (ptr >= (int)M.size()) {
    ptr = (int)M.size() - 1;
  }
  while (ptr + 1 < (int)M.size() &&
         M[ptr + 1]*x + B[ptr + 1] <= M[ptr]*x + B[ptr]) {
    ptr++;
  }
  return M[ptr]*x + B[ptr];
}

/*** Example Usage ***/

#include <cassert>

int main() {
  add_line(3, 0);
  add_line(2, 1);
  add_line(1, 2);
  add_line(0, 6);
  assert(query(0) == 0);
  assert(query(1) == 3);
  assert(query(2) == 4);
  assert(query(3) == 5);
  return 0;
}
