/*

1.4.1 - Cycle Detection (Floyd's Algorithm)

For a function f which maps a finite set S to itself and any initial
value x[0] in S, the same value must occur twice in the sequence below:
x[0], x[1] = f(x[0]), x[2] = f(x[1]), ..., x[i] = f(x[i - 1])
That is, there must exist numbers i, j (i < j) such that x[i] = x[j].
Once this happens, the sequence will continue periodically by repeating
the same sequence of values from x[i] to x[j − 1]. Cycle detection asks
to find i and j, given the function f and initial value x[0]. This is
also analogous to the problem of detecting a cycle in a linked list,
which will make it degenerate.

Floyd's cycle-finding algorithm, a.k.a. the "tortoise and the hare
algorithm", is a space-efficient algorithm that moves two pointers
through the sequence at different speeds. Each step in the algorithm
moves the "tortoise" one step forward and the "hare" two steps forward
in the sequence, comparing the sequence values at each step. The first
value which is simultaneously pointed to by both pointers is the start
of the sequence.

Time Complexity: O(mu + lambda), where mu is the smallest index of the
sequence on which a cycle starts, and lambda is the cycle's length.

Space Complexity: O(1) auxiliary.

*/

#include <utility> /* std::pair */

template<class IntFunction>
std::pair<int, int> find_cycle(IntFunction f, int x0) {
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
#include <set>
#include <iostream>
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
