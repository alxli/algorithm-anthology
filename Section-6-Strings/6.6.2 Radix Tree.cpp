/*

A radix tree, radix trie, patricia trie, or compressed trie is a
data structure that is used to store a dynamic set or associative
array where the keys are strings. Each leaf node represents a string
that has been inserted into the trie. Unlike simple tries, radix
tries are space-optimized by merging each node that is an only child
with its parent.

Time Complexity: O(n) for insert(), contains(), and erase(), where
n is the length of the string being inserted, searched, or erased.

Space Complexity: At worst O(l), where l is the sum of all lengths
of strings that have been inserted so far.

*/

#include <string>
#include <vector>

class radix_trie {
  struct node_t {
    std::string label;
    std::vector<node_t*> children;

    node_t(const std::string & s = "") {
      label = s;
    }
  } *root;

  unsigned int lcplen(const std::string & s, const std::string & t) {
    int minsize = (t.size() < s.size()) ? t.size() : s.size();
    if (minsize == 0) return 0;
    unsigned int res = 0;
    for (int i = 0; i < minsize && s[i] == t[i]; i++)
      res++;
    return res;
  }

  void insert(const std::string & s, node_t * n) {
    unsigned int lcp = lcplen(s, n->label);
    if (lcp == 0 || n == root ||
        (lcp > 0 && lcp < s.size() && lcp >= n->label.size())) {
      bool inserted = false;
      std::string newstr = s.substr(lcp, s.size() - lcp);
      for (int i = 0; i < (int)n->children.size(); i++) {
        if (n->children[i]->label[0] == newstr[0]) {
          inserted = true;
          insert(newstr, n->children[i]);
        }
      }
      if (!inserted)
        n->children.push_back(new node_t(newstr));
    } else if (lcp < s.size()) {
      node_t * t = new node_t();
      t->label = n->label.substr(lcp, n->label.size() - lcp);
      t->children.assign(n->children.begin(), n->children.end());
      n->label = s.substr(0, lcp);
      n->children.assign(1, t);
      n->children.push_back(new node_t(s.substr(lcp, s.size() - lcp)));
    }
  }

  void erase(const std::string & s, node_t * n) {
    unsigned int lcp = lcplen(s, n->label);
    if (lcp == 0 || n == root ||
        (lcp > 0 && lcp < s.size() && lcp >= n->label.size())) {
      std::string newstr = s.substr(lcp, s.size() - lcp);
      for (int i = 0; i < (int)n->children.size(); i++) {
        if (n->children[i]->label[0] == newstr[0]) {
          if (newstr == n->children[i]->label &&
              n->children[i]->children.empty()) {
            n->children.erase(n->children.begin() + i);
            return;
          }
          erase(newstr, n->children[i]);
        }
      }
    }
  }

  bool contains(const std::string & s, node_t * n) {
    unsigned int lcp = lcplen(s, n->label);
    if (lcp == 0 || n == root ||
        (lcp > 0 && lcp < s.size() && lcp >= n->label.size())) {
      std::string newstr = s.substr(lcp, s.size() - lcp);
      for (int i = 0; i < (int)n->children.size(); i++)
      if (n->children[i]->label[0] == newstr[0])
        return contains(newstr, n->children[i]);
      return false;
    }
    return lcp == n->label.size();
  }

  static void clean_up(node_t * n) {
    if (n == 0) return;
    for (int i = 0; i < (int)n->children.size(); i++)
      clean_up(n->children[i]);
    delete n;
  }

 public:
  template <class UnaryFunction>
  void walk(node_t * n, UnaryFunction f) {
    if (n == 0) return;
    if (n != root) f(n->label);
    for (int i = 0; i < (int)n->children.size(); i++)
      walk(n->children[i], f);
  }

  radix_trie() { root = new node_t(); }
  ~radix_trie() { clean_up(root); }

  void insert(const std::string & s) { insert(s, root); }
  void erase(const std::string & s) { erase(s, root); }
  bool contains(const std::string & s) { return contains(s, root); }

  template <class UnaryFunction> void walk(UnaryFunction f) {
    walk(root, f);
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

string preorder;

void concat(const string & s) {
  preorder += (s + " ");
}

int main() {
  {
    string s[8] = {"a", "to", "tea", "ted", "ten", "i", "in", "inn"};
    radix_trie t;
    for (int i = 0; i < 8; i++)
      t.insert(s[i]);
    assert(t.contains("ten"));
    t.erase("tea");
    assert(!t.contains("tea"));
  }
  {
    radix_trie t;
    t.insert("test");
    t.insert("toaster");
    t.insert("toasting");
    t.insert("slow");
    t.insert("slowly");
    preorder = "";
    t.walk(concat);
    assert(preorder == "t est oast er ing slow ly ");
  }
  return 0;
}
