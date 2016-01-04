/*

For a function f which maps a finite set S to itself and any initial
value x[0] in S, the same value must occur twice in the sequence below:

  x[0], x[1] = f(x[0]), x[2] = f(x[1]), ..., x[i] = f(x[i - 1])

That is, there must exist numbers i, j (i < j) such that x[i] = x[j].
Once this happens, the sequence will continue periodically by repeating
the same sequence of values from x[i] to x[j − 1]. Cycle detection asks
to find i and j, given the function f and initial value x[0]. This is
also analogous to the problem of detecting a cycle in a linked list,
which will make it degenerate.

While Floyd's cycle-finding algorithm finds cycles by simultaneously
moving two pointers at different speeds, Brent's algorithm keeps the
tortoise pointer stationary and "teleports" it to the hare pointer
every power of two. The smallest power of two for which they meet is
the start of the first cycle. This improves upon the constant factor
of Floyd's algorithm by reducing the number of function calls.

Time Complexity: O(mu + lambda), where mu is the smallest index of the
sequence on which a cycle starts, and lambda is the cycle's length.

Space Complexity: O(1) auxiliary.

*/


#include <utility> /* std::pair */

template<class IntFunction>
std::pair<int, int> find_cycle(IntFunction f, int x0) {
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
#include <set>
using namespace std;

const int x0 = 0;

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
  pair<int, int> res = find_cycle(f, x0);
  assert(res == make_pair(4, 2));
  verify(x0, res.first, res.second);
  return 0;
}
