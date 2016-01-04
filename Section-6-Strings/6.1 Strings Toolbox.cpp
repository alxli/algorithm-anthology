/*

Useful or trivial string operations. These functions are not particularly
algorithmic. They are typically naive implementations using C++ features.
They depend on many features of the C++ <string> library, which tend to
have an unspecified complexity. They may not be optimally efficient.

*/

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

//integer to string conversion and vice versa using C++ features

//note that a similar std::to_string is introduced in C++0x
template<class Int>
std::string to_string(const Int & i) {
  std::ostringstream oss;
  oss << i;
  return oss.str();
}

//like atoi, except during special cases like overflows
int to_int(const std::string & s) {
  std::istringstream iss(s);
  int res;
  if (!(iss >> res)) /* complain */;
  return res;
}

/*

itoa implementation (fast)
documentation: http://www.cplusplus.com/reference/cstdlib/itoa/
taken from: http://www.jb.man.ac.uk/~slowe/cpp/itoa.html

*/

char* itoa(int value, char * str, int base = 10) {
  if (base < 2 || base > 36) {
    *str = '\0';
    return str;
  }
  char *ptr = str, *ptr1 = str, tmp_c;
  int tmp_v;
  do {
    tmp_v = value;
    value /= base;
    *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789"
             "abcdefghijklmnopqrstuvwxyz"[35 + (tmp_v - value * base)];
  } while (value);
  if (tmp_v < 0) *ptr++ = '-';
  for (*ptr-- = '\0'; ptr1 < ptr; *ptr1++ = tmp_c) {
    tmp_c = *ptr;
    *ptr-- = *ptr1;
  }
  return str;
}

/*

Trimming functions (in place). Given a string and optionally a series
of characters to be considered for trimming, trims the string's ends
(left, right, or both) and returns the string. Note that the ORIGINAL
string is trimmed as it's passed by reference, despite the original
reference being returned for convenience.

*/

std::string& ltrim(std::string & s, const std::string & delim = " \n\t\v\f\r") {
  unsigned int pos = s.find_first_not_of(delim);
  if (pos != std::string::npos) s.erase(0, pos);
  return s;
}

std::string& rtrim(std::string & s, const std::string & delim = " \n\t\v\f\r") {
  unsigned int pos = s.find_last_not_of(delim);
  if (pos != std::string::npos) s.erase(pos);
  return s;
}

std::string& trim(std::string & s, const std::string & delim = " \n\t\v\f\r") {
  return ltrim(rtrim(s));
}

/*

Returns a copy of the string s with all occurrences of the given
string search replaced with the given string replace.

Time Complexity: Unspecified, but proportional to the number of times
the search string occurs and the complexity of std::string::replace,
which is unspecified.

*/

std::string replace(std::string s,
                    const std::string & search,
                    const std::string & replace) {
  if (search.empty()) return s;
  unsigned int pos = 0;
  while ((pos = s.find(search, pos)) != std::string::npos) {
    s.replace(pos, search.length(), replace);
    pos += replace.length();
  }
  return s;
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
  e.g. explode("a::b", ":") yields {"a", "", "b"}, not {"a", "b"}.

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
#include <cstdio>
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
  char buffer[50];
  assert(string(itoa(1750, buffer, 10)) == "1750");
  assert(string(itoa(1750, buffer, 16)) == "6d6");
  assert(string(itoa(1750, buffer, 2)) == "11011010110");

  string s("   abc \n");
  string t = s;
  assert(ltrim(s) == "abc \n");
  assert(rtrim(s) == trim(t));
  assert(replace("abcdabba", "ab", "00") == "00cd00ba");

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
