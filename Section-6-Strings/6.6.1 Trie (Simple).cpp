/*

A trie, digital tree, or prefix tree, is an ordered tree data
structure that is used to store a dynamic set or associative array
where the keys are strings. Each leaf node represents a string that
has been inserted into the trie. This makes tries easier to implement
than balanced binary search trees, and also potentially faster.

Time Complexity: O(n) for insert(), contains(), and erase(), where
n is the length of the string being inserted, searched, or erased.

Space Complexity: At worst O(l * ALPHABET_SIZE), where l is the
sum of all lengths of strings that have been inserted so far.

*/

#include <string>

class trie {
  static const int ALPHABET_SIZE = 26;

  static int map_alphabet(char c) {
    return (int)(c - 'a');
  }

  struct node_t {
    bool leaf;

    node_t * children[ALPHABET_SIZE];

    node_t(): leaf(false) {
      for (int i = 0; i < ALPHABET_SIZE; i++)
        children[i] = 0;
    }

    bool is_free() {
      for (int i = 0; i < ALPHABET_SIZE; i++)
        if (this->children[i] != 0) return true;
      return false;
    }
  } *root;

  bool erase(const std::string & s, node_t * n, int depth) {
    if (n == 0) return false;
    if (depth == (int)s.size()) {
      if (n->leaf) {
        n->leaf = false;
        return n->is_free();
      }
    } else {
      int idx = map_alphabet(s[depth]);
      if (erase(s, n->children[idx], depth + 1)) {
        delete n->children[idx];
        return !n->leaf && n->is_free();
      }
    }
    return false;
  }

  static void clean_up(node_t * n) {
    if (n == 0 || n->leaf) return;
    for (int i = 0; i < ALPHABET_SIZE; i++)
      clean_up(n->children[i]);
    delete n;
  }

 public:
  trie() { root = new node_t(); }
  ~trie() { clean_up(root); }

  void insert(const std::string & s) {
    node_t * n = root;
    for (int i = 0; i < (int)s.size(); i++) {
      int c = map_alphabet(s[i]);
      if (n->children[c] == 0)
        n->children[c] = new node_t();
      n = n->children[c];
    }
    n->leaf = true;
  }

  bool contains(const std::string & s) {
    node_t *n = root;
    for (int i = 0; i < (int)s.size(); i++) {
      int c = map_alphabet(s[i]);
      if (n->children[c] == 0)
        return false;
      n = n->children[c];
    }
    return n != 0 && n->leaf;
  }

  bool erase(const std::string & s) {
    return erase(s, root, 0);
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  string s[8] = {"a", "to", "tea", "ted", "ten", "i", "in", "inn"};
  trie t;
  for (int i = 0; i < 8; i++)
    t.insert(s[i]);
  assert(t.contains("ten"));
  t.erase("tea");
  assert(!t.contains("tea"));
  return 0;
}
