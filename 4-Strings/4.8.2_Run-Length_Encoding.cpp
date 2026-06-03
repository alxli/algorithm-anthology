/*

Encodes consecutive equal characters as (`character`, `count`) runs. Run-length
encoding is a simple lossless compression technique that works well when strings
contain long repeated blocks and poorly when repetitions are rare.

The representation below stores runs in a vector of pairs rather than formatting
them as a string. This avoids ambiguity when the original text contains digits or
separator characters.

- `run_length_encode(s)` returns the sequence of runs in string `s`.
- `run_length_decode(runs)` reconstructs the original string from a sequence of
  runs.

Time Complexity:
- O(n) per call, where $n$ is the input or output length.

Space Complexity:
- O(r) for encoded output, where $r$ is the number of runs.
- O(n) for decoded output.

*/

#include <string>
#include <utility>
#include <vector>
using std::string;

std::vector<std::pair<char, int> > run_length_encode(const string &s) {
  std::vector<std::pair<char, int> > res;
  for (int i = 0; i < (int)s.size(); i++) {
    if (res.empty() || res.back().first != s[i]) {
      res.push_back(std::make_pair(s[i], 1));
    } else {
      res.back().second++;
    }
  }
  return res;
}

string run_length_decode(const std::vector<std::pair<char, int> > &runs) {
  string res;
  for (int i = 0; i < (int)runs.size(); i++) {
    res.append(runs[i].second, runs[i].first);
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  string s = "aaabccccdd";
  vector<pair<char, int> > runs = run_length_encode(s);
  assert(runs.size() == 4);
  assert(runs[0] == make_pair('a', 3));
  assert(runs[2] == make_pair('c', 4));
  assert(run_length_decode(runs) == s);
  return 0;
}
