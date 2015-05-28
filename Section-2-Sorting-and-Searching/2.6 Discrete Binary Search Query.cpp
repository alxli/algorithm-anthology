/*

2.6 - Discrete Binary Search Query

Not only can binary search be used on arrays (std::binary_search),
it can also be used on monotonic functions whose domain is the set
of integers. In this case we aren’t restricted by practical
quantities such as available memory, as was the case with arrays.

BSQ_min() returns the smallest value x in the range [lo, hi),
(i.e. including lo, but not including hi) for which the boolean
function query(x) is true. It can be used if and only if for all x
in the queried range, query(x) implies query(y) for all y > x.
In other words, the return value of query(x) should be false for
x < k (k is constant) and true for x >= k. BSQ_min() will find k.

BSQ_max() returns the greatest value x in the range [lo, hi) for
which the boolean function query(x) is true. It can be used if and
only if for all x in the range, query(x) implies query(y) for all y < x.
In other words, the return value of query(x) should be false for
x > k (k is constant) and true for x <= k. BSQ_max() will find k.

fBSQ_min() works like BSQ_min(), but on floating point query functions.
Since the set of reals numbers is dense, it is clear that we cannot
find the exact target. Instead, fBSQ_min() will return a value that is
very close to the border between false and true. Note that the number
of iterations performed depends on how precise we want the answer to be.
Although one can loop while (hi - lo > epsilon) until the distance
between hi and lo is really small, we can instead just follow the KISS
principle and just let the loop run for a few hundred iterations.
In most contest applications, 100 iterations is sufficient enough and
will reduce our search space to 10^-30 times its original size.

*/

template<class T, class UnaryPredicate>
T BSQ_min(T lo, T hi, UnaryPredicate query) {
  T mid, hi_backup = hi;
  while (lo < hi) {
    mid = lo + (hi - lo)/2;
    if (query(mid)) hi = mid;
    else lo = mid + 1;
  }
  if (!query(lo)) return hi_backup; //all false
  return lo;
}

template<class T, class UnaryPredicate>
T BSQ_max(T lo, T hi, UnaryPredicate query) {
  T mid, hi_backup = hi;
  while (lo < hi) {
    mid = lo + (hi - lo + 1)/2;
    if (query(mid)) lo = mid;
    else hi = mid - 1;
  }
  if (!query(lo)) return hi_backup; //all true
  return lo;
}

template<class T, class UnaryPredicate>
T fBSQ_min(T lo, T hi, UnaryPredicate query) {
  T mid;
  for (int reps = 0; reps < 100; reps++) {
  	mid = (lo + hi)/2.0;
  	//simply swap hi and lo for fBSQ_max()
    (query(mid) ? hi : lo) = mid;
  }
  return lo;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

//Define your own black-box query function here.
//Simple examples:
bool query1(int x) { return x >= 3; }
bool query2(int x) { return false; }
bool query3(int x) { return x <= 5; }
bool query4(int x) { return true; }
bool query5(double x) { return x >= 1.2345; }

int main() {
  cout << BSQ_min(0, 7, query1) << endl;         //3
  cout << BSQ_min(0, 7, query2) << endl;         //7
  cout << BSQ_max(0, 7, query3) << endl;         //5
  cout << BSQ_max(0, 7, query4) << endl;         //7
  cout << fBSQ_min(-10.0, 10.0, query5) << endl; //1.2345
  return 0;
}
