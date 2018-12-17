/*

Common string functions, many of which are substitutes for features which are
not available in standard C++, or may not be available on compilers that do not
support C++11 and later. These operations are naive implementations and often
depend on certain std::string functions that have unspecified complexity.

*/

#include <cstdlib>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using std::string;

/*

Integer Conversion

- to_str(i) returns the string representation of integer i, much like
  std::to_string() in C++11 and later.
- to_int(s) returns the integer representation of string s, much like atoi(),
  except handling special cases of overflow by throwing an exception.
- itoa(value, &str, base) implements the non-standard C function which converts
  value into a C string, storing it into pointer str in the given base. For more
  generalized base conversion, see the math utilities section.

*/

template<class Int>
string to_str(Int i) {
  std::ostringstream oss;
  oss << i;
  return oss.str();
}

int to_int(const string &s) {
  std::istringstream iss(s);
  int res;
  if (!(iss >> res)) {
    throw std::runtime_error("to_int failed");
  }
  return res;
}

char* itoa(int value, char *str, int base = 10) {
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
  if (tmp_v < 0) {
    *ptr++ = '-';
  }
  for (*ptr-- = '\0'; ptr1 < ptr; *ptr1++ = tmp_c) {
    tmp_c = *ptr;
    *ptr-- = *ptr1;
  }
  return str;
}

/*

Case Conversion

- to_upper(s) returns s with all alphabetical characters converted to uppercase.
- to_lower(s) returns s with all alphabetical characters converted to lowercase.
- to_title(s) returns the title case representation of string s, where the first
  letter of every word (consecutive alphabetical characters) is capitalized.

*/

string to_upper(const string &s) {
  string res;
  for (int i = 0; i < (int)s.size(); i++) {
    res.push_back(toupper(s[i]));
  }
  return res;
}

string to_lower(const string &s) {
  string res;
  for (int i = 0; i < (int)s.size(); i++) {
    res.push_back(tolower(s[i]));
  }
  return res;
}

string to_title(const string &s) {
  string res;
  char prev = '\0';
  for (int i = 0; i < (int)s.size(); i++) {
    if (isalpha(prev)) {
      res.push_back(tolower(s[i]));
    } else {
      res.push_back(toupper(s[i]));
    }
    prev = res.back();
  }
  return res;
}

/*

Stripping

- lstrip(s) strips the left side of s in-place (that is, the input is modified)
  using the given delimiters and returns a reference to the stripped string.
- rstrip(s) strips the right side of s in-place using the given delimiters and
  returns a reference to the stripped string.
- strip(s) strips both sides of s in-place and returns a reference to the
  stripped string.

*/

string& lstrip(string &s, const string &delim = " \n\t\v\f\r") {
  size_t pos = s.find_first_not_of(delim);
  if (pos != string::npos) {
    s.erase(0, pos);
  }
  return s;
}

string& rstrip(string &s, const string &delim = " \n\t\v\f\r") {
  size_t pos = s.find_last_not_of(delim);
  if (pos != string::npos) {
    s.erase(pos);
  }
  return s;
}

string& strip(string &s, const string &delim = " \n\t\v\f\r") {
  return lstrip(rstrip(s));
}

/*

Find and Replace

- find_all(haystack, needle) returns a vector of all positions where the string
  needle appears in the string haystack.
- replace(s, old, replacement) returns a copy of s with all occurrences of the
  string old replaced with the given replacement.

*/

std::vector<int> find_all(const string &haystack, const string &needle) {
  std::vector<int> res;
  size_t pos = haystack.find(needle, 0);
  while (pos != string::npos) {
      res.push_back(pos);
      pos = haystack.find(needle, pos + 1);
  }
  return res;
}

string replace(const string &s, const string &old, const string &replacement) {
  if (old.empty()) {
    return s;
  }
  string res(s);
  size_t pos = 0;
  while ((pos = res.find(old, pos)) != string::npos) {
    res.replace(pos, old.length(), replacement);
    pos += replacement.length();
  }
  return res;
}

/*

Joining and Splitting

- join(v, delim) returns the strings in vector v concatenated, separated by the
  given delimiter.
- split(s, char delim) returns a vector of tokens of s, split on a single
  character delimiter. Note that this version will not skip empty tokens. For
  example, split("a::b", ":") returns {"a", "b"}, not {"a", "", "b"}.
- split(s, string delim) returns a vector of tokens of s, split on a set of many
  possible single character delimiters. All characters ofz delim will be removed
  from s, and the remaining token(s) of s will be added sequentially to a vector
  and returned. Unlike the first version, empty tokens are skipped. For example,
  split("a::b", ":") returns {"a", "b"}, not {"a", "", "b"}.
- explode(s, delim) returns a vector of tokens of s, split on the entire
  delimiter string delim. Unlike the split() functions above, delim is treated
  as a contiguous boundary string, not merely a set of possible boundary
  characters. This will not skip empty tokens. For example,
  explode("a::::b", "::") yields {"a", "", "b"}, not {"a", "b"}.

*/

string join(const std::vector<string> &v, const string &delim = " ") {
  string res;
  for (int i = 0; i < (int)v.size(); i++) {
    if (i > 0) {
      res += delim;
    }
    res += v[i];
  }
  return res;
}

std::vector<string> split(const string &s, char delim) {
  std::vector<string> res;
  std::stringstream ss(s);
  string curr;
  while (std::getline(ss, curr, delim)) {
    res.push_back(curr);
  }
  return res;
}

std::vector<string> split(const string &s,
                          const string &delim = " \n\t\v\f\r") {
  std::vector<string> res;
  string curr;
  for (int i = 0; i < (int)s.size(); i++) {
    if (delim.find(s[i]) == string::npos) {
      curr += s[i];
    } else if (!curr.empty()) {
      res.push_back(curr);
      curr = "";
    }
  }
  if (!curr.empty()) {
    res.push_back(curr);
  }
  return res;
}

std::vector<string> explode(const string &s, const string &delim) {
  std::vector<string> res;
  size_t last = 0, next = 0;
  while ((next = s.find(delim, last)) != string::npos) {
    res.push_back(s.substr(last, (int)next - last));
    last = next + delim.size();
  }
  res.push_back(s.substr(last));
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(to_str(123) + "4" == "1234");
  assert(to_int("1234") == 1234);
  char buffer[50];
  assert(string(itoa(1750, buffer, 10)) == "1750");
  assert(string(itoa(1750, buffer, 16)) == "6d6");
  assert(string(itoa(1750, buffer, 2)) == "11011010110");

  assert(to_upper("Hello world") == "HELLO WORLD");
  assert(to_lower("Hello World") == "hello world");
  assert(to_title("hello world") == "Hello World");

  string s("   abc \n");
  string t = s;
  assert(lstrip(s) == "abc \n");
  assert(rstrip(s) == strip(t));

  vector<int> pos;
  pos.push_back(0);
  pos.push_back(7);
  assert(find_all("abracadabra", "ab") == pos);
  assert(replace("abcdabba", "ab", "00") == "00cd00ba");

  assert(join(split("a\nb\ncde\nf", '\n'), "|") == "a|b|cde|f");  // split v1
  assert(join(split("a::b,cde:,f", ":,"), "|") == "a|b|cde|f");  // split v2
  assert(join(explode("a..b.cde....f", ".."), "|") == "a|b.cde||f");
  return 0;
}
