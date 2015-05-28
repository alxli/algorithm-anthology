/*

3.2.6 - Binary Indexed Tree (Range Update with Range Query
                             with Co-ordinate Compression)

Description: Using maps for storing the trees, there no
longer needs to be a restriction on the magnitude of
queried indices. any value less than SIZE may be used
in the operations.

Time Complexity: query() and update() are O(log^2(N)).
If map is replaced with an unordered_map, the running time will
become O(log(N)) amortized.

Space Complexity: Equivalent to space complexity of std::map

*/

#include <map>

const int SIZE = 1 << 30;
std::map<int, int> dataMul, dataAdd;
 
void internal_update(int at, int mul, int add) {
  for (int i = at; i < SIZE; i = (i | (i+1))) {
    dataMul[i] += mul;
    dataAdd[i] += add;
  }
}

void update(int L, int H, int inc) {
  internal_update(L, inc, -inc*(L-1));
  internal_update(H, -inc, inc*H);
} /* add inc to each value in range [L,H] */

int query(int x) {
  int mul = 0, add = 0, start = x;
  for (int i = x; i >= 0; i = (i & (i + 1)) - 1) {
    if (dataMul.find(i) != dataMul.end())
      mul += dataMul[i];
    if (dataAdd.find(i) != dataAdd.end())
      add += dataAdd[i];
  }
  return mul*start + add;
} /* returns sum of range [0, x] */

int query(int L, int H) {
  return query(H) - query(L-1);
} /* returns sum of range [L, H] */

void update(int x, int v) {
  update(x, x, v - query(x, x));
} /* set value at index x to v */

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  update(500000001, 500000010, 3);
  update(500000011, 500000015, 5);
  update(500000000, 10);
  cout << query(500000000, 500000015) << endl; //65
  return 0;
}
