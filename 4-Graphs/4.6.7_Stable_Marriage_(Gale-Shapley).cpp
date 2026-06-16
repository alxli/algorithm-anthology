/*

The stable marriage problem pairs $n$ men with $n$ women, given that every person ranks all members
of the opposite group in strict order of preference. A perfect matching is stable if no man and
woman both prefer each other to their assigned partners; such a pair would otherwise abandon their
matches. Unlike maximum matching, the goal is not the size of the matching (it is always perfect)
but its stability with respect to the preference lists.

The Gale-Shapley algorithm produces a stable matching, and a stable matching always exists. Each
free man proposes to the most preferred woman who has not yet rejected him. A woman provisionally
accepts her best proposal so far and rejects the rest, possibly breaking a previous engagement when
a man she prefers proposes later. Because every man descends his list at most once and each proposal
is constant work after precomputing the women's rankings, the algorithm runs in quadratic time. The
result is man-optimal: every man receives the best partner he could have in any stable matching.

Both preference lists are given as $n$ by $n$ matrices. `men_pref[m]` lists the women in `m`'s order
of preference, most preferred first, and `women_pref[w]` lists the men in `w`'s order of preference.

- `stable_marriage(men_pref, women_pref)` returns a vector `wife` where `wife[m]` is the woman
  matched to man `m` in the man-optimal stable matching.

Time Complexity:
- O(n^2) per call, where $n$ is the number of men (equivalently, women).

Space Complexity:
- O(n^2) auxiliary heap space for the precomputed rankings.

*/

#include <queue>
#include <vector>

std::vector<int> stable_marriage(
    const std::vector<std::vector<int>> &men_pref, const std::vector<std::vector<int>> &women_pref
) {
  int n = static_cast<int>(men_pref.size());
  std::vector<std::vector<int>> rank(n, std::vector<int>(n));
  for (int w = 0; w < n; w++) {
    for (int i = 0; i < n; i++) {
      rank[w][women_pref[w][i]] = i;  // How woman w ranks each man (smaller is better).
    }
  }
  std::vector<int> next_proposal(n, 0), husband(n, -1), wife(n, -1);
  std::queue<int> free_men;
  for (int m = 0; m < n; m++) {
    free_men.push(m);
  }
  while (!free_men.empty()) {
    int m = free_men.front();
    free_men.pop();
    int w = men_pref[m][next_proposal[m]++];
    if (husband[w] == -1) {
      husband[w] = m;
      wife[m] = w;
    } else if (rank[w][m] < rank[w][husband[w]]) {
      wife[husband[w]] = -1;
      free_men.push(husband[w]);
      husband[w] = m;
      wife[m] = w;
    } else {
      free_men.push(m);  // Rejected; m remains free and will propose to the next woman.
    }
  }
  return wife;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // All three men prefer w0 > w1 > w2; all three women prefer m2 > m1 > m0.
  vector<vector<int>> men_pref{{0, 1, 2}, {0, 1, 2}, {0, 1, 2}};
  vector<vector<int>> women_pref{{2, 1, 0}, {2, 1, 0}, {2, 1, 0}};

  // Cascading proposals leave m2 with w0, m1 with w1, m0 with w2.
  assert((stable_marriage(men_pref, women_pref) == vector<int>{2, 1, 0}));
  return 0;
}
