/*

Given two strings, determine their minimum-cost alignment. An alignment of two
strings is a transformation of both strings by inserting gap characters '_' in
some way to make the final lengths equal. The total cost of an alignment given
a gap_cost (insertion or deletion cost) and a sub_cost (substitution, i.e.
mismatch cost) is gap_cost*(the number of gaps inserted across both strings),
plus sub_cost*(the number of indices at which the two aligned strings differ).

- align_sequences(s1, s2, gap_cost, sub_cost) returns a pair of aligned strings
  for strings s1 and s2, using a classic dynamic programming approach. This
  implementation first computes dp[i][j] (the cost of aligning the length i
  prefix of s1 with the length j prefix of s2) before following the path
  backwards to construct the answer. For gap_cost = sub_cost = 1, dp[n][m] will
  be the Levenshtein edit distance, where n and m are the lengths of s1 and
  s2, respectively.
- hirschberg_align_sequences(s1, s2, gap_cost, sub_cost) returns the sequence
  alignment of strings s1 and s2 using the more memory efficient Hirschberg's
  algorithm.

Time Complexity:
- O(n*m) per call to align_sequences(s1, s2) as well as
  hirschberg_align_sequences(s1, s2), where n and m are the lengths of s1 and
  s2, respectively.

Space Complexity:
- O(n*m) auxiliary heap space for align_sequences(s1, s2), where n and m are the
  lengths of s1 and s2, respectively.
- O(log max(n, m)) auxiliary stack space and O(min(n, m)) auxiliary heap space
  for hirschberg_align_sequences(s1, s2), where n and m are the lengths of s1
  and s2, respectively.

*/

#include <algorithm>
#include <string>
#include <vector>
#include <utility>
using std::string;

std::pair<string, string> align_sequences(
    const string &s1, const string &s2, int gap_cost = 1, int sub_cost = 1) {
  int n = s1.size(), m = s2.size();
  std::vector<std::vector<int> > dp(n + 1, std::vector<int>(m + 1, 0));
  for (int i = 0; i <= n; i++) {
    dp[i][0] = i*gap_cost;
  }
  for (int j = 0; j <= m; j++) {
    dp[0][j] = j*gap_cost;
  }
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= m; j++) {
      dp[i][j] = (s1[i - 1] == s2[j - 1]) ? dp[i - 1][j - 1] : std::min(
          dp[i - 1][j - 1] + sub_cost,
          std::min(dp[i - 1][j], dp[i][j - 1]) + gap_cost);
    }
  }
  string res1, res2;
  int i = n, j = m;
  while (i > 0 && j > 0) {
    if (s1[i - 1] == s2[j - 1] || dp[i][j] == dp[i - 1][j - 1] + sub_cost) {
      res1 += s1[--i];
      res2 += s2[--j];
    } else if (dp[i][j] == dp[i - 1][j] + gap_cost) {
      res1 += s1[--i];
      res2 += '_';
    } else if (dp[i][j] == dp[i][j - 1] + gap_cost) {
      res1 += '_';
      res2 += s2[--j];
    }
  }
  while (i > 0 || j > 0) {
    res1 += (i > 0) ? s1[--i] : '_';
    res2 += (j > 0) ? s2[--j] : '_';
  }
  std::reverse(res1.begin(), res1.end());
  std::reverse(res2.begin(), res2.end());
  return std::make_pair(res1, res2);
}

template<class It>
std::vector<int> row_cost(It lo1, It hi1, It lo2, It hi2,
                          int gap_cost, int sub_cost) {
  std::vector<int> res(std::distance(lo2, hi2) + 1), prev(res);
  for (It it1 = lo1; it1 != hi1; ++it1) {
    res.swap(prev);
    int i = 0;
    for (It it2 = lo2; it2 != hi2; ++it2) {
      res[i + 1] = (*it1 == *it2) ? prev[i] : std::min(prev[i] + sub_cost,
                                                       res[i] + gap_cost);
      i++;
    }
  }
  return res;
}

template<class It>
void hirschberg_rec(It lo1, It hi1, It lo2, It hi2,
                    string *res1, string *res2, int gap_cost, int sub_cost) {
  if (lo1 == hi1) {
    for (It it2 = lo2; it2 != hi2; ++it2) {
      *res1 += '_';
      *res2 += *it2;
    }
    return;
  }
  if (lo1 + 1 == hi1) {
    It pos = std::find(lo2, hi2, *lo1);
    bool insert = (pos == hi2) && (gap_cost*(hi2 - lo2 + 1) < sub_cost);
    if (lo2 == hi2 || insert) {
      *res1 += *lo1;
      *res2 += '_';
    }
    for (It it2 = lo2; it2 != hi2; ++it2) {
      *res1 += (pos == it2 || (!insert && it2 == lo2)) ? *lo1 : '_';
      *res2 += *it2;
    }
    return;
  }
  It mid1 = lo1 + (hi1 - lo1)/2;
  std::reverse_iterator<It> rlo1(hi1), rmid1(mid1), rlo2(hi2), rhi2(lo2);
  std::vector<int> fwd = row_cost(lo1, mid1, lo2, hi2, gap_cost, sub_cost);
  std::vector<int> rev = row_cost(rlo1, rmid1, rlo2, rhi2, gap_cost, sub_cost);
  It mid2 = lo2;
  int mincost = -1;
  for (int i = 0, j = (int)rev.size() - 1; i < (int)fwd.size(); i++, j--) {
    if (mincost < 0 || fwd[i] + rev[j] < mincost) {
      mincost = fwd[i] + rev[j];
      mid2 = lo2 + i;
    }
  }
  hirschberg_rec(lo1, mid1, lo2, mid2, res1, res2, gap_cost, sub_cost);
  hirschberg_rec(mid1, hi1, mid2, hi2, res1, res2, gap_cost, sub_cost);
}

std::pair<string, string> hirschberg_align_sequences(
    const string &s1, const string &s2, int gap_cost = 1, int sub_cost = 1) {
  if (s1.size() < s2.size()) {
    return hirschberg_align_sequences(s2, s1, gap_cost, sub_cost);
  }
  string res1, res2;
  hirschberg_rec(s1.begin(), s1.end(), s2.begin(), s2.end(), &res1, &res2,
                 gap_cost, sub_cost);
  return std::make_pair(res1, res2);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(align_sequences("AGGGCT", "AGGCA", 2, 3) ==
             make_pair(string("AGGGCT"), string("A_GGCA")));
  assert(hirschberg_align_sequences("AGGGCT", "AGGCA", 2, 3) ==
             make_pair(string("AGGGCT"), string("A_GGCA")));
  return 0;
}
