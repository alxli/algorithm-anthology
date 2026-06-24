/*

Given a set of strings (needles) and subsequent queries of texts (haystacks) to be searched,
determine all positions in which needles occur within the given haystacks in linear time using the
Aho-Corasick algorithm. The needles are arranged in a trie whose nodes carry failure links to the
longest proper suffix that is also a trie node. Scanning the haystack once while following
transitions and failure links then reports every match through each node's output set.

This implementation stores transitions in hash tables, which keeps lookups expected constant time
without assuming a fixed alphabet size. The output tables are stored as ordered sets so matches
ending at the same position are reported in deterministic needle order.

- `AhoCorasick(needles)` constructs the finite-state automaton for a set of needle strings that are
  to be searched for subsequently in haystack queries. Empty needles are ignored.
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
    while (next != 0 && adj[next].find(c) == adj[next].end()) {
      next = fail[next];
    }
    auto it = adj[next].find(c);
    return (it != adj[next].end()) ? it->second : 0;
  }

 public:
  explicit AhoCorasick(const std::vector<string> &needles) : needles(needles) {
    int total_len = 0;
    for (const auto &needle : needles) {
      total_len += static_cast<int>(needle.size());
    }
    // The trie has at most total_len + 1 states: the root plus one per character when no prefixes
    // are shared. Sizing to total_len alone overflows for low-sharing needle sets (e.g. a single
    // needle, which still needs the root plus one node).
    int max_states = total_len + 1;
    fail.resize(max_states, -1);
    adj.resize(max_states);
    out.resize(max_states);
    int states = 1;
    for (int i = 0; i < static_cast<int>(needles.size()); i++) {
      if (needles[i].empty()) {
        continue;
      }
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
        while (f != 0 && adj[f].find(c) == adj[f].end()) {
          f = fail[f];
        }
        auto fit = adj[f].find(c);
        f = (fit != adj[f].end()) ? fit->second : 0;
        fail[v] = f;
        out[v].insert(out[f].begin(), out[f].end());
        q.push(v);
      }
    }
  }

  template<typename Fn>
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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <utility>
using namespace std;

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

  AhoCorasick ac(needles);
  vector<pair<string, int>> matches;
  ac.find_all_in("abccab", [&](const string &needle, int pos) {
    matches.push_back({needle, pos});
  });
  sort(matches.begin(), matches.end());
  assert((matches == vector<pair<string, int>>{
                         {"a", 0}, {"a", 4}, {"ab", 0}, {"ab", 4}, {"abccab", 0}, {"bc", 1},
                         {"c", 2}, {"c", 3}}));

  vector<pair<string, int>> none;
  ac.find_all_in("zzzz", [&](const string &needle, int pos) { none.push_back({needle, pos}); });
  assert(none.empty());

  ac.find_all_in("abccab", [](const string &needle, int pos) {
    cout << "Matched \"" << needle << "\" at position " << pos << "." << endl;
  });
  return 0;
}
