/*

1.3.4 - Convex Hull Trick (Semi-Dynamic)

Given a set of pairs (m, b) describing lines of the form y = mx + b,
process a set of x-coordinate queries each asking to find the minimum
y-value of any of the given lines when evaluated at the specified x.
The convex hull optimization technique first ignores all lines which
never take on the maximum at any x value, then sorts the rest in order
of descending slope. The intersection points of adjacent lines in this
sorted list form the upper envelope of a convex hull, and line segments
connecting these points always take on the minimum y-value. The result
can be split up into x-intervals each mapped to the line which takes on
the minimum in that interval. The intervals can be binary searched to
solve each query in O(log n) time on the number of lines.

Explanation: http://wcipeg.com/wiki/Convex_hull_trick

The following implementation is a concise, semi-dynamic version which
supports an an interlaced series of add line and query operations.
However, two key preconditions are that each call to add_line(m, b)
must have m as the minimum slope of all lines added so far, and each
call to get_min(x) must have x as the maximum x of all queries so far.
As a result, pre-sorting the lines and queries may be necessary (in
which case the running time will be that of the sorting algorithm).

Time Complexity: O(n) on the number of calls to add_line(). Since the
number of steps taken by add_line() and get_min() are both bounded by
the number of lines added so far, their running times are respectively
O(1) amortized.

Space Complexity: O(n) auxiliary on the number of calls to add_line().

*/

#include <vector>

std::vector<long long> M, B;
int ptr = 0;

void add_line(long long m, long long b) {
  int len = M.size();
  while (len > 1 && (B[len - 2] - B[len - 1]) * (m - M[len - 1]) >=
                    (B[len - 1] - b) * (M[len - 1] - M[len - 2])) {
    len--;
  }
  M.resize(len);
  B.resize(len);
  M.push_back(m);
  B.push_back(b);
}

long long get_min(long long x) {
  if (ptr >= (int)M.size())
    ptr = (int)M.size() - 1;
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
  add_line(1, 2);
  add_line(0, 6);
  assert(get_min(0) == 0);
  assert(get_min(1) == 3);
  assert(get_min(2) == 4);
  assert(get_min(3) == 5);
  return 0;
}
