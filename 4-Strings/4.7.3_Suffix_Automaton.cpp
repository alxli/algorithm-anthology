/*

Maintains a compact deterministic automaton representing all substrings of a string. A suffix
automaton is useful for substring membership queries, counting distinct substrings, finding longest
common substrings, and many other string dynamic programming tasks.

Each state represents an equivalence class of substrings with the same set of ending positions. The
value `st[v].len` is the maximum length represented by state `v`, while `st[v].link` points to the
state representing the longest proper suffix of that class.

- `SuffixAutomaton()` constructs an empty automaton.
- `SuffixAutomaton(s)` constructs the automaton for string `s`.
- `extend(c)` appends character `c` to the current string.
- `contains(t)` returns whether string `t` occurs as a substring.
- `first_occurrence(t)` returns the starting index of the first occurrence of `t`, or $-1$ if absent.
- `count_distinct_substrings()` returns the number of distinct nonempty substrings.
- `longest_common_substring(t)` returns one longest substring common to the built string and string
  `t`.

Time Complexity:
- O(n) expected to construct the automaton for a string of length $n$.
- O(m) expected per call to `contains(t)`, `first_occurrence(t)`, or `longest_common_substring(t)`,
  where $m$ is the length of `t`.
- O(n) per call to `count_distinct_substrings()`.

Space Complexity:
- O(n) transition storage and O(n) states.
- O(1) auxiliary per character processed.

*/

#include <string>
#include <unordered_map>
#include <vector>
using std::string;

class SuffixAutomaton {
  struct State {
    int len, link, first_pos;
    std::unordered_map<char, int> next;

    State() : len(0), link(-1), first_pos(-1) {}
  };

  std::vector<State> st;
  int last;

 public:
  SuffixAutomaton() : st(1), last(0) {}

  explicit SuffixAutomaton(const string &s) : st(1), last(0) {
    for (char c : s) {
      extend(c);
    }
  }

  void extend(char c) {
    int cur = st.size();
    st.emplace_back();
    st[cur].len = st[last].len + 1;
    st[cur].first_pos = st[cur].len - 1;

    int p = last;
    while (p != -1 && st[p].next.find(c) == st[p].next.end()) {
      st[p].next[c] = cur;
      p = st[p].link;
    }
    if (p == -1) {
      st[cur].link = 0;
    } else {
      int q = st[p].next[c];
      if (st[p].len + 1 == st[q].len) {
        st[cur].link = q;
      } else {
        int clone = st.size();
        st.push_back(st[q]);
        st[clone].len = st[p].len + 1;
        while (p != -1 && st[p].next[c] == q) {
          st[p].next[c] = clone;
          p = st[p].link;
        }
        st[q].link = st[cur].link = clone;
      }
    }
    last = cur;
  }

  bool contains(const string &t) const {
    int v = 0;
    for (char c : t) {
      if (auto it = st[v].next.find(c); it != st[v].next.end()) {
        v = it->second;
      } else {
        return false;
      }
    }
    return true;
  }

  // Returns the starting index of the first (leftmost) occurrence of `t` as a substring, or -1 if
  // `t` does not occur. `first_pos` stores the end index of the first occurrence of each state's
  // class, so subtracting the length of `t` recovers its start.
  int first_occurrence(const string &t) const {
    int v = 0;
    for (char c : t) {
      auto it = st[v].next.find(c);
      if (it == st[v].next.end()) {
        return -1;
      }
      v = it->second;
    }
    return st[v].first_pos - static_cast<int>(t.size()) + 1;
  }

  long long count_distinct_substrings() const {
    long long res = 0;
    for (int v = 1; v < static_cast<int>(st.size()); v++) {
      res += st[v].len - st[st[v].link].len;
    }
    return res;
  }

  string longest_common_substring(const string &t) const {
    int v = 0, len = 0, best = 0, best_pos = -1;
    for (int i = 0; i < static_cast<int>(t.size()); i++) {
      while (v && !st[v].next.count(t[i])) {
        v = st[v].link;
        len = st[v].len;
      }
      if (auto it = st[v].next.find(t[i]); it != st[v].next.end()) {
        v = it->second;
        len++;
      } else {
        v = 0;
        len = 0;
      }
      if (len > best) {
        best = len;
        best_pos = i;
      }
    }
    return best == 0 ? "" : t.substr(best_pos - best + 1, best);
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  SuffixAutomaton sa("ababa");
  assert(sa.contains("aba"));
  assert(sa.contains("bab"));
  assert(!sa.contains("abba"));
  assert(sa.count_distinct_substrings() == 9);
  assert(sa.longest_common_substring("zzbabab") == "baba");
  assert(sa.first_occurrence("aba") == 0);
  assert(sa.first_occurrence("bab") == 1);
  assert(sa.first_occurrence("abba") == -1);

  SuffixAutomaton online;
  online.extend('a');
  online.extend('b');
  online.extend('c');
  assert(online.contains("bc"));
  return 0;
}
