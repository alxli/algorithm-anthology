/*

Maintains all distinct palindromic substrings of a string online using an Eertree, also known as a
palindromic tree. Each non-root node represents one distinct palindrome, and suffix links connect
each palindrome to its longest proper palindromic suffix.

The structure has two roots: one of length $-1$, which simplifies boundary cases, and one of length
$0$, representing the empty palindrome. After each appended character, `last` points to the node for
the longest palindromic suffix of the current string.

- `Eertree()` constructs an empty palindromic tree.
- `Eertree(s)` constructs the tree for string `s`.
- `add(c)` appends character `c` and returns `true` if this creates a new distinct palindrome.
- `count_distinct_palindromes()` returns the number of distinct nonempty palindromic substrings.
- `longest_suffix_length()` returns the length of the current longest palindromic suffix.
- `count_occurrences()` propagates occurrence counts through suffix links and returns `occ[v]` for
  each node `v`.

Time Complexity:
- O(n log A) to build the tree for a string of length $n$, where $A$ is the alphabet size, due to
  ordered map transitions.
- O(log A) amortized per call to `add(c)`.
- O(n) per call to `count_occurrences()`.

Space Complexity:
- O(n) transition storage and O(n) nodes.
- O(1) auxiliary per appended character.

*/

#include <map>
#include <string>
#include <vector>
using std::string;

class Eertree {
  struct Node {
    int len, link, occ;
    std::map<char, int> next;

    Node(int len = 0) : len(len), link(0), occ(0) {}
  };

  std::vector<Node> tree;
  string s;
  int last;

  int get_suffix(int v, int pos, char c) const {
    while (pos - 1 - tree[v].len < 0 || s[pos - 1 - tree[v].len] != c) {
      v = tree[v].link;
    }
    return v;
  }

 public:
  Eertree() : tree(), s(), last(1) {
    tree.emplace_back(-1);
    tree.emplace_back(0);
    tree[0].link = 0;
    tree[1].link = 0;
  }

  explicit Eertree(const string &text) : tree(), s(), last(1) {
    tree.emplace_back(-1);
    tree.emplace_back(0);
    tree[0].link = 0;
    tree[1].link = 0;
    for (char c : text) {
      add(c);
    }
  }

  bool add(char c) {
    s += c;
    int pos = s.size() - 1;
    int cur = get_suffix(last, pos, c);
    std::map<char, int>::iterator it = tree[cur].next.find(c);
    if (it != tree[cur].next.end()) {
      last = it->second;
      tree[last].occ++;
      return false;
    }

    last = tree.size();
    tree.emplace_back(tree[cur].len + 2);
    tree[last].occ = 1;
    tree[cur].next[c] = last;

    if (tree[last].len == 1) {
      tree[last].link = 1;
      return true;
    }
    int link_parent = get_suffix(tree[cur].link, pos, c);
    tree[last].link = tree[link_parent].next[c];
    return true;
  }

  int count_distinct_palindromes() const { return static_cast<int>(tree.size()) - 2; }
  int longest_suffix_length() const { return tree[last].len; }

  std::vector<int> count_occurrences() const {
    std::vector<int> occ(tree.size());
    for (int i = 0; i < static_cast<int>(tree.size()); i++) {
      occ[i] = tree[i].occ;
    }
    for (int i = static_cast<int>(tree.size()) - 1; i >= 2; i--) {
      occ[tree[i].link] += occ[i];
    }
    return occ;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  Eertree t("abacaba");
  assert(t.count_distinct_palindromes() == 7);
  assert(t.longest_suffix_length() == 7);

  Eertree online;
  assert(online.add('a'));
  assert(online.add('a'));
  assert(online.add('b'));
  assert(online.add('a'));
  assert(online.count_distinct_palindromes() == 4);
  assert(online.longest_suffix_length() == 3);

  Eertree duplicate;
  assert(duplicate.add('a'));
  assert(duplicate.add('b'));
  assert(duplicate.add('c'));
  assert(!duplicate.add('a'));

  std::vector<int> occ = t.count_occurrences();
  assert(static_cast<int>(occ.size()) == t.count_distinct_palindromes() + 2);
  return 0;
}
