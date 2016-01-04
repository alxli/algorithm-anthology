/*

A suffix tree of a string S is a compressed trie of all the suffixes
of S. While it can be constructed in O(n^2) time on the length of S
by simply inserting the suffixes into a radix tree, Ukkonen (1995)
provided an algorithm to construct one in O(n * ALPHABET_SIZE).

Suffix trees can be used for string searching, pattern matching, and
solving the longest common substring problem. The implementation
below is optimized for solving the latter.

Time Complexity: O(n) for construction of suffix_tree() and
per call to longest_common_substring(), respectively.

Space Complexity: O(n) auxiliary.

*/

#include <cstdio>
#include <string>

struct suffix_tree {

  static const int ALPHABET_SIZE = 38;

  static int map_alphabet(char c) {
    static const std::string ALPHABET(
      "abcdefghijklmnopqrstuvwxyz0123456789\01\02"
    );
    return ALPHABET.find(c);
  }

  struct node_t {
    int begin, end, depth;
    node_t *parent, *suffix_link;
    node_t *children[ALPHABET_SIZE];

    node_t(int begin, int end, int depth, node_t * parent) {
      this->begin = begin;
      this->end = end;
      this->depth = depth;
      this->parent = parent;
      for (int i = 0; i < ALPHABET_SIZE; i++)
        children[i] = 0;
    }
  } *root;

  suffix_tree(const std::string & s) {
    int n = s.size();
    int * c = new int[n];
    for (int i = 0; i < n; i++) c[i] = map_alphabet(s[i]);
    root = new node_t(0, 0, 0, 0);
    node_t *node = root;
    for (int i = 0, tail = 0; i < n; i++, tail++) {
      node_t *last = 0;
      while (tail >= 0) {
        node_t *ch = node->children[c[i - tail]];
        while (ch != 0 && tail >= ch->end - ch->begin) {
          tail -= ch->end - ch->begin;
          node = ch;
          ch = ch->children[c[i - tail]];
        }
        if (ch == 0) {
          node->children[c[i]] = new node_t(i, n,
                                  node->depth + node->end - node->begin, node);
          if (last != 0) last->suffix_link = node;
          last = 0;
        } else {
          int aftertail = c[ch->begin + tail];
          if (aftertail == c[i]) {
            if (last != 0) last->suffix_link = node;
            break;
          } else {
            node_t *split = new node_t(ch->begin, ch->begin + tail,
                             node->depth + node->end - node->begin, node);
            split->children[c[i]] = new node_t(i, n, ch->depth + tail, split);
            split->children[aftertail] = ch;
            ch->begin += tail;
            ch->depth += tail;
            ch->parent = split;
            node->children[c[i - tail]] = split;
            if (last != 0)
              last->suffix_link = split;
            last = split;
          }
        }
        if (node == root) {
          tail--;
        } else {
          node = node->suffix_link;
        }
      }
    }
  }
};

int lcs_begin, lcs_len;

int lcs_rec(suffix_tree::node_t * n, int i1, int i2) {
  if (n->begin <= i1 && i1 < n->end) return 1;
  if (n->begin <= i2 && i2 < n->end) return 2;
  int mask = 0;
  for (int i = 0; i < suffix_tree::ALPHABET_SIZE; i++) {
    if (n->children[i] != 0)
      mask |= lcs_rec(n->children[i], i1, i2);
  }
  if (mask == 3) {
    int curr_len = n->depth + n->end - n->begin;
    if (lcs_len < curr_len) {
      lcs_len = curr_len;
      lcs_begin = n->begin;
    }
  }
  return mask;
}

std::string longest_common_substring
(const std::string & s1, const std::string & s2) {
  std::string s(s1 + '\01' + s2 + '\02');
  suffix_tree tree(s);
  lcs_begin = lcs_len = 0;
  lcs_rec(tree.root, s1.size(), s1.size() + s2.size() + 1);
  return s.substr(lcs_begin - 1, lcs_len);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(longest_common_substring("bbbabca", "aababcd") == "babc");
  return 0;
}
