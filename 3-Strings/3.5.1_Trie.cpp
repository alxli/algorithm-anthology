/*

Maintain a map of strings to values using a tree data structure. Each node corresponds to a
character, and each inserted string corresponds to a path from the root to a node that is flagged as
a terminal node. Children are stored in hash tables, and `walk()` sorts child labels as needed to
preserve lexicographic traversal.

- `Trie()` constructs an empty map.
- `size()` returns the size of the map.
- `empty()` returns whether the map is empty.
- `insert(s, v)` adds an entry with string key `s` and value `v` to the map, returning `true` if a
  new entry was added or `false` if the string already exists (in which case the map is unchanged
  and the old value associated with the string key is preserved).
- `erase(s)` removes the entry with string key `s` from the map, returning `true` if the removal was
  successful or `false` if the string to be removed was not found.
- `find(s)` returns a pointer to a const value associated with string key `s`, or `nullptr` if the
  key was not found.
- `count_prefix(s)` returns the number of keys currently in the map that have `s` as a prefix (a key
  equal to `s` counts as well). `count_prefix("")` therefore equals `size()`.
- `walk(f)` calls the function `f(s, v)` on each entry of the map, in lexicographically ascending
  order of the string keys.

For a small, fixed alphabet, the `std::unordered_map` children can be replaced by a fixed-size array
of child pointers indexed by character. This removes the per-step hashing and lets `walk()` traverse
in sorted order without sorting each node's labels, at the cost of restricting the alphabet and
spending more memory per node.

Time Complexity:
- O(n) expected per call to `insert(s, v)`, `erase(s)`, `find(s)`, and `count_prefix(s)`, where $n$
  is the length of `s`.
- O(l) per call to `walk()`, where $l$ is the total length of string keys that are currently in the
  map, treating the character alphabet as constant.
- O(1) per call to all other operations.

Space Complexity:
- O(l) for storage of the Trie, where $l$ is the total length of string keys that are currently in
  the map.
- O(n) auxiliary stack space for construction, destruction, `walk()`, where $n$ is the maximum
  length of any string that has been inserted so far.
- O(n) auxiliary stack space for `erase(s)`, where $n$ is the length of `s`.
- O(1) auxiliary for all other operations.

*/

#include <algorithm>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
using std::string;

template<class V>
class Trie {
  struct Node {
    V value;
    bool is_terminal;
    int cnt;  // Optional: maintain count of terminal keys in this subtree for count_prefix queries.
    std::unordered_map<char, Node *> children;

    Node() : is_terminal(false), cnt(0) {}
  } *root;

  static bool erase(Node *n, const string &s, int i) {
    if (i == static_cast<int>(s.size())) {
      if (!n->is_terminal) {
        return false;
      }
      n->is_terminal = false;
      n->cnt--;
      return true;
    }
    auto it = n->children.find(s[i]);
    if (it == n->children.end() || !erase(it->second, s, i + 1)) {
      return false;
    }
    n->cnt--;
    // Prune the child only if it is now a non-terminal leaf; a childless terminal is still a key.
    if (it->second->children.empty() && !it->second->is_terminal) {
      delete it->second;
      n->children.erase(it);
    }
    return true;
  }

  template<class Fn>
  static void walk(Node *n, string &s, Fn f) {
    if (n->is_terminal) {
      f(s, n->value);
    }
    std::vector<std::pair<char, Node *>> children(n->children.begin(), n->children.end());
    std::sort(children.begin(), children.end());
    for (auto &[c, child] : children) {
      s += c;
      walk(child, s, f);
      s.pop_back();
    }
  }

  static void clean_up(Node *n) {
    for (auto &[c, child] : n->children) {
      clean_up(child);
    }
    delete n;
  }

  int num_terminals;

 public:
  Trie() : root(new Node()), num_terminals(0) {}

  ~Trie() { clean_up(root); }
  Trie(const Trie &) = delete;
  Trie &operator=(const Trie &) = delete;
  int size() const { return num_terminals; }
  bool empty() const { return num_terminals == 0; }

  bool insert(const string &s, const V &v) {
    Node *n = root;
    for (char c : s) {
      auto [it, inserted] = n->children.try_emplace(c, nullptr);
      if (inserted) {
        it->second = new Node();
      }
      n = it->second;
    }
    if (n->is_terminal) {
      return false;
    }
    num_terminals++;
    n->is_terminal = true;
    n->value = v;
    // Re-walk from the root to bump the subtree count along the inserted path.
    n = root;
    n->cnt++;
    for (char c : s) {
      n = n->children[c];
      n->cnt++;
    }
    return true;
  }

  bool erase(const string &s) {
    if (erase(root, s, 0)) {
      num_terminals--;
      return true;
    }
    return false;
  }

  const V *find(const string &s) const {
    Node *n = root;
    for (char c : s) {
      if (auto it = n->children.find(c); it != n->children.end()) {
        n = it->second;
      } else {
        return nullptr;
      }
    }
    return n->is_terminal ? &(n->value) : nullptr;
  }

  int count_prefix(const string &s) const {
    Node *n = root;
    for (char c : s) {
      auto it = n->children.find(c);
      if (it == n->children.end()) {
        return 0;
      }
      n = it->second;
    }
    return n->cnt;
  }

  template<class Fn>
  void walk(Fn f) const {
    string s;
    walk(root, s, f);
  }
};

/*** Example Usage and Output:

("", 0)
("a", 1)
("i", 6)
("in", 7)
("inn", 8)
("tea", 3)
("ted", 4)
("ten", 5)
("to", 2)

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  vector<string> s{"", "a", "to", "tea", "ted", "ten", "i", "in", "inn"};
  Trie<int> t;
  assert(t.empty());
  for (int i = 0; i < static_cast<int>(s.size()); i++) {
    assert(t.insert(s[i], i));
  }
  t.walk([](string k, int v) { cout << "(\"" << k << "\", " << v << ")" << endl; });
  assert(!t.empty());
  assert(t.size() == 9);
  assert(!t.insert(s[0], 2));
  assert(t.size() == 9);
  assert(*t.find("") == 0);
  assert(*t.find("ten") == 5);
  assert(t.count_prefix("") == 9);    // Every key has the empty prefix.
  assert(t.count_prefix("te") == 3);  // "tea", "ted", "ten".
  assert(t.count_prefix("in") == 2);  // "in", "inn".
  assert(t.count_prefix("z") == 0);
  assert(t.erase("tea"));
  assert(t.size() == 8);
  assert(t.find("tea") == nullptr);
  assert(t.count_prefix("te") == 2);  // "ted", "ten" remain.
  assert(t.erase(""));
  assert(t.find("") == nullptr);
  assert(t.count_prefix("") == 7);

  // Erasing a key must not delete a shorter key sharing its path.
  Trie<int> u;
  u.insert("bc", 1);
  u.insert("bca", 2);
  assert(u.erase("bca"));
  assert(u.find("bc") != nullptr && *u.find("bc") == 1);
  assert(u.size() == 1 && u.count_prefix("bc") == 1);
  return 0;
}
