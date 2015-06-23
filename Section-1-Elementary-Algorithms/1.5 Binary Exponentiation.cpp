/*

1.5 - Binary Exponentiation

Exponentiation by squaring is a general method
for fast computation of large positive integer
powers of a number. This method is also known
as "square-and-multiply." The following version
computes x^n modulo m. Remove the "% m" if you
do not want the answer to be modded - but as
this causes overflow, you should then switch
to a bigger data type.

Complexity: O(M(x) log n), where M(x) is the
            complexity to multiply x.

*/

int pow(int x, int n, int m) {
  long long y = x;
  int res = 1;
  for (; n > 0; n >>= 1) {
    if (n & 1) res = res * y % m;
    y = y * y % m;
  }
  return res;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int x = pow(2, 10, 1000000007);
  cout << x << endl;
  return 0;
}
