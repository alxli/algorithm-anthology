/*

Given a set of strings (needles) and subsequent queries of texts (haystacks)
to be searched, determine all positions in which needles occur within the given
haystacks in linear time using the Aho-Corasick algorithm.

Note that this implementation uses an ordered map for storage of the graph,
adding an additional log k factor to the time complexities of all operations,
where k is the size of the alphabet (number of distinct characters used across
the needles). It also uses an ordered set for storage of the precomputed output
tables, adding an additional log m factor to the time complexities, where m is
the number of needles. In C++11 and later, both of these containers should be
replaced by their unordered versions for constant time access, thus eliminating
the log factors from the time complexities.

- aho_corasick(needles) constructs the finite-state automaton for a set of
  needle strings that are to be searched for subsequently in haystack queries.
- find_all_in(haystack, report_match) calls the function report_match(s, pos)
  once on each occurrence of each needle that occurs in the haystack, where pos
  is the starting position in the haystack at which string s (a matched needle)
  occurs. The matches will be reported in increasing order of their ending
  positions within the haystack.

Time Complexity:
- O(m*((log m) + l*log k)) per call to the constructor, where m is the number of
  needles, l is the maximum length for any needle, and k is the size of the
  alphabet used by the needles. If unordered containers are used, then the time
  complexity reduces to O(m*l), or linear on the input size.
- O(n*(log k) + z) per call to find_all_in(haystack, report_match), where n is
  the length of the haystack, k is the size of the alphabet used by the needles,
  and z is the number of matches. If unordered containers are used, then the
  time complexity reduces to O(n + z), or linear on the input size.

Space Complexity:
- O(m*l) for storage of the automaton, where where m is the number of needles
  and l is the maximum length for any needle.
- O(1) auxiliary space per call to find_all_in(haystack, report_match).

*/

#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>
using std::string;

class aho_corasick {
  std::vector<string> needles;
  std::vector<int> fail;
  std::vector<std::map<char, int> > graph;
  std::vector<std::set<int> > out;

  int next_state(int curr, char c) {
    int next = curr;
    while (graph[next].find(c) == graph[next].end()) {
      next = fail[next];
    }
    return graph[next][c];
  }

 public:
  aho_corasick(const std::vector<string> &needles) : needles(needles) {
    int total_len = 0;
    for (int i = 0; i < (int)needles.size(); i++) {
      total_len += needles[i].size();
    }
    fail.resize(total_len, -1);
    graph.resize(total_len);
    out.resize(total_len);
    int states = 1;
    std::map<char, int>::iterator it;
    for (int i = 0; i < (int)needles.size(); i++) {
      int curr = 0;
      for (int j = 0; j < (int)needles[i].size(); j++) {
        char c = needles[i][j];
        if ((it = graph[curr].find(c)) != graph[curr].end()) {
          curr = it->second;
        } else {
          curr = graph[curr][c] = states++;
        }
      }
      out[curr].insert(i);
    }
    std::queue<int> q;
    for (it = graph[0].begin(); it != graph[0].end(); ++it) {
      if (it->second != 0) {
        fail[it->second] = 0;
        q.push(it->second);
      }
    }
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (it = graph[u].begin(); it != graph[u].end(); ++it) {
        int v = it->second, f = fail[u];
        while (graph[f].find(it->first) == graph[f].end()) {
          f = fail[f];
        }
        f = graph[f].find(it->first)->second;
        fail[v] = f;
        out[v].insert(out[f].begin(), out[f].end());
        q.push(v);
      }
    }
  }

  template<class ReportFunction>
  void find_all_in(const string &haystack, ReportFunction report_match) {
    int state = 0;
    std::set<int>::iterator it;
    for (int i = 0; i < (int)haystack.size(); i++) {
      state = next_state(state, haystack[i]);
      for (it = out[state].begin(); it != out[state].end(); ++it) {
        report_match(needles[*it], i - needles[*it].size() + 1);
      }
    }
  }
};

/*** Example Usage and Output:

Matched "a" at position 0.
Matched "ab" at position 0.
Matched "bc" at position 1.
Matched "c" at position 2.
Matched "c" at position 3.
Matched "a" at position 4.
Matched "ab" at position 4.
Matched "abccab" at position 0.

***/

#include <iostream>
using namespace std;

void report_match(const string &needle, int pos) {
  cout << "Matched \"" << needle << "\" at position " << pos << "." << endl;
}

int main() {
  vector<string> needles;
  needles.push_back("a");
  needles.push_back("ab");
  needles.push_back("bab");
  needles.push_back("bc");
  needles.push_back("bca");
  needles.push_back("c");
  needles.push_back("caa");
  needles.push_back("abccab");
  aho_corasick automaton(needles);

  automaton.find_all_in("abccab", report_match);
  return 0;
}
