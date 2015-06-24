/*

1.4.2 - Cycle Detection (Brent's Algorithm)

For a function f which maps a finite set S to itself and any
initial value x0 in S, the sequence of iterated values:

x_0, x_1 = f(x_0), x_2 = f(x_1), ... x_i = f(x_(i-1))

must eventually use the same value twice: there must be some
i <> j such that xi = xj. Once this happens, the sequence
must continue periodically, by repeating the same sequence
of values from x_i to x_(j−1). Cycle detection asks to find
i and j, given the function f(x) and x_0.

Brent's cycle-finding algorithm is based on a different idea
than Floyd's: searching for the smallest power of two, 2^i
that is larger than both lambda and mu.

Time Complexity: O(lambda + mu), where lambda is the length
of the cycle and mu is the first index of x for which the
cycle starts to occur. Brent claims that, on average, his
cycle finding algorithm runs around 36% more quickly than
Floyd's and that it speeds up the Pollard rho algorithm by
around 24% (see mathematics chapter).

Space Complexity: O(1) auxiliary.

*/

#include <utility> /* std::pair */

//returns pair<mu, lambda> (as described above)
template<class IntFunction>
std::pair<int, int> brent(IntFunction f, int x0) {
  int power = 1, length = 1;
  int tortoise = x0, hare = f(x0);
  while (tortoise != hare) {
    if (power == length) {
      tortoise = hare;
      power *= 2;
      length = 0;
    }
    hare = f(hare);
    length++;
  }
  hare = x0;
  for (int i = 0; i < length; i++)
    hare = f(hare);
  int start = 0;
  tortoise = x0;
  while (tortoise != hare) {
    tortoise = f(tortoise);
    hare = f(hare);
    start++;
  }
  return std::make_pair(start, length);
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
#include <set>
using namespace std;

int f(int x) {
  return (123 * x * x + 4567890) % 1337;
}

void verify(int x0, int start, int length) {
  set<int> s;
  int x = x0;
  for (int i = 0; i < start; i++) {
    assert(!s.count(x));
    s.insert(x);
    x = f(x);
  }
  int startx = x;
  s.clear();
  for (int i = 0; i < length; i++) {
    assert(!s.count(x));
    s.insert(x);
    x = f(x);
  }
  assert(startx == x);
}

int main () {
  int x0 = 0;
  pair<int, int> res = brent(f, x0);
  cout << "Found cycle of length " << res.second;
  cout << " starting at x_" << res.first << ".\n";
  verify(x0, res.first, res.second);
  return 0;
}
