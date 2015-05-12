/*

2.6 - Binary Search Query 

BSQ_min() returns the smallest value x in the range [lo, hi),
i.e. including lo, but not including hi, for which the boolean
function query(x) is true. It can be used if and only if for all x
in the queried range, query(x) implies query(y) for all y > x.

BSQ_max() returns the greatest value x in the range [lo, hi) for
which the boolean function query(x) is true. It can be used if and
only if for all x in the range, query(x) implies query(y) for all y < x.

For both of the functions:
If all values in the range return false when queried, then hi is returned.

*/

int query(int x) {
    //insert your own query black-box function here:
    //a very simple example:
    return x >= 3;
}

template<class T, class UnaryPredicate>
T BSQ_min(T lo, T hi, UnaryPredicate query) {
    T mid, _hi = hi;
    while (lo < hi) {
        mid = lo + (hi - lo)/2;
        if (query(mid)) hi = mid;
        else lo = mid + 1;
    }
    if (!query(lo)) return _hi;
    return lo;
}

template<class T, class UnaryPredicate>
T BSQ_max(T lo, T hi, UnaryPredicate query) {
    T mid, _hi = hi;
    while (lo < hi) {
        mid = lo + (hi - lo + 1)/2;
        if (query(mid)) lo = mid;
        else hi = mid - 1;
    }
    if (!query(lo)) return _hi;
    return lo;
} 

int main() {
    cout << BSQ_min(0, 7, query) << endl;
    return 0;
}

/*

Examples:

x:        0 1 2 3 4 5 6      BSQ_min(0,7,query)
query(x): 0 0 0 1 1 1 1      >> returns 3

x:        0 1 2 3 4 5 6      BSQ_min(0,7,query)
query(x): 0 0 0 0 0 0 0      >> returns 7

x:        0 1 2 3 4 5 6      BSQ_max(0,7,query)
query(x): 1 1 1 1 1 1 0      >> returns 5

x:        0 1 2 3 4 5 6      BSQ_max(0,7,query)
query(x): 1 0 0 0 0 0 0      >> returns 0

*/
