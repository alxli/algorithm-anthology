/*

A suffix tree is the compressed trie of every suffix of a string: a rooted tree in which each edge
is labeled with a substring, no two edges leaving a node start with the same character, and the
concatenation of labels from the root to each leaf spells one suffix. Paths from the root therefore
spell exactly the distinct substrings of the string, which makes the tree a powerful index for
substring queries, repeated-substring problems, and many other tasks. It is the explicit-pointer
counterpart of the suffix automaton; the two store the same information and either can be converted
into the other.

This implementation is Ukkonen's algorithm, which builds the tree online in linear time by feeding
one character at a time. It maintains an "active point" (a node, an edge, and a length along that
edge) describing where the next suffix must be inserted, together with suffix links that let it hop
between successive insertion points in amortized constant time. Edges to leaves are stored with an
open-ended length that grows automatically as the string lengthens, so each phase touches only the
suffixes that genuinely change. A unique terminal sentinel smaller than every real character is
appended internally so that no suffix is a prefix of another and every suffix ends at its own leaf.
The input must therefore not contain the character `'\1'`.

- `SuffixTree(s)` builds the suffix tree of string `s`.
- `contains(t)` returns whether string `t` occurs as a substring of `s`.
- `count_distinct_substrings()` returns the number of distinct nonempty substrings of `s`.
- `longest_repeated_substring()` returns one longest substring of `s` that occurs at least twice, or
  the empty string if every character is distinct.

Time Complexity:
- O(n) to construct the tree for a string of length $n$ (using a per-node ordered map, O(n log s)
  where $s$ is the alphabet size).
- O(m log s) per call to `contains(t)`, where $m$ is the length of `t`.
- O(n) per call to `count_distinct_substrings()` and `longest_repeated_substring()`.

Space Complexity:
- O(n) nodes and edges.
- O(n) auxiliary stack space for the traversals.

*/

#include <map>
#include <string>
#include <vector>

class SuffixTree {
  static const int OPEN = -1;  // Marks a leaf edge whose right end follows the growing string.

  struct Node {
    int start, end;  // Edge into this node labels s[start..end]; end == OPEN means leaf_end.
    int link;
    std::map<char, int> next;
    Node(int start, int end) : start(start), end(end), link(0) {}
  };

  std::string s;
  std::vector<Node> tree;
  int leaf_end;
  // Active point and bookkeeping for Ukkonen's algorithm.
  int active_node, active_edge, active_length, remaining, last_new;

  int edge_end(int v) const { return tree[v].end == OPEN ? leaf_end : tree[v].end; }
  int edge_len(int v) const { return edge_end(v) - tree[v].start + 1; }

  int make_node(int start, int end) {
    tree.emplace_back(start, end);
    return static_cast<int>(tree.size()) - 1;
  }

  // If the active length runs past the edge into v, descend and report that we moved.
  bool walk_down(int v) {
    if (active_length >= edge_len(v)) {
      active_edge += edge_len(v);
      active_length -= edge_len(v);
      active_node = v;
      return true;
    }
    return false;
  }

  void extend(int pos) {
    leaf_end = pos;
    remaining++;
    last_new = -1;
    while (remaining > 0) {
      if (active_length == 0) {
        active_edge = pos;
      }
      char c = s[active_edge];
      auto it = tree[active_node].next.find(c);
      if (it == tree[active_node].next.end()) {
        // Rule 2: no edge starts with c, so grow a new leaf from the active node.
        tree[active_node].next[c] = make_node(pos, OPEN);
        if (last_new != -1) {
          tree[last_new].link = active_node;
          last_new = -1;
        }
      } else {
        int v = it->second;
        if (walk_down(v)) {
          continue;  // Re-evaluate the same suffix from the deeper node.
        }
        if (s[tree[v].start + active_length] == s[pos]) {
          // Rule 3: the character is already present; stop and lengthen the active point.
          if (last_new != -1) {
            tree[last_new].link = active_node;
          }
          active_length++;
          break;
        }
        // Rule 2 with a split: break edge v in two and attach a new leaf at the split point.
        int split = make_node(tree[v].start, tree[v].start + active_length - 1);
        tree[active_node].next[c] = split;
        tree[split].next[s[pos]] = make_node(pos, OPEN);
        tree[v].start += active_length;
        tree[split].next[s[tree[v].start]] = v;
        if (last_new != -1) {
          tree[last_new].link = split;
        }
        last_new = split;
      }
      remaining--;
      if (active_node == 0 && active_length > 0) {
        active_length--;
        active_edge = pos - remaining + 1;
      } else if (active_node != 0) {
        active_node = tree[active_node].link;
      }
    }
  }

  // String depth and right end (one past) of the deepest internal node, for repeated substrings.
  void deepest_internal(int v, int depth, int &best_depth, int &best_end) const {
    bool internal = false;
    for (const auto &kv : tree[v].next) {
      internal = true;
      int w = kv.second;
      deepest_internal(w, depth + edge_len(w), best_depth, best_end);
    }
    if (internal && v != 0 && depth > best_depth) {
      best_depth = depth;
      best_end = edge_end(v) + 1;
    }
  }

 public:
  explicit SuffixTree(const std::string &str) : s(str) {
    s.push_back('\1');  // Unique terminal sentinel, smaller than every real character.
    make_node(-1, -1);  // Root, at index 0; its incoming edge is never read.
    active_node = 0;
    active_edge = 0;
    active_length = 0;
    remaining = 0;
    leaf_end = -1;
    last_new = -1;
    for (int i = 0; i < static_cast<int>(s.size()); i++) {
      extend(i);
    }
  }

  bool contains(const std::string &t) const {
    int v = 0, i = 0, m = static_cast<int>(t.size());
    while (i < m) {
      auto it = tree[v].next.find(t[i]);
      if (it == tree[v].next.end()) {
        return false;
      }
      v = it->second;
      int len = edge_len(v);
      for (int j = 0; j < len && i < m; j++, i++) {
        if (s[tree[v].start + j] != t[i]) {
          return false;
        }
      }
    }
    return true;
  }

  int count_distinct_substrings() const {
    int total = 0;
    for (int v = 1; v < static_cast<int>(tree.size()); v++) {
      total += edge_len(v);
    }
    // Every leaf edge ends in the sentinel; the s.size() suffixes of the sentineled string are the
    // only substrings that contain it, so removing them leaves the count for the original string.
    return total - static_cast<int>(s.size());
  }

  std::string longest_repeated_substring() const {
    int best_depth = 0, best_end = 0;
    deepest_internal(0, 0, best_depth, best_end);
    return s.substr(best_end - best_depth, best_depth);
  }
};

/*** Example Usage ***/

#include <cassert>
#include <set>
using namespace std;

int main() {
  SuffixTree st("banana");
  assert(st.contains("ana"));
  assert(st.contains("banana"));
  assert(st.contains(""));
  assert(!st.contains("anana_"));
  assert(!st.contains("nb"));
  assert(st.longest_repeated_substring() == "ana");

  // "banana" has 15 distinct substrings:
  // a, an, ana, anan, anana, b, ba, ban, bana, banan, banana, n, na, nan, nana.
  assert(st.count_distinct_substrings() == 15);

  // Cross-check both queries against brute force on a small string.
  string text = "mississippi";
  SuffixTree t(text);
  set<string> subs;
  int n = static_cast<int>(text.size());
  for (int i = 0; i < n; i++) {
    for (int len = 1; i + len <= n; len++) {
      subs.insert(text.substr(i, len));
    }
  }
  assert(t.count_distinct_substrings() == static_cast<int>(subs.size()));
  for (const string &sub : subs) {
    assert(t.contains(sub));
  }
  assert(!t.contains("issa"));
  assert(t.longest_repeated_substring() == "issi");  // "issi" appears at indices 1 and 4.
  return 0;
}
