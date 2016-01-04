/*

A suffix array SA of a string S[1..n] is a sorted array of indices of
all the suffixes of S ("abc" has suffixes "abc", "bc", and "c").
SA[i] contains the starting position of the i-th smallest suffix in S,
ensuring that for all 1 < i <= n, S[SA[i - 1], n] < S[A[i], n] holds.
It is a simple, space efficient alternative to suffix trees.
By binary searching on a suffix array, one can determine whether a
substring exists in a string in O(log n) time per query.

The longest common prefix array (LCP array) stores the lengths of the
longest common prefixes between all pairs of consecutive suffixes in
a sorted suffix array and can be found in O(n) given the suffix array.

The following algorithm uses a "gap" partitioning algorithm
explained here: http://stackoverflow.com/a/17763563

Time Complexity: O(n log^2 n) for suffix_array() and O(n) for
lcp_array(), where n is the length of the input string.

Space Complexity: O(n) auxiliary.

*/

#include <algorithm>
#include <string>
#include <vector>

std::vector<long long> rank2;

bool comp(const int & a, const int & b) {
  return rank2[a] < rank2[b];
}

std::vector<int> suffix_array(const std::string & s) {
  int n = s.size();
  std::vector<int> sa(n), rank(n);
  for (int i = 0; i < n; i++) {
    sa[i] = i;
    rank[i] = (int)s[i];
  }
  rank2.resize(n);
  for (int len = 1; len < n; len *= 2) {
    for (int i = 0; i < n; i++)
      rank2[i] = ((long long)rank[i] << 32) +
                 (i + len < n ? rank[i + len] + 1 : 0);
    std::sort(sa.begin(), sa.end(), comp);
    for (int i = 0; i < n; i++)
      rank[sa[i]] = (i > 0 && rank2[sa[i - 1]] == rank2[sa[i]]) ?
                    rank[sa[i - 1]] : i;
  }
  return sa;
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
