/*

Given a set of strings (needles) and subsequent queries of texts (haystacks) to be searched,
determine all positions in which needles occur within the given haystacks in linear time using the
Aho-Corasick algorithm.

This implementation stores transitions in hash tables, which keeps lookups expected constant time
without assuming a fixed alphabet size. The output tables are stored as ordered sets so matches
ending at the same position are reported in deterministic needle order.

- `AhoCorasick(needles)` constructs the finite-state automaton for a set of needle strings that are
  to be searched for subsequently in haystack queries.
- `find_all_in(haystack, report_match)` calls the function `report_match(s, pos)` once on each
  occurrence of each needle that occurs in the `haystack`, where `pos` is the starting position in
  the `haystack` at which string `s` (a matched needle) occurs. The matches will be reported in
  increasing order of their ending positions within the `haystack`.

Time Complexity:
- O(m*l + z log m) expected per call to the constructor, where $m$ is the number of needles, $l$ is
  the maximum length of any needle, and $z$ is the total number of inherited output entries.
- O(n + z) expected per call to `find_all_in(haystack, report_match)`, where $n$ is the length of
  `haystack` and $z$ is the number of matches.

Space Complexity:
- O(m*l) for storage of the automaton, where $m$ is the number of needles and $l$ is the maximum
  length for any needle.
- O(1) auxiliary space per call to `find_all_in(haystack, report_match)`.

*/

#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
using std::string;

class AhoCorasick {
  std::vector<string> needles;
  std::vector<int> fail;
  std::vector<std::unordered_map<char, int>> adj;
  std::vector<std::set<int>> out;

  int next_state(int curr, char c) {
    int next = curr;
    while (adj[next].find(c) == adj[next].end()) {
      next = fail[next];
    }
    return adj[next][c];
  }

 public:
  explicit AhoCorasick(const std::vector<string> &needles) : needles(needles) {
    int total_len = 0;
    for (const auto &needle : needles) {
      total_len += needle.size();
    }
    fail.resize(total_len, -1);
    adj.resize(total_len);
    out.resize(total_len);
    int states = 1;
    for (int i = 0; i < static_cast<int>(needles.size()); i++) {
      int curr = 0;
      for (char c : needles[i]) {
        if (auto it = adj[curr].find(c); it != adj[curr].end()) {
          curr = it->second;
        } else {
          curr = adj[curr][c] = states++;
        }
      }
      out[curr].insert(i);
    }
    std::queue<int> q;
    for (auto &[c, v] : adj[0]) {
      if (v != 0) {
        fail[v] = 0;
        q.push(v);
      }
    }
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (auto &[c, v] : adj[u]) {
        int f = fail[u];
        while (adj[f].find(c) == adj[f].end()) {
          f = fail[f];
        }
        f = adj[f].find(c)->second;
        fail[v] = f;
        out[v].insert(out[f].begin(), out[f].end());
        q.push(v);
      }
    }
  }

  template<class Fn>
  void find_all_in(const string &haystack, Fn report_match) {
    int state = 0;
    for (int i = 0; i < static_cast<int>(haystack.size()); i++) {
      state = next_state(state, haystack[i]);
      for (int idx : out[state]) {
        report_match(needles[idx], i - static_cast<int>(needles[idx].size()) + 1);
      }
    }
  }
};

/*** Example Usage and Output:

Matched "a" at position 0.
Matched "ab" at position 0.
Matched "bc" at position 1.
Matched "c" at position 2.
Matched "c" at position 3.
Matched "a" at position 4.
Matched "ab" at position 4.
Matched "abccab" at position 0.

***/

#include <iostream>
using namespace std;

void report_match(const string &needle, int pos) {
  cout << "Matched \"" << needle << "\" at position " << pos << "." << endl;
}

int main() {
  vector<string> needles;
  needles.push_back("a");
  needles.push_back("ab");
  needles.push_back("bab");
  needles.push_back("bc");
  needles.push_back("bca");
  needles.push_back("c");
  needles.push_back("caa");
  needles.push_back("abccab");

  AhoCorasick(needles).find_all_in("abccab", report_match);
  return 0;
}
