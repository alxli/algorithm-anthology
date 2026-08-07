/*

Maintains all distinct palindromic substrings of a string online using an Eertree, also known as a
palindromic tree. Each non-root node represents one distinct palindrome, and suffix links connect
each palindrome to its longest proper palindromic suffix.

The structure has two roots: one of length $-1$, which simplifies boundary cases, and one of length
$0$, representing the empty palindrome. After each appended character, `last` points to the node for
the longest palindromic suffix of the current string.

To append a character, `append()` follows suffix links from `last` until finding the longest suffix
palindrome that can be enclosed by the new character. If the corresponding transition already
exists, that node becomes `last`; otherwise, a node two characters longer is created and linked to
its longest proper palindromic suffix, found by the same search. Exactly one distinct palindrome can
be created at each position, so the number of non-root nodes is the distinct count. Each node first
counts the times it is the longest palindromic suffix; propagating those counts through suffix links
then includes occurrences where it appears as a suffix of a longer palindrome.

- `Eertree()` constructs an empty palindromic tree.
- `Eertree(s)` constructs the tree for string `s`.
- `append(c)` appends character `c` and returns `true` if this creates a new distinct palindrome.
- `count_distinct_palindromes()` returns the number of distinct nonempty palindromic substrings.
- `longest_suffix_length()` returns the length of the current longest palindromic suffix.
- `count_occurrences()` propagates occurrence counts through suffix links and returns `occ[v]` for
  each node `v`.

Time Complexity:
- O(n) expected per call to the constructor for a string of length $n$.
- O(1) expected amortized per call to `append()`.
- O(n) per call to `count_occurrences()`.

Space Complexity:
- O(n) transition storage and O(n) nodes.
- O(1) auxiliary per appended character.

*/

#include <string>
#include <unordered_map>
#include <vector>
using std::string;

class Eertree {
  struct Node {
    int len, link, occ;
    std::unordered_map<char, int> next;

    Node(int len = 0) : len(len), link(0), occ(0) {}
  };

  std::vector<Node> tree;
  string str;
  int last;

  int get_suffix(int v, int pos, char c) const {
    while (pos - 1 - tree[v].len < 0 || str[pos - 1 - tree[v].len] != c) {
      v = tree[v].link;
    }
    return v;
  }

 public:
  Eertree() : last(1) {
    tree.emplace_back(-1);
    tree.emplace_back(0);
    tree[0].link = 0;
    tree[1].link = 0;
  }

  explicit Eertree(const string &s) : Eertree() {
    for (char c : s) {
      append(c);
    }
  }

  bool append(char c) {
    str += c;
    int pos = static_cast<int>(str.size()) - 1;
    int cur = get_suffix(last, pos, c);
    auto it = tree[cur].next.find(c);
    if (it != tree[cur].next.end()) {
      last = it->second;
      tree[last].occ++;
      return false;
    }
    last = static_cast<int>(tree.size());
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
using namespace std;

int main() {
  Eertree t("abacaba");
  // Distinct palindromes: a, b, c, aba, aca, bacab, abacaba.
  assert(t.count_distinct_palindromes() == 7);
  assert(t.longest_suffix_length() == 7);

  Eertree online;
  assert(online.append('a'));
  assert(online.append('a'));
  assert(online.append('b'));
  assert(online.append('a'));
  assert(online.count_distinct_palindromes() == 4);
  assert(online.longest_suffix_length() == 3);

  Eertree duplicate;
  assert(duplicate.append('a'));
  assert(duplicate.append('b'));
  assert(duplicate.append('c'));
  // The second 'a' creates substring "abca", but palindrome "a" already exists.
  assert(!duplicate.append('a'));

  // Nodes 2-8 represent a, b, aba, c, aca, bacab, and abacaba in creation order.
  assert((t.count_occurrences() == vector<int>{0, 7, 4, 2, 2, 1, 1, 1, 1}));
  return 0;
}
