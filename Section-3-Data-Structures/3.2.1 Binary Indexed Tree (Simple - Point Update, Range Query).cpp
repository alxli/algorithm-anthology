/*

3.2.1 - Binary Indexed Tree (Simple - Point Update, Range Query) 

Description: A binary indexed tree (a.k.a. Fenwick Tree or BIT)
is a data structure that allows for the sum of an arbitrary
range of values in an array to be dynamically queried in
logarithmic time. The operations has indices accessible in the
range [1...SIZE];

Time Complexity: query() and update() are O(log(N)). All other
functions are O(1).

Space Complexity: O(N) storage and O(N) auxiliary on the number
of elements in the array. 

*/

#include <iostream>
using namespace std;

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

int update_add(int x, int v) { //a[x] += v
    for (; x < SIZE; x += x & -x) bit[x] += v;
}

int update_set(int x, int v) { //a[x] = v
    int inc = v - a[x];
    a[x] = v;
    update_add(x, inc);
}

/*** Example Usage: ***/

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
