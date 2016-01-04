/*

A suffix array SA of a string S[1, n] is a sorted array of indices of
all the suffixes of S ("abc" has suffixes "abc", "bc", and "c").
SA[i] contains the starting position of the i-th smallest suffix in S,
ensuring that for all 1 < i <= n, S[SA[i - 1], n] < S[A[i], n] holds.
It is a simple, space efficient alternative to suffix trees.
By binary searching on a suffix array, one can determine whether a
substring exists in a string in O(log n) time per query.

The longest common prefix array (LCP array) stores the lengths of the
longest common prefixes between all pairs of consecutive suffixes in
a sorted suffix array and can be found in O(n) given the suffix array.

The following implementation uses the sophisticated DC3/skew algorithm
by Karkkainen & Sanders (2003), using radix sort on integer alphabets
for linear construction. The function suffix_array(s, SA, n, K) takes
in s, an array [0, n - 1] of ints with n values in the range [1, K].
It stores the indices defining the suffix array into SA. The last value
of the input array s[n - 1] must be equal 0, the sentinel character. A
C++ wrapper function suffix_array(std::string) is implemented below it.

Time Complexity: O(n) for suffix_array() and lcp_array(), where n is
the length of the input string.

Space Complexity: O(n) auxiliary.

*/

inline bool leq(int a1, int a2, int b1, int b2) {
  return a1 < b1 || (a1 == b1 && a2 <= b2);
}

inline bool leq(int a1, int a2, int a3, int b1, int b2, int b3) {
  return a1 < b1 || (a1 == b1 && leq(a2, a3, b2, b3));
}

static void radix_pass(int * a, int * b, int * r, int n, int K) {
  int *c = new int[K + 1];
  for (int i = 0; i <= K; i++)
    c[i] = 0;
  for (int i = 0; i < n; i++)
    c[r[a[i]]]++;
  for (int i = 0, sum = 0; i <= K; i++) {
    int tmp = c[i];
    c[i] = sum;
    sum += tmp;
  }
  for (int i = 0; i < n; i++)
    b[c[r[a[i]]]++] = a[i];
  delete[] c;
}

void suffix_array(int * s, int * sa, int n, int K) {
  int n0 = (n + 2) / 3, n1 = (n + 1) / 3, n2 = n / 3, n02 = n0 + n2;
  int *s12 = new int[n02 + 3], *SA12 = new int[n02 + 3];
  s12[n02] = s12[n02 + 1] = s12[n02 + 2] = 0;
  SA12[n02] = SA12[n02 + 1] = SA12[n02 + 2] = 0;
  int *s0 = new int[n0], *SA0 = new int[n0];
  for (int i = 0, j = 0; i < n + n0 - n1; i++)
    if (i % 3 != 0) s12[j++] = i;
  radix_pass(s12 , SA12, s + 2, n02, K);
  radix_pass(SA12, s12 , s + 1, n02, K);
  radix_pass(s12 , SA12, s , n02, K);
  int name = 0, c0 = -1, c1 = -1, c2 = -1;
  for (int i = 0; i < n02; i++) {
    if (s[SA12[i]] != c0 || s[SA12[i] + 1] != c1 || s[SA12[i] + 2] != c2) {
      name++;
      c0 = s[SA12[i]];
      c1 = s[SA12[i] + 1];
      c2 = s[SA12[i] + 2];
    }
    if (SA12[i] % 3 == 1)
      s12[SA12[i] / 3] = name;
    else
      s12[SA12[i] / 3 + n0] = name;
  }
  if (name < n02) {
    suffix_array(s12, SA12, n02, name);
    for (int i = 0; i < n02; i++)
      s12[SA12[i]] = i + 1;
  } else {
    for (int i = 0; i < n02; i++)
      SA12[s12[i] - 1] = i;
  }
  for (int i = 0, j = 0; i < n02; i++)
    if (SA12[i] < n0)
      s0[j++] = 3 * SA12[i];
  radix_pass(s0, SA0, s, n0, K);
#define GetI() (SA12[t] < n0 ? SA12[t] * 3 + 1 : (SA12[t] - n0) * 3 + 2)
  for (int p = 0, t = n0 - n1, k = 0; k < n; k++) {
    int i = GetI(), j = SA0[p];
    if (SA12[t] < n0 ? leq(s[i], s12[SA12[t] + n0],s[j], s12[j/3]) :
        leq(s[i], s[i + 1], s12[SA12[t] - n0 + 1], s[j], s[j + 1], s12[j / 3 + n0])) {
      sa[k] = i;
      if (++t == n02)
        for (k++; p < n0; p++, k++)
          sa[k] = SA0[p];
    } else {
      sa[k] = j;
      if (++p == n0)
        for (k++; t < n02; t++, k++)
          sa[k] = GetI();
    }
  }
#undef GetI
  delete[] s12;
  delete[] SA12;
  delete[] SA0;
  delete[] s0;
}

#include <string>
#include <vector>

// C++ wrapper function
std::vector<int> suffix_array(const std::string & s) {
  int n = s.size();
  int *str = new int[n + 5], *sa = new int[n + 1];
  for (int i = 0; i < n + 5; i++) str[i] = 0;
  for (int i = 0; i < n; i++) str[i] = (int)s[i];
  suffix_array(str, sa, n + 1, 256);
  return std::vector<int>(sa + 1, sa + n + 1);
}

std::vector<int> lcp_array(const std::string & s,
                           const std::vector<int> & sa) {
  int n = sa.size();
  std::vector<int> rank(n), lcp(n - 1);
  for (int i = 0; i < n; i++)
    rank[sa[i]] = i;
  for (int i = 0, h = 0; i < n; i++) {
    if (rank[i] < n - 1) {
      int j = sa[rank[i] + 1];
      while (std::max(i, j) + h < n && s[i + h] == s[j + h])
        h++;
      lcp[rank[i]] = h;
      if (h > 0) h--;
    }
  }
  return lcp;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  string s("banana");
  vector<int> sa = suffix_array(s);
  vector<int> lcp = lcp_array(s, sa);
  int sa_ans[] = {5, 3, 1, 0, 4, 2};
  int lcp_ans[] = {1, 3, 0, 0, 2};
  assert(equal(sa.begin(), sa.end(), sa_ans));
  assert(equal(lcp.begin(), lcp.end(), lcp_ans));
  return 0;
}
