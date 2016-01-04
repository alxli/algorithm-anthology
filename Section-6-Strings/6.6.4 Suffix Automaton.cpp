/*

A suffix automaton is a data structure to efficiently represent the
suffixes of a string. It can be considered a compressed version of
a suffix tree. The data structure supports querying for substrings
within the text from with the automaton is constructed in linear
time. It also supports computation of the longest common substring
in linear time.

Time Complexity: O(n * ALPHABET_SIZE) for construction, and O(n)
for find_all(), as well as longest_common_substring().

Space Complexity: O(n * ALPHABET_SIZE) auxiliary.

*/

#include <algorithm>
#include <queue>
#include <string>
#include <vector>

struct suffix_automaton {

  static const int ALPHABET_SIZE = 26;

  static int map_alphabet(char c) {
    return (int)(c - 'a');
  }

  struct state_t {
    int length, suffix_link;
    int firstpos, next[ALPHABET_SIZE];
    std::vector<int> invlinks;

    state_t() {
      length = 0;
      suffix_link = 0;
      firstpos = -1;
      for (int i = 0; i < ALPHABET_SIZE; i++)
        next[i] = -1;
    }
  };

  std::vector<state_t> states;

  suffix_automaton(const std::string & s) {
    int n = s.size();
    states.resize(std::max(2, 2 * n - 1));
    states[0].suffix_link = -1;
    int last = 0;
    int size = 1;
    for (int i = 0; i < n; i++) {
      int c = map_alphabet(s[i]);
      int curr = size++;
      states[curr].length = i + 1;
      states[curr].firstpos = i;
      int p = last;
      while (p != -1 && states[p].next[c] == -1) {
        states[p].next[c] = curr;
        p = states[p].suffix_link;
      }
      if (p == -1) {
        states[curr].suffix_link = 0;
      } else {
        int q = states[p].next[c];
        if (states[p].length + 1 == states[q].length) {
          states[curr].suffix_link = q;
        } else {
          int clone = size++;
          states[clone].length = states[p].length + 1;
          for (int i = 0; i < ALPHABET_SIZE; i++)
            states[clone].next[i] = states[q].next[i];
          states[clone].suffix_link = states[q].suffix_link;
          while (p != -1 && states[p].next[c] == q) {
            states[p].next[c] = clone;
            p = states[p].suffix_link;
          }
          states[q].suffix_link = clone;
          states[curr].suffix_link = clone;
        }
      }
      last = curr;
    }
    for (int i = 1; i < size; i++)
      states[states[i].suffix_link].invlinks.push_back(i);
    states.resize(size);
  }

  std::vector<int> find_all(const std::string & s) {
    std::vector<int> res;
    int node = 0;
    for (int i = 0; i < (int)s.size(); i++) {
      int next = states[node].next[map_alphabet(s[i])];
      if (next == -1) return res;
      node = next;
    }
    std::queue<int> q;
    q.push(node);
    while (!q.empty()) {
      int curr = q.front();
      q.pop();
      if (states[curr].firstpos != -1)
        res.push_back(states[curr].firstpos - (int)s.size() + 1);
      for (int j = 0; j < (int)states[curr].invlinks.size(); j++)
        q.push(states[curr].invlinks[j]);
    }
    return res;
  }

  std::string longest_common_substring(const std::string & s) {
    int len = 0, bestlen = 0, bestpos = -1;
    for (int i = 0, cur = 0; i < (int)s.size(); i++) {
      int c = map_alphabet(s[i]);
      if (states[cur].next[c] == -1) {
        while (cur != -1 && states[cur].next[c] == -1)
          cur = states[cur].suffix_link;
        if (cur == -1) {
          cur = len = 0;
          continue;
        }
        len = states[cur].length;
      }
      len++;
      cur = states[cur].next[c];
      if (bestlen < len) {
        bestlen = len;
        bestpos = i;
      }
    }
    return s.substr(bestpos - bestlen + 1, bestlen);
  }
};

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
using namespace std;

int main() {
  {
    suffix_automaton sa("bananas");
    vector<int> pos_a, pos_an, pos_ana;
    int ans_a[] = {1, 3, 5};
    int ans_an[] = {1, 3};
    int ans_ana[] = {1, 3};
    pos_a = sa.find_all("a");
    pos_an = sa.find_all("an");
    pos_ana = sa.find_all("ana");
    assert(equal(pos_a.begin(), pos_a.end(), ans_a));
    assert(equal(pos_an.begin(), pos_an.end(), ans_an));
    assert(equal(pos_ana.begin(), pos_ana.end(), ans_ana));
  }
  {
    suffix_automaton sa("bbbabca");
    assert(sa.longest_common_substring("aababcd") == "babc");
  }
  return 0;
}
