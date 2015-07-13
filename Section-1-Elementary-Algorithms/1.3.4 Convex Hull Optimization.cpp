/*

1.3.4 - Convex Hull Optimization

Given a set of lines in the form y = mx + b process queries
of x-coordinates to find the minimum y-value for any of the
lines when evaluated at x. For the following implementation,
the lines must be added in order of descending slope (m),
and queries must be called in nondecreasing order of x.
See: http://wcipeg.com/wiki/Convex_hull_optimization

Time Complexity: O(n log n) overall, where n is the number
of lines, for any number of calls to the two functions that
satisfy the preconditions.

Space Complexity: O(n) auxiliary on the number of lines.

*/

#include <vector>

std::vector<long long> M, B;
int ptr = 0;

//precondition: m must be the minimum of all lines added so far
void add_line(long long m, long long b) {
  int len = M.size();
  while (len >= 2 && (B[len - 2] - B[len - 1]) * (m - M[len - 1]) >=
                     (B[len - 1] - b) * (M[len - 1] - M[len - 2])) {
    len--;
  }
  M.resize(len);
  B.resize(len);
  M.push_back(m);
  B.push_back(b);
}

//precondition: x must be the maximum of all x queried so far
long long min_y(long long x) {
  if (ptr >= (int)M.size()) ptr = (int)M.size() - 1;
  while (ptr + 1 < (int)M.size() && M[ptr + 1] * x + B[ptr + 1] <=
                                    M[ptr] * x + B[ptr]) {
    ptr++;
  }
  return M[ptr] * x + B[ptr];
}

/*** Example Usage ***/

#include <cassert>

int main() {
  add_line(3, 0);
  add_line(2, 1);
  add_line(3, 2);
  add_line(0, 6);
  assert(min_y(0) == 0);
  assert(min_y(1) == 3);
  assert(min_y(2) == 5);
  assert(min_y(3) == 6);
  return 0;
}
