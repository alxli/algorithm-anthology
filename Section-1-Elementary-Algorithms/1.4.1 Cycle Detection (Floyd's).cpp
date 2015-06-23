/*

1.4.1 - Cycle Detection (Floyd's Algorithm)

For a function f which maps a finite set S to itself
and any initial value x0 in S, the sequence of
iterated function values:

x_0, x_1 = f(x_0), x_2 = f(x_1), ... x_i = f(x_(i-1))

must eventually use the same value twice: there must
be some i <> j such that xi = xj. Once this happens,
the sequence must continue periodically, by repeating
the same sequence of values from x_i to x_(j−1). Cycle
detection asks to find i and j, given ƒ and x_0.

Floyd's cycle-finding algorithm, a.k.a. the "tortoise
and the hare algorithm", is a pointer algorithm that
uses only two pointers, moving through the sequence at
different speeds.

Time Complexity: O(lambda + mu) where lambda is the
length of the cycle and mu is the position of the first
repetition starting at x1.

Space Complexity: O(1)

*/

#include <utility> /* std::pair */

template<class IntFunction>
std::pair<int, int> floyd(IntFunction f, int x0) {
  int tortoise = f(x0), hare = f(f(x0));
  while (tortoise != hare) {
    tortoise = f(tortoise);
    hare = f(f(hare));
  }
  int start = 0;
  tortoise = x0;
  while (tortoise != hare) {
    tortoise = f(tortoise);
    hare = f(hare);
    start++;
  }
  int length = 1;
  hare = f(tortoise);
  while (tortoise != hare) {
    hare = f(hare);
    length++;
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
  pair<int, int> res = floyd(f, x0);
  cout << "Found cycle of length " << res.second;
  cout << " starting at " << res.first << ".\n";
  verify(x0, res.first, res.second);
  return 0;
}
