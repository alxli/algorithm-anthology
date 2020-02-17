/*

Given a function f mapping a set of integers to itself and an x-coordinate in
the set, return a pair containing the (position, length) of a cycle in the
sequence of numbers obtained from repeatedly composing f with itself starting
with the initial x. Formally, since f maps a finite set S to itself, some value
is guaranteed to eventually repeat in the sequence:
  x[0], x[1]=f(x[0]), x[2]=f(x[1]), ..., x[n]=f(x[n - 1]), ...

There must exist a pair of indices i and j (i < j) such that x[i] = x[j]. When
this happens, the rest of the sequence will consist of the subsequence from x[i]
to x[j - 1] repeating indefinitely. The cycle detection problem asks to find
such an i, along with the length of the repeating subsequence. A well-known
special case is the problem of cycle-detection in a degenerate linked list.

Floyd's cycle-finding algorithm, a.k.a. the "tortoise and the hare algorithm",
is a space-efficient algorithm that moves two pointers through the sequence at
different speeds. Each step in the algorithm moves the "tortoise" one step
forward and the "hare" two steps forward in the sequence, comparing the sequence
values at each step. The first value which is simultaneously pointed to by both
pointers is the start of the sequence.

Time Complexity:
- O(m + n) per call to find_cycle_floyd(), where m is the smallest index of the
  sequence which is the beginning of a cycle, and n is the cycle's length.

Space Complexity:
- O(1) auxiliary.

*/

#include <utility>

template<class IntFunction>
std::pair<int, int> find_cycle_floyd(IntFunction f, int x0) {
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
using namespace std;

int f(int x) {
  return (123*x*x + 4567890) % 1337;
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
  pair<int, int> res = find_cycle_floyd(f, x0);
  assert(res == make_pair(4, 2));
  verify(x0, res.first, res.second);
  return 0;
}
