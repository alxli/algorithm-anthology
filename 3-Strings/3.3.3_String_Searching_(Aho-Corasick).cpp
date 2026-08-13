/*

Given a set of strings (needles) and subsequent queries of texts (haystacks) to be searched,
determine all positions in which needles occur within the given haystacks in linear time using the
Aho-Corasick algorithm. The needles are arranged in a trie whose nodes carry failure links to the
longest proper suffix that is also a trie node. Scanning the haystack once while following
transitions and failure links then reports every match through each node's output list.

This implementation stores transitions in hash tables, which keeps lookups expected constant time
without assuming a fixed alphabet size. Output lists store both needles ending at each trie node and
needles inherited through its failure link.

- `AhoCorasick(needles)` constructs the finite-state automaton for a set of needle strings that are
  to be searched for subsequently in haystack queries. Empty needles are ignored.
- `find_all_in(haystack)` returns a vector of (`needle_idx`, `pos`) pairs for all matches, where
  `needle_idx` is the 0-based index of a matched needle and `pos` is its starting position in
  `haystack`. Matches are ordered by increasing ending position in `haystack`.

Time Complexity:
- O(L + z) expected per call to the constructor, where $L$ is the total needle length and $z$ is the
  total number of inherited output entries.
- O(n + z) expected per call to `find_all_in(haystack)`, where $n$ is the length of `haystack` and
  $z$ is the number of matches.

Space Complexity:
- O(L + z) for storage of the automaton and inherited output entries.
- O(1) auxiliary and O(z) for the vector returned by `find_all_in()`.

*/

#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
using std::string;

class AhoCorasick {
  std::vector<string> needles;
  std::vector<int> fail;
  std::vector<std::unordered_map<char, int>> adj;
  std::vector<std::vector<int>> out;

  int next_state(int curr, char c) const {
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
      out[curr].push_back(i);
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
        out[v].insert(out[v].end(), out[f].begin(), out[f].end());
        q.push(v);
      }
    }
  }

  std::vector<std::pair<int, int>> find_all_in(const string &haystack) const {
    std::vector<std::pair<int, int>> matches;
    int state = 0;
    for (int i = 0; i < static_cast<int>(haystack.size()); i++) {
      state = next_state(state, haystack[i]);
      for (int idx : out[state]) {
        matches.emplace_back(idx, i - static_cast<int>(needles[idx].size()) + 1);
      }
    }
    return matches;
  }
};

/*** Example Usage ***/

#include <cassert>
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
  assert(
      (ac.find_all_in("abccab") ==
       vector<pair<int, int>>{{0, 0}, {1, 0}, {3, 1}, {5, 2}, {5, 3}, {0, 4}, {7, 0}, {1, 4}})
  );
  assert(ac.find_all_in("zzzz").empty());
  return 0;
}
