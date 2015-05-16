/*

3.2.5 - Binary Indexed Tree (Range Update with Point Query)

Description: Range updating can only increment values in a range,
not set them to the same value. This version is a very concise
implementation of a BIT if all that's required is point queries.

Time Complexity: query() and update() are O(log(N)).

Space Complexity: O(N) on the number of elements in the array.

*/

const int SIZE = 1000;
int bit[SIZE+1];

void internal_update(int i, int inc) {
    for (i++; i <= SIZE; i += i & -i) bit[i] += inc;
}

void update(int L, int H, int inc) {
    internal_update(L, inc);
    internal_update(H+1, -inc);
} /* add inc to each value in range [L,H] */

int query(int i) {
    int sum = 0;
    for (i++; i > 0; i -= i & -i) sum += bit[i];
    return sum;
} /* returns value at index i */

/*** Example Usage: ***/

#include <iostream>
using namespace std;

int main() {
    for (int i = 1; i <= 5; i++) update(i, i, i);
    cout << "BIT values: ";
    for (int i = 1; i <= 5; i++)
      cout << query(i) << " ";
    return 0;
} 

/*
=~=~=~=~= Sample Output =~=~=~=~=
BIT values: 1 2 3 4 5
*/
