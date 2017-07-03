/*

Given a text and multiple patterns to be searched for within the
text, simultaneously determine the position of all matches.
All of the patterns will be first required for precomputing
the automata, after which any input text may be given without
having to recompute the automata for the pattern.

Time Complexity: O(n) for build_automata(), where n is the sum of
all pattern lengths, and O(1) amortized for next_state(). However,
since it must be called m times for an input text of length m, and
if there are z matches throughout the entire text, then the entire
algorithm will have a running time of O(n + m + z).

Note that in this implementation, a bitset is used to speed up
build_automata() at the cost of making the later text search cost
O(n * m). To truly make the algorithm O(n + m + z), bitset must be
substituted for an unordered_set, which will not encounter any
blank spaces during iteration of the bitset. However, for simply
counting the number of matches, bitsets are clearly advantages.

Space Complexity: O(l * c), where l is the sum of all pattern
lengths and c is the size of the alphabet.

*/

#include <bitset>
#include <cstring>
#include <queue>
#include <string>
#include <vector>

const int MAXP = 1000;  //maximum number of patterns
const int MAXL = 10000; //max possible sum of all pattern lengths
const int MAXC = 26;    //size of the alphabet (e.g. 'a'..'z')

//This function should be customized to return a mapping from
//the input alphabet (e.g. 'a'..'z') to the integers 0..MAXC-1
inline int map_alphabet(char c) {
  return (int)(c - 'a');
}

std::bitset<MAXP> out[MAXL];  //std::unordered_set<int> out[MAXL]
int fail[MAXL], g[MAXL][MAXC + 1];

int build_automata(const std::vector<std::string> & patterns) {
  memset(fail, -1, sizeof fail);
  memset(g, -1, sizeof g);
  for (int i = 0; i < MAXL; i++)
    out[i].reset();  //out[i].clear();
  int states = 1;
  for (int i = 0; i < (int)patterns.size(); i++) {
    const std::string & pattern = patterns[i];
    int curr = 0;
    for (int j = 0; j < (int)pattern.size(); j++) {
      int c = map_alphabet(pattern[j]);
      if (g[curr][c] == -1)
        g[curr][c] = states++;
      curr = g[curr][c];
    }
    out[curr][i] = out[curr][i] | 1;  //out[curr].insert(i);
  }
  for (int c = 0; c < MAXC; c++)
    if (g[0][c] == -1) g[0][c] = 0;
  std::queue<int> q;
  for (int c = 0; c <= MAXC; c++) {
    if (g[0][c] != -1 && g[0][c] != 0) {
      fail[g[0][c]] = 0;
      q.push(g[0][c]);
    }
  }
  while (!q.empty()) {
    int s = q.front(), t;
    q.pop();
    for (int c = 0; c <= MAXC; c++) {
      t = g[s][c];
      if (t != -1) {
        int f = fail[s];
        while (g[f][c] == -1)
          f = fail[f];
        f = g[f][c];
        fail[t] = f;
        out[t] |= out[f];  //out[t].insert(out[f].begin(), out[f].end());
        q.push(t);
      }
    }
  }
  return states;
}

int next_state(int curr, char ch) {
  int next = curr, c = map_alphabet(ch);
  while (g[next][c] == -1)
    next = fail[next];
  return g[next][c];
}

/*** Example Usage (en.wikipedia.org/wiki/Aho–Corasick_algorithm) ***/

#include <iostream>
using namespace std;

int main() {
  vector<string> patterns;
  patterns.push_back("a");
  patterns.push_back("ab");
  patterns.push_back("bab");
  patterns.push_back("bc");
  patterns.push_back("bca");
  patterns.push_back("c");
  patterns.push_back("caa");
  build_automata(patterns);

  string text("abccab");
  int state = 0;
  for (int i = 0; i < (int)text.size(); i++) {
    state = next_state(state, text[i]);
    cout << "Matches ending at position " << i << ":" << endl;
    if (out[state].any())
      for (int j = 0; j < (int)out[state].size(); j++)
        if (out[state][j])
          cout << "'" << patterns[j] << "'" << endl;
  }
  return 0;
}
