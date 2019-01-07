/*

Maintain a map of strings to values using an ordered tree data structure. Each
node corresponds to a substring of an inserted string, and each inserted string
corresponds to a path from the root to a node that is flagged as a terminal
node. Compared to a regular trie, a radix tree is more space efficient as it
combines chains of nodes with only a single child.

- radix_tree() constructs an empty map.
- size() returns the size of the map.
- empty() returns whether the map is empty.
- insert(s, v) adds an entry with string key s and value v to the map, returning
  true if an new entry was added or false if the string already exists (in which
  case the map is unchanged and the old value associated with the string key is
  preserved).
- erase(s) removes the entry with string key s from the map, returning true if
  the removal was successful or false if the string to be removed was not found.
- find(s) returns a pointer to a const value associated with string key s, or
  NULL if the key was not found.
- walk(f) calls the function f(s, v) on each entry of the map, in
  lexicographically ascending order of the string keys.

Time Complexity:
- O(n) per call to insert(s, v), erase(s), and find(s), where n is the length of
  s. Note that there is a hidden factor of log(alphabet_size) which can be
  considered constant, since char can only take on 2^CHAR_BIT values. The
  implementation may be optimized by storing the children of nodes in an
  std::unordered_map in C++11 and later, or an array if a smaller alphabet size
  is guaranteed.
- O(l) per call to walk(), where l is the total length of string keys that are
  currently in the map.
- O(1) per call to all other operations.

Space Complexity:
- O(l) for storage of the radix tree, where l is the total length of string keys
  that are currently in the map.
- O(n) auxiliary stack space for construction, destruction, walk(), where n is
  the maximum length of any string that has been inserted so far.
- O(n) auxiliary stack space for erase(s), where n is the length of s.
- O(1) auxiliary for all other operations.

*/

#include <cstddef>
#include <map>
#include <string>
#include <utility>
using std::string;

template<class V>
class radix_tree {
  struct node_t {
    V value;
    bool is_terminal;
    std::map<string, node_t*> children;

    node_t(const V &value = V(), bool is_terminal = false)
        : value(value), is_terminal(is_terminal) {}
  } *root;

  typedef typename std::map<string, node_t*>::iterator cit;

  static int lcp_len(const string &s1, const string &s2, int s2start) {
    int i = 0;
    for (int j = s2start; i < (int)s1.size() && j < (int)s2.size(); i++, j++) {
      if (s1[i] != s2[j]) {
        break;
      }
    }
    return i;
  }

  static bool insert(node_t *n, const string &s, int i, const V &v) {
    if (i == (int)s.size()) {
      if (n->is_terminal) {
        return false;
      }
      n->is_terminal = true;
      return true;
    }
    for (cit it = n->children.begin(); it != n->children.end(); ++it) {
      int len = lcp_len(it->first, s, i);
      if (len == 0) {
        continue;
      }
      if (len == (int)it->first.size()) {
        return insert(it->second, s, i + len, v);
      }
      string left = it->first.substr(0, len);
      string right = it->first.substr(len);
      node_t *tmp = new node_t();
      tmp->children[right] = it->second;
      n->children.erase(it);
      n->children[left] = tmp;
      if (len == (int)s.size() - i) {
        tmp->value = v;
        tmp->is_terminal = true;
        return true;
      }
      return insert(tmp, s, i + len, v);
    }
    n->children[s.substr(i)] = new node_t(v, true);
    return true;
  }

  static bool erase(node_t *n, const string &s, int i) {
    if (i == (int)s.size()) {
      if (!n->is_terminal) {
        return false;
      }
      n->is_terminal = false;
      return true;
    }
    for (cit it = n->children.begin(); it != n->children.end(); ++it) {
      int len = lcp_len(it->first, s, i);
      if (len == 0) {
        continue;
      }
      node_t *child = it->second;
      if (!erase(child, s, i + len)) {
        return false;
      }
      if (child->children.empty()) {
        delete child;
        n->children.erase(it);
      } else if (child->children.size() == 1) {
        node_t *grandchild = child->children.begin()->second;
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

  template<class KVFunction>
  static void walk(node_t *n, string &s, KVFunction f) {
    if (n->is_terminal) {
      f(s, n->value);
    }
    for (cit it = n->children.begin(); it != n->children.end(); ++it) {
      s += it->first;
      walk(it->second, s, f);
      s.pop_back();
    }
  }

  static void clean_up(node_t *n) {
    for (cit it = n->children.begin(); it != n->children.end(); ++it) {
      clean_up(it->second);
    }
    delete n;
  }

  int num_terminals;

 public:
  radix_tree() : root(new node_t()), num_terminals(0) {}

  ~radix_tree() {
    clean_up(root);
  }

  int size() const {
    return num_terminals;
  }

  bool empty() const {
    return num_terminals == 0;
  }

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

  const V* find(const string &s) const {
    node_t *n = root;
    int i = 0;
    while (i < (int)s.size()) {
      bool found = false;;
      for (cit it = n->children.begin(); it != n->children.end(); ++it) {
        if (it->first[0] == s[i]) {
          i += lcp_len(it->first, s, i);
          n = it->second;
          found = true;
          break;
        }
      }
      if (!found) {
        return NULL;
      }
    }
    return n->is_terminal ? &(n->value) : NULL;
  }

  template<class KVFunction>
  void walk(KVFunction f) const {
    string s = "";
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
  string s[9] = {"", "a", "to", "tea", "ted", "ten", "i", "in", "inn"};
  radix_tree<int> t;
  assert(t.empty());
  for (int i = 0; i < 9; i++) {
    assert(t.insert(s[i], i));
  }
  t.walk(print_entry);
  assert(!t.empty());
  assert(t.size() == 9);
  assert(!t.insert(s[0], 2));
  assert(t.size() == 9);
  assert(t.find("") && *t.find("") == 0);
  assert(*t.find("ten") == 5);
  assert(t.erase("tea"));
  assert(t.size() == 8);
  assert(t.find("tea") == NULL);
  assert(t.erase(""));
  assert(t.find("") == NULL);
  return 0;
}
