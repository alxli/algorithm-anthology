/*

GCD, LCM, Modular Inverse, Chinese Remainder Theorem

*/

#include <utility> /* std::pair */
#include <vector>

//C++98 does not have abs() declared for long long
template<class T> inline T _abs(const T & x) {
  return x < 0 ? -x : x;
}

//GCD using Euclid's algorithm - O(log(a + b))
template<class Int> Int gcd(Int a, Int b) {
  return b == 0 ? _abs(a) : gcd(b, a % b);
}

//non-recursive version
template<class Int> Int gcd2(Int a, Int b) {
  while (b != 0) {
    Int t = b;
    b = a % b;
    a = t;
  }
  return _abs(a);
}

template<class Int> Int lcm(Int a, Int b) {
  return _abs(a / gcd(a, b) * b);
}

//returns <gcd(a, b), <x, y>> such that gcd(a, b) = ax + by
template<class Int>
std::pair<Int, std::pair<Int, Int> > euclid(Int a, Int b) {
  Int x = 1, y = 0, x1 = 0, y1 = 1;
  //invariant: a = a * x + b * y, b = a * x1 + b * y1
  while (b != 0) {
    Int q = a / b, _x1 = x1, _y1 = y1, _b = b;
    x1 = x - q * x1;
    y1 = y - q * y1;
    b = a - q * b;
    x = _x1;
    y = _y1;
    a = _b;
  }
  return a > 0 ? std::make_pair(a, std::make_pair(x, y)) :
                 std::make_pair(-a, std::make_pair(-x, -y));
}

//recursive version
template<class Int>
std::pair<Int, std::pair<Int, Int> > euclid2(Int a, Int b) {
  if (b == 0) {
    return a > 0 ? std::make_pair(a, std::make_pair(1, 0)) :
                   std::make_pair(-a, std::make_pair(-1, 0));
  }
  std::pair<Int, std::pair<Int, Int> > r = euclid2(b, a % b);
  return std::make_pair(r.first, std::make_pair(r.second.second,
                    r.second.first - a / b * r.second.second));
}

/*

Modulo Operation - Euclidean Definition

The % operator in C/C++ returns the remainder of division (which
may be positive or negative) The true Euclidean definition of
modulo, however, defines the remainder to be always nonnegative.
For positive operators, % and mod are the same. But for negative
operands, they differ. The result here is consistent with the
Euclidean division algorithm.

e.g. -21 % 4 == -1 since -21 / 4 == -5 and 4 * -5 + (-1) == -21
      however, -21 mod 4 is equal to 3 because -21 + 4 * 6 is 3.

*/

template<class Int> Int mod(Int a, Int m) {
  Int r = (Int)(a % m);
  return r >= 0 ? r : r + m;
}

//returns x such that a * x = 1 (mod m)
//precondition: m > 0 && gcd(a, m) = 1
template<class Int> Int mod_inverse(Int a, Int m) {
  a = mod(a, m);
  return a == 0 ? 0 : mod((1 - m * mod_inverse(m % a, a)) / a, m);
}

//precondition: m > 0 && gcd(a, m) = 1
template<class Int> Int mod_inverse2(Int a, Int m) {
  return mod(euclid(a, m).second.first, m);
}

//returns a vector where i*v[i] = 1 (mod p) in O(p) time
//precondition: p is prime
std::vector<int> generate_inverses(int p) {
  std::vector<int> res(p);
  res[1] = 1;
  for (int i = 2; i < p; i++)
    res[i] = (p - (p / i) * res[p % i] % p) % p;
  return res;
}

/*

Chinese Remainder Theorem

Let r and s be positive integers which are relatively prime and
let a and b be any two integers. Then there exists an integer N
such that N = a (mod r) and N = b (mod s). Moreover, N is
uniquely determined modulo rs.

More generally, given a set of simultaneous congruences for
which all values in p[] are pairwise relative prime:

 x = a[i] (mod p[i]), for i = 1..n

the solution of the set of congruences is:

 x = a[1] * b[1] * (M/p[1]) + ... + a[n] * b[n] * (M/p[n]) (mod M)

where M = p[1] * p[2] ... * p[n] and the b[i] are determined for

 b[i] * (M/p[i]) = 1 (mod p[i]).

The following functions solves for this value of x, with the
first function computed using the method above while the
second function using a special case of Garner's algorithm.

http://e-maxx-eng.github.io/algebra/chinese-remainder-theorem.html

*/

long long simple_restore(int n, int a[], int p[]) {
  long long res = 0, m = 1;
  for (int i = 0; i < n; i++) {
    while (res % p[i] != a[i]) res += m;
    m *= p[i];
  }
  return res;
}

long long garner_restore(int n, int a[], int p[]) {
  int x[n];
  for (int i = 0; i < n; i++) x[i] = a[i];
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < i; j++)
      x[i] = mod_inverse((long long)p[j], (long long)p[i]) *
                         (long long)(x[i] - x[j]);
      x[i] = (x[i] % p[i] + p[i]) % p[i];
  }
  long long res = x[0], m = 1;
  for (int i = 1; i < n; i++) {
    m *= p[i - 1];
    res += x[i] * m;
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
using namespace std;

int main() {
  {
    srand(time(0));
    for (int steps = 0; steps < 10000; steps++) {
      int a = rand() % 200 - 10;
      int b = rand() % 200 - 10;
      int g1 = gcd(a, b), g2 = gcd2(a, b);
      assert(g1 == g2);
      if (g1 == 1 && b > 1) {
        int inv1 = mod_inverse(a, b);
        int inv2 = mod_inverse2(a, b);
        assert(inv1 == inv2 && mod(a * inv1, b) == 1);
      }
      pair<int, pair<int, int> > euc1 = euclid(a, b);
      pair<int, pair<int, int> > euc2 = euclid2(a, b);
      assert(euc1.first == g1 && euc1 == euc2);
      int x = euc1.second.first;
      int y = euc1.second.second;
      assert(g1 == a * x + b * y);
    }
  }

  {
    long long a = 6, b = 9;
    pair<int, pair<int, int> > r = euclid(6, 9);
    cout << r.second.first << " * (" << a << ")" << " + ";
    cout << r.second.second << " * (" << b << ") = gcd(";
    cout << a << "," << b << ") = " << r.first << "\n";
  }

  {
    int prime = 17;
    std::vector<int> res = generate_inverses(prime);
    for (int i = 0; i < prime; i++) {
      if (i > 0) assert(mod(i * res[i], prime) == 1);
      cout << res[i] << " ";
    }
    cout << "\n";
  }

  {
    int n = 3, a[] = {2, 3, 1}, m[] = {3, 4, 5};
    //solves for x in the simultaneous congruences:
    //x = 2 (mod 3)
    //x = 3 (mod 4)
    //x = 1 (mod 5)
    int x1 = simple_restore(n, a, m);
    int x2 = garner_restore(n, a, m);
    assert(x1 == x2);
    for (int i = 0; i < n; i++)
      assert(mod(x1, m[i]) == a[i]);
    cout << "Solution: " << x1 << "\n"; //11
  }

  return 0;
}
