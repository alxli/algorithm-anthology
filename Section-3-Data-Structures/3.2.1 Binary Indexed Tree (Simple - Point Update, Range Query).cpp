/*

3.2.1 - Binary Indexed Tree (Simple - Point Update, Range Query)

Description: A binary indexed tree (a.k.a. Fenwick Tree or BIT)
is a data structure that allows for the sum of an arbitrary
range of values in an array to be dynamically queried in
logarithmic time. Note that unlike the OOP versions of BIT's
provided in this anthology, the operations work for 1-based
indices only (i.e. between 1 and SIZE, inclusive).

Time Complexity: query() and update() are O(log(N)). All other
functions are O(1).

Space Complexity: O(N) storage and O(N) auxiliary on the number
of elements in the array.

*/

const int SIZE = 1000;
int a[SIZE+1], bit[SIZE+1];

int query_pre(int x) { //sum(a[1..x])
    int ret = 0;
    for (; x > 0; x -= x & -x) ret += bit[x];
    return ret;
}

int query(int x, int y) { //sum(a[x..y])
    return query_pre(y) - query_pre(x - 1);
}

int update_inc(int x, int v) { //a[x] += v
    for (; x <= SIZE; x += x & -x) bit[x] += v;
}

int update(int x, int v) { //a[x] = v
    int inc = v - a[x];
    a[x] = v;
    update_inc(x, inc);
}

/*** Example Usage: ***/

#include <iostream>
using namespace std;

int main() {
    for (int i = 1; i <= 5; i++) update(i, i);
    cout << "BIT values: ";
    for (int i = 1; i <= 5; i++)
      cout << query(i, i) << " ";
    cout << "\nSum of range [1,3] is ";
    cout << query(1, 3) << ".\n";
    return 0;
} 

/*

=~=~=~=~= Sample Output =~=~=~=~=
BIT values: 1 2 3 4 5
Sum of range [1,3] is 6.

*/
