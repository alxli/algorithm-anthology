/*

6.1 - String Toolbox

Useful or trivial string operations. These functions are not particularly
algorithmics, and are simply naive implementations using C++ features.

*/

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

//integer to string conversion and vice versa using C++ features

//itoa() is a nonstandard function in the C standard library (see below)
//in addition, a standard std::to_string is introduced in C++0x
template<class Int>
std::string to_string(const Int & i) {
  std::ostringstream oss;
  oss << i;
  return oss.str();
}

//like atoi except during exceptional cases like overflows
int to_int(const std::string & s) {
  std::istringstream iss(s);
  int res;
  if (!(iss >> res)) /* complain */;
  return res;
}

/*

Tokenizes the string s based on single character delimiters.

Version 1: Simpler. Only one delimiter character allowed, and this will
not skip empty tokens.
  e.g. split("a::b", ":") yields {"a", "b"}, not {"a", "", "b"}.

Version 2: All of the characters in the delim parameter that also exists
in s will be removed from s, and the token(s) of s that are left over will
be added sequentially to a vector and returned. Empty tokens are skipped.
  e.g. split("a::b", ":") yields {"a", "b"}, not {"a", "", "b"}.

Time Complexity: O(s.length() * delim.length())

*/

std::vector<std::string> split(const std::string & s, char delim) {
  std::vector<std::string> res;
  std::stringstream ss(s);
  std::string curr;
  while (std::getline(ss, curr, delim))
    res.push_back(curr);
  return res;
}

std::vector<std::string> split(const std::string & s,
                               const std::string & delim = " \n\t\v\f\r") {
  std::vector<std::string> res;
  std::string curr;
  for (int i = 0; i < (int)s.size(); i++) {
    if (delim.find(s[i]) == std::string::npos) {
      curr += s[i];
    } else if (!curr.empty()) {
      res.push_back(curr);
      curr = "";
    }
  }
  if (!curr.empty()) res.push_back(curr);
  return res;
}

/*

Like the explode() function in PHP, the string s is tokenized based
on delim, which is considered as a whole boundary string, not just a
sequence of possible boundary characters like the split() function above.
This will not skip empty tokens.
  e.g. explode("a::b", ":") yields {"a", "b"}, not {"a", "", "b"}.

Time Complexity: O(s.length() * delim.length())

*/

std::vector<std::string> explode(const std::string & s,
                                 const std::string & delim) {
  std::vector<std::string> res;
  unsigned int last = 0, next = 0;
  while ((next = s.find(delim, last)) != std::string::npos) {
    res.push_back(s.substr(last, next - last));
    last = next + delim.size();
  }
  res.push_back(s.substr(last));
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

void print(const vector<string> & v) {
  cout << "[";
  for (int i = 0; i < (int)v.size(); i++)
    cout << (i ? "\", \"" : "\"") << v[i];
  cout << "\"]\n";
}

int main() {
  assert(to_string(123) + "4" == "1234");
  assert(to_int("1234") == 1234);

  vector<string> tokens;

  tokens = split("a\nb\ncde\nf", '\n');
  cout << "split v1: ";
  print(tokens); //["a", "b", "cde", "f"]

  tokens = split("a::b,cde:,f", ":,");
  cout << "split v2: ";
  print(tokens); //["a", "b", "cde", "f"]

  tokens = explode("a..b.cde....f", "..");
  cout << "explode: ";
  print(tokens); //["a", ".b.cde", "", ".f"]

  return 0;
}
