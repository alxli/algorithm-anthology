/*

Maintain a map of strings to values using a compressed tree data structure. Each node corresponds to
a substring of an inserted string, and each inserted string corresponds to a path from the root to a
node that is flagged as a terminal node. Contrary to a regular trie, a radix tree is more space
efficient as it combines chains of nodes with only a single child. Children are stored in hash
tables, and `walk()` sorts child labels as needed to preserve lexicographic traversal.

- `RadixTree<V>()` constructs an empty map.
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
  equal to `s` counts as well), including the case where `s` ends partway along a compressed edge.
  `count_prefix("")` therefore equals `size()`.
- `walk(f)` calls the function `f(s, v)` on each entry of the map, in lexicographically ascending
  order of the string keys.

Time Complexity:
- O(n) expected per call to `insert(s, v)`, `erase(s)`, `find(s)`, and `count_prefix(s)`, where $n$
  is the length of `s`, assuming the number of outgoing compressed edges checked at each node is
  small.
- O(l log l) per call to `walk()`, where $l$ is the total length of string keys that are currently
  in the map.
- O(1) per call to all other operations.

Space Complexity:
- O(l) for storage of the radix tree, where $l$ is the total length of string keys that are
  currently in the map.
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

template<typename V>
class RadixTree {
  struct Node {
    V value;
    bool is_terminal;
    int cnt;  // Optional: maintain count of terminal keys in this subtree for count_prefix queries.
    std::unordered_map<string, Node *> children;

    Node(const V &value = V(), bool is_terminal = false)
        : value(value), is_terminal(is_terminal), cnt(0) {}
  } *root;

  static int lcp_len(const string &s1, const string &s2, int s2start) {
    int i = 0;
    for (int j = s2start; i < static_cast<int>(s1.size()) && j < static_cast<int>(s2.size());
         i++, j++) {
      if (s1[i] != s2[j]) {
        break;
      }
    }
    return i;
  }

  static bool insert(Node *n, const string &s, int i, const V &v) {
    if (i == static_cast<int>(s.size())) {
      if (n->is_terminal) {
        return false;
      }
      n->value = v;
      n->is_terminal = true;
      n->cnt++;
      return true;
    }
    for (auto it = n->children.begin(); it != n->children.end(); ++it) {
      int len = lcp_len(it->first, s, i);
      if (len == 0) {
        continue;
      }
      if (len == static_cast<int>(it->first.size())) {
        if (!insert(it->second, s, i + len, v)) {
          return false;
        }
        n->cnt++;
        return true;
      }
      string left = it->first.substr(0, len);
      string right = it->first.substr(len);
      Node *tmp = new Node();
      tmp->cnt = it->second->cnt;  // The new split node heads the same subtree as the old child.
      tmp->children[right] = it->second;
      n->children.erase(it);
      n->children[left] = tmp;
      if (len == static_cast<int>(s.size()) - i) {
        tmp->value = v;
        tmp->is_terminal = true;
        tmp->cnt++;
      } else {
        insert(tmp, s, i + len, v);
      }
      n->cnt++;
      return true;
    }
    Node *leaf = new Node(v, true);
    leaf->cnt = 1;
    n->children[s.substr(i)] = leaf;
    n->cnt++;
    return true;
  }

  static bool erase(Node *n, const string &s, int i) {
    if (i == static_cast<int>(s.size())) {
      if (!n->is_terminal) {
        return false;
      }
      n->is_terminal = false;
      n->cnt--;
      return true;
    }
    for (auto it = n->children.begin(); it != n->children.end(); ++it) {
      int len = lcp_len(it->first, s, i);
      if (len == 0) {
        continue;
      }
      // The entire edge label must be consumed; a partial match means the key is not present, so
      // erasing must fail rather than descend (otherwise erase("te") would remove "tea").
      if (len < static_cast<int>(it->first.size())) {
        return false;
      }
      Node *child = it->second;
      if (!erase(child, s, i + len)) {
        return false;
      }
      n->cnt--;
      // The merge below leaves counts intact: a non-terminal node with one child already has the
      // same subtree count as that child, so reusing the node keeps its `cnt` correct.
      if (child->children.empty() && !child->is_terminal) {
        delete child;
        n->children.erase(it);
      } else if (child->children.size() == 1) {
        Node *grandchild = child->children.begin()->second;
        if (!child->is_terminal) {
          string merged_key(it->first + child->children.begin()->first);
          child->value = grandchild->value;
          child->is_terminal = grandchild->is_terminal;
          child->children = grandchild->children;
          delete grandchild;
          n->children.erase(it);
          n->children[merged_key] = child;
        }
      }
      return true;
    }
    return false;
  }

  template<typename Fn>
  static void walk(Node *n, string &s, Fn f) {
    if (n->is_terminal) {
      f(s, n->value);
    }
    std::vector<std::pair<string, Node *>> children(n->children.begin(), n->children.end());
    std::sort(children.begin(), children.end());
    for (auto &[key, child] : children) {
      s += key;
      walk(child, s, f);
      s.erase(s.size() - key.size());
    }
  }

  static void clean_up(Node *n) {
    for (auto &[key, child] : n->children) {
      clean_up(child);
    }
    delete n;
  }

  int num_terminals;

 public:
  RadixTree() : root(new Node()), num_terminals(0) {}

  ~RadixTree() { clean_up(root); }
  RadixTree(const RadixTree &) = delete;
  RadixTree &operator=(const RadixTree &) = delete;
  int size() const { return num_terminals; }
  bool empty() const { return num_terminals == 0; }

  bool insert(const string &s, const V &v) {
    if (insert(root, s, 0, v)) {
      num_terminals++;
      return true;
    }
    return false;
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
    int i = 0;
    while (i < static_cast<int>(s.size())) {
      bool found = false;
      for (auto &[key, child] : n->children) {
        if (key[0] == s[i]) {
          // The entire edge label must be consumed; a partial match (s ends mid-edge or diverges
          // from it) means the key is not present.
          if (lcp_len(key, s, i) < static_cast<int>(key.size())) {
            return nullptr;
          }
          i += static_cast<int>(key.size());
          n = child;
          found = true;
          break;
        }
      }
      if (!found) {
        return nullptr;
      }
    }
    return n->is_terminal ? &(n->value) : nullptr;
  }

  int count_prefix(const string &s) const {
    Node *n = root;
    int i = 0;
    while (i < static_cast<int>(s.size())) {
      bool found = false;
      for (auto &[key, child] : n->children) {
        if (key[0] == s[i]) {
          int len = lcp_len(key, s, i);
          if (i + len == static_cast<int>(s.size())) {
            // `s` ends within (or exactly at the end of) this edge: every key below qualifies.
            return child->cnt;
          }
          if (len < static_cast<int>(key.size())) {
            return 0;  // `s` diverges partway along the edge.
          }
          i += len;
          n = child;
          found = true;
          break;
        }
      }
      if (!found) {
        return 0;
      }
    }
    return n->cnt;
  }

  template<typename Fn>
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

void print_entry(const string &k, int v) {
  cout << "(\"" << k << "\", " << v << ")" << endl;
}

int main() {
  vector<string> s{"", "a", "to", "tea", "ted", "ten", "i", "in", "inn"};
  RadixTree<int> t;
  assert(t.empty());
  for (int i = 0; i < static_cast<int>(s.size()); i++) {
    assert(t.insert(s[i], i));
  }
  t.walk([](string k, int v) { cout << "(\"" << k << "\", " << v << ")" << endl; });
  assert(!t.empty());
  assert(t.size() == 9);
  assert(!t.insert(s[0], 2));
  assert(t.size() == 9);
  assert(t.find("") && *t.find("") == 0);
  assert(*t.find("ten") == 5);
  assert(t.count_prefix("") == 9);    // Every key has the empty prefix.
  assert(t.count_prefix("te") == 3);  // "tea", "ted", "ten" (prefix ends mid-edge).
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
  RadixTree<int> u;
  u.insert("bc", 1);
  u.insert("bca", 2);
  assert(u.erase("bca"));
  assert(u.find("bc") != nullptr && *u.find("bc") == 1);
  assert(u.size() == 1 && u.count_prefix("bc") == 1);
  return 0;
}
