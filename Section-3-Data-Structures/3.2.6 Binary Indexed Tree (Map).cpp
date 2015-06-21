/*

3.2.6 - Binary Indexed Tree (Range Updates and Range Query
                             with Co-ordinate Compression)

Description: Using maps for storing the trees, there no
longer needs to be a restriction on the magnitude of
queried indices. Any value less than MAXN may be used
in the operations. This version is in essence the "swiss-
army knife" of binary indexed trees.

Time Complexity: All functions are O(log^2 MAXN). If the
std::map is replaced with an std::unordered_map, then the
running time will become O(log MAXN) amortized.

Space Complexity: Equal to space complexity of std::map.

*/

#include <map>

const int MAXN = 1 << 30;
std::map<int, int> bitmul, bitadd;

void _add(int at, int mul, int add) {
  for (int i = at; i < MAXN; i = (i | (i+1))) {
    bitmul[i] += mul;
    bitadd[i] += add;
  }
}

//a[i] += v for all i = lo..hi, inclusive
void add(int lo, int hi, int v) {
  _add(lo, v, -v * (lo - 1));
  _add(hi, -v, v * hi);
}

//returns sum(a[i] for i = 1..hi inclusive)
int sum(int hi) {
  int mul = 0, add = 0, start = hi;
  for (int i = hi; i >= 0; i = (i & (i + 1)) - 1) {
    if (bitmul.find(i) != bitmul.end())
      mul += bitmul[i];
    if (bitadd.find(i) != bitadd.end())
      add += bitadd[i];
  }
  return mul*start + add;
}

//returns sum(a[i] for i = lo..hi inclusive)
int sum(int lo, int hi) {
  return sum(hi) - sum(lo - 1);
}

//a[i] = v
void set(int i, int v) {
  add(i, i, v - sum(i, i));
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  add(500000001, 500000010, 3);
  add(500000011, 500000015, 5);
  set(500000000, 10);
  cout << sum(500000000, 500000015) << "\n"; //65
  return 0;
}
