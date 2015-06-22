/*

1.2.1 - Discrete Binary Search

Not only can binary search be used on arrays (std::binary_search),
it can also be used on monotonic functions. In this case we aren't
restricted by practical quantities such as available memory, as is
the case with storing values explicitly in a sorted array.

binary_search_first_true() returns the smallest integer k in the
range [lo, hi) - (i.e. including lo, but excluding hi) for which
the boolean function query(k) is true. It can be used if and only
if there exists a constant k where the return value of query(x)
is false for all x < k and true for all x >= k.

binary_search_last_true() returns the greatest integer k in the
range [lo, hi) - (i.e. including lo, but excluding hi) for which
the boolean function query(k) is true. It can be used if and only
if there exists a constant k where the return value of query(x)
is true for all x <= k and false for all x > k.

Complexity: At most O(log N) calls to query(), where N is the
search space (the distance between lo and hi).

fbinary_search() works like binary_search_first_true(), but on
floating point query functions. Since the set of reals numbers is
dense, it is clear that we cannot find the exact target. Instead,
fbinary_search() will return a value that is very close to the
border between false and true. Note that the number of iterations
performed depends on how precise we want the answer to be. Although
one can loop while (hi - lo > epsilon) until the distance between
lo and hi is really small, we can instead just keep it simple and
let the loop run for a few hundred iterations. In most contest
applications, 100 iterations is more than sufficient, reducing the
search space to 10^-30 times its original size.

*/

//000[1]11
template<class Int, class UnaryPredicate>
Int binary_search_first_true(Int lo, Int hi, UnaryPredicate query) {
  Int mid, _hi = hi;
  while (lo < hi) {
    mid = lo + (hi - lo) / 2;
    if (query(mid)) hi = mid;
    else lo = mid + 1;
  }
  if (!query(lo)) return _hi; //all [lo, hi) is false
  return lo;
}

//11[1]000
template<class Int, class UnaryPredicate>
Int binary_search_last_true(Int lo, Int hi, UnaryPredicate query) {
  Int mid, _hi = hi;
  while (lo < hi) {
    mid = lo + (hi - lo + 1) / 2;
    if (query(mid)) lo = mid;
    else hi = mid - 1;
  }
  if (!query(lo)) return _hi; //all [lo, hi) is true
  return lo;
}

//000[1]11
template<class Double, class UnaryPredicate>
Double fbinary_search(Double lo, Double hi, UnaryPredicate query) {
  Double mid;
  for (int reps = 0; reps < 100; reps++) {
    mid = (lo + hi) / 2.0;
    //simply switch hi and lo in the line below to find last true
    (query(mid) ? hi : lo) = mid;
  }
  return lo;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

//Define your own black-box query function here. Simple examples:
bool query1(int x) { return x >= 3; }
bool query2(int x) { return false; }
bool query3(int x) { return x <= 5; }
bool query4(int x) { return true; }
bool query5(double x) { return x >= 1.2345; }

int main() {
  cout << binary_search_first_true(0, 7, query1) << "\n"; //3
  cout << binary_search_first_true(0, 7, query2) << "\n"; //7
  cout << binary_search_last_true(0, 7, query3)  << "\n"; //5
  cout << binary_search_last_true(0, 7, query4)  << "\n"; //7
  cout << fbinary_search(-10.0, 10.0, query5)    << "\n"; //1.2345
  return 0;
}
