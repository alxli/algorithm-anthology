/*

4.3.2 - Generating Primes

The following are three methods to generate primes.
Although the latter two functions are theoretically
linear, the former function with the sieve of
Eratosthenes is still significantly the fastest even
for n under 1 billion, since its constant factor is
so much better because of its minimal arithmetic
operations. For this reason, it should be favored
over the other two algorithms in most contest
applications. For the computation of larger primes,
you should replace int with long long or an arbitrary
precision class.

*/

#include <cmath> /* ceil(), sqrt() */
#include <vector>

//Sieve of Eratosthenes in ~ O(n log log n)
//returns: a vector of all primes under n
std::vector<int> gen_primes(int n) {
  std::vector<bool> prime(n + 1, true);
  int sqrtn = (int)ceil(sqrt(n));
  for (int i = 2; i <= sqrtn; i++) {
    if (prime[i])
      for (int j = i * i; j <= n; j += i)
        prime[j] = false;
  }
  std::vector<int> res;
  for (int i = 2; i <= n; i++)
    if (prime[i]) res.push_back(i);
  return res;
}

//Technically O(n), but on -O2, this is about
//as fast as the above sieve for n = 100 million
std::vector<int> gen_primes_linear(int n) {
  std::vector<int> lp(n + 1), res;
  for (int i = 2; i <= n; i++) {
    if (lp[i] == 0) {
      lp[i] = i;
      res.push_back(i);
    }
    for (int j = 0; j < (int)res.size(); j++) {
      if (res[j] > lp[i] || i * res[j] > n)
        break;
      lp[i * res[j]] = res[j];
    }
  }
  return res;
}

//Sieve of Atkins in O(n), somewhat slow due to
//its heavier arithmetic compared to the above
std::vector<int> gen_primes_atkins(int n) {
  std::vector<bool> prime(n + 1, false);
  std::vector<int> res;
  prime[2] = true;
  prime[3] = true;
  int num, lim = ceil(sqrt(n));
  for (int x = 1; x <= lim; x++) {
    for (int y = 1; y <= lim; y++) {
      num = 4 * x * x + y * y;
      if (num <= n && (num % 12 == 1 || num % 12 == 5))
        prime[num] = true;
      num = 3 * x * x + y * y;
      if (num <= n && (num % 12 == 7))
        prime[num] = true;
      if (x > y) {
        num = (3 * x * x - y * y);
        if (num <= n && num % 12 == 11)
          prime[num] = true;
      }
    }
  }
  for (int i = 5; i <= lim; i++) {
    if (prime[i])
      for (int j = i * i; j <= n; j += i)
        prime[j] = false;
  }
  for (int i = 2; i <= n; i++)
    if (prime[i]) res.push_back(i);
  return res;
}

//Double sieve to find primes in [l, h]
//Approximately O(sqrt(h) * log log(h - l))
std::vector<int> gen_primes(int l, int h) {
  int sqrth = (int)ceil(sqrt(h));
  int sqrtsqrth = (int)ceil(sqrt(sqrth));
  std::vector<bool> prime1(sqrth + 1, true);
  std::vector<bool> prime2(h - l + 1, true);
  for (int i = 2; i <= sqrtsqrth; i++) {
    if (prime1[i])
      for (int j = i * i; j <= sqrth; j += i)
        prime1[j] = false;
  }
  for (int i = 2, n = h - l; i <= sqrth; i++) {
    if (prime1[i])
      for (int j = l / i * i - l; j <= n; j += i)
        if (j >= 0 && j + l != i)
          prime2[j] = false;
  }
  std::vector<int> res;
  for (int i = l > 1 ? l : 2; i <= h; i++)
    if (prime2[i - l]) res.push_back(i);
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <ctime>
#include <iostream>
using namespace std;

template<class It> void print(It lo, It hi) {
  while (lo != hi) cout << *(lo++) << " ";
  cout << "\n";
}

int main() {
  int pmax = 10000000;
  vector<int> p;
  time_t start;
  double delta;

  cout << "Generating primes up to " << pmax << "...\n";
  start = clock();
  p = gen_primes(pmax);
  delta = (double)(clock() - start)/CLOCKS_PER_SEC;
  cout << "gen_primes() took " << delta << "s.\n";

  start = clock();
  p = gen_primes_linear(pmax);
  delta = (double)(clock() - start)/CLOCKS_PER_SEC;
  cout << "gen_primes_linear() took " << delta << "s.\n";

  start = clock();
  p = gen_primes_atkins(pmax);
  delta = (double)(clock() - start)/CLOCKS_PER_SEC;
  cout << "gen_primes_atkins() took " << delta << "s.\n";

  cout << "Generated " << p.size() << " primes.\n";
  //print(p.begin(), p.end());

  for (int i = 0; i <= 1000; i++) {
    assert(gen_primes(i) == gen_primes_linear(i));
    assert(gen_primes(i) == gen_primes_atkins(i));
  }

  int l = 1000000000, h = 1000000500;
  cout << "Generating primes in [" << l << ", " << h << "]...\n";
  start = clock();
  p = gen_primes(l, h);
  delta = (double)(clock() - start)/CLOCKS_PER_SEC;
  cout << "Generated " << p.size() << " primes in " << delta << "s.\n";
  print(p.begin(), p.end());
  return 0;
}
