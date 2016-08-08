/*

Given a set of pairs (m, b) specifying lines of the form y = mx + b, process a
set of x-coordinate queries each asking to find the minimum y-value when any of
the given lines are evaluated at the specified x. For each add_line(m, b) call,
m must be the minimum m of all lines added so far. For each get_min(x) call, x
must be the maximum x of all queries made so far.

The following implementation is a concise, semi-dynamic version of the convex
hull optimization technique. It supports an an interlaced sequence of add_line()
and get_min() calls, as long as the preconditions of descending m and ascending
x are satisfied. As a result, it may be necessary to sort the lines/queries
before calling the functions. In that case, the overall time complexity will be
dominated by the sorting step.

Explanation of technique: http://wcipeg.com/wiki/Convex_hull_trick

Time Complexity: O(n) on the total number of calls made to add_line(), for any
interlaced sequence of add_line() and get_min() calls. This is because the
overall number of steps taken by add_line() and get_min() are respectively
bounded by the number of lines added so far. As such, a single call to either
add_line() or get_min() will have an O(1) amortized running time.

Space Complexity: O(n) auxiliary on the number of calls made to add_line().

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

long long get_min(long long x) {
  if (ptr >= (int)M.size()) {
    ptr = (int)M.size() - 1;
  }
  while (ptr + 1 < (int)M.size() && M[ptr + 1]*x + B[ptr + 1] <=
                                    M[ptr]*x + B[ptr]) {
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
  assert(get_min(0) == 0);
  assert(get_min(1) == 3);
  assert(get_min(2) == 4);
  assert(get_min(3) == 5);
  return 0;
}
