/*

Maintains a compact deterministic automaton representing all substrings of a
string. A suffix automaton is useful for substring membership queries, counting
distinct substrings, finding longest common substrings, and many other string
dynamic programming tasks.

Each state represents an equivalence class of substrings with the same set of
ending positions. The value `st[v].len` is the maximum length represented by state
`v`, while `st[v].link` points to the state representing the longest proper suffix
of that class.

- `suffix_automaton()` constructs an empty automaton.
- `suffix_automaton(s)` constructs the automaton for string `s`.
- `extend(c)` appends character `c` to the current string.
- `contains(t)` returns whether string `t` occurs as a substring.
- `count_distinct_substrings()` returns the number of distinct nonempty substrings.
- `longest_common_substring(t)` returns one longest substring common to the built
  string and string `t`.

Time Complexity:
- O(n log A) to construct the automaton for a string of length $n$, where $A$ is
  the alphabet size, due to ordered map transitions.
- O(m log A) per call to `contains(t)` or `longest_common_substring(t)`, where $m$
  is the length of `t`.
- O(n) per call to `count_distinct_substrings()`.

Space Complexity:
- O(n) transition storage and O(n) states.
- O(1) auxiliary per character processed.

*/

#include <map>
#include <string>
#include <vector>
using std::string;

class suffix_automaton {
  struct state {
    int len, link, first_pos;
    std::map<char, int> next;

    state() : len(0), link(-1), first_pos(-1) {}
  };

  std::vector<state> st;
  int last;

 public:
  suffix_automaton() : st(1), last(0) {}

  explicit suffix_automaton(const string &s) : st(1), last(0) {
    for (int i = 0; i < (int)s.size(); i++) {
      extend(s[i]);
    }
  }

  void extend(char c) {
    int cur = st.size();
    st.push_back(state());
    st[cur].len = st[last].len + 1;
    st[cur].first_pos = st[cur].len - 1;

    int p = last;
    while (p != -1 && !st[p].next.count(c)) {
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
    for (int i = 0; i < (int)t.size(); i++) {
      std::map<char, int>::const_iterator it = st[v].next.find(t[i]);
      if (it == st[v].next.end()) {
        return false;
      }
      v = it->second;
    }
    return true;
  }

  long long count_distinct_substrings() const {
    long long res = 0;
    for (int v = 1; v < (int)st.size(); v++) {
      res += st[v].len - st[st[v].link].len;
    }
    return res;
  }

  string longest_common_substring(const string &t) const {
    int v = 0, len = 0, best = 0, best_pos = -1;
    for (int i = 0; i < (int)t.size(); i++) {
      while (v && !st[v].next.count(t[i])) {
        v = st[v].link;
        len = st[v].len;
      }
      std::map<char, int>::const_iterator it = st[v].next.find(t[i]);
      if (it != st[v].next.end()) {
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
  suffix_automaton sa("ababa");
  assert(sa.contains("aba"));
  assert(sa.contains("bab"));
  assert(!sa.contains("abba"));
  assert(sa.count_distinct_substrings() == 9);
  assert(sa.longest_common_substring("zzbabab") == "baba");

  suffix_automaton online;
  online.extend('a');
  online.extend('b');
  online.extend('c');
  assert(online.contains("bc"));
  return 0;
}
