/*

Local-only debug printing for contest code. The macro expands to a no-op unless `LOCAL` is defined
(e.g. by passing `-DLOCAL` to the compiler flags), so it can remain in submitted code without
producing output.

- `dbg_repr(x)` returns a string representation of arithmetic types, strings, pairs, tuples, and
  iterable containers.
- `dbg(a, b, c)` prints the function name, line number, argument names, and values to `std::cerr`
  when `LOCAL` is defined.
- `pr(a, b, c)` prints only the values to `std::cerr` when `LOCAL` is defined.
- `dbg(...)` evaluates to no output when `LOCAL` is not defined.
- `pr(...)` evaluates to no output when `LOCAL` is not defined.

Avoid naming these helpers `to_string()` unless you want them mixed into overload resolution with
`std::to_string()` and other user-defined `to_string()` functions. A separate name keeps debug
formatting local to this snippet.

*/

#include <cassert>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

template<class T, class = void>
struct is_iterable : std::false_type {};

template<class T>
struct is_iterable<
    T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>>
    : std::true_type {};

std::string dbg_repr(const std::string &s) {
  return '"' + s + '"';
}

std::string dbg_repr(const char *s) {
  return dbg_repr(std::string(s));
}

std::string dbg_repr(char c) {
  return std::string("'") + c + "'";
}

std::string dbg_repr(bool b) {
  return b ? "true" : "false";
}

template<class T>
typename std::enable_if<
    std::is_arithmetic<T>::value && !std::is_same<T, char>::value &&
        !std::is_same<T, bool>::value && !std::is_floating_point<T>::value,
    std::string>::type
dbg_repr(T x) {
  std::ostringstream out;
  out << x;
  return out.str();
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value, std::string>::type dbg_repr(T x) {
  std::ostringstream out;
  out << std::setprecision(10) << x;  // Set floating point dbg precision here.
  return out.str();
}

template<class A, class B>
std::string dbg_repr(const std::pair<A, B> &p) {
  return "(" + dbg_repr(p.first) + ", " + dbg_repr(p.second) + ")";
}

template<class Tuple, size_t... Is>
std::string dbg_tuple_repr(const Tuple &t, std::index_sequence<Is...>) {
  std::string res = "(";
  bool first = true;
  ((res += (first ? (first = false, "") : ", ") + dbg_repr(std::get<Is>(t))), ...);
  return res + ")";
}

template<class... Ts>
std::string dbg_repr(const std::tuple<Ts...> &t) {
  return dbg_tuple_repr(t, std::index_sequence_for<Ts...>{});
}

template<class T>
typename std::enable_if<
    is_iterable<T>::value && !std::is_convertible<T, std::string>::value, std::string>::type
dbg_repr(const T &v) {
  std::string res = "{";
  bool first = true;
  for (const auto &x : v) {
    if (!first) {
      res += ", ";
    }
    first = false;
    res += dbg_repr(x);
  }
  return res + "}";
}

void dbg_out(bool) {
  std::cerr << '\n';
}

template<class Head, class... Tail>
void dbg_out(bool leading_space, const Head &head, const Tail &...tail) {
  if (leading_space) {
    std::cerr << ' ';
  }
  std::cerr << dbg_repr(head);
  ((std::cerr << ' ' << dbg_repr(tail)), ...);
  std::cerr << '\n';
}

#ifdef LOCAL
#define dbg(...)                                                                  \
  std::cerr << "[" << __func__ << ":" << __LINE__ << "] " << #__VA_ARGS__ << ":", \
      dbg_out(true, __VA_ARGS__)
#define pr(...) dbg_out(false, __VA_ARGS__)
#else
#define dbg(...) ((void)0)
#define pr(...) ((void)0)
#endif

/*** Example Usage and Output:

[main:175] v, mp: {1, 2, 3} {("a", 1), ("b", 2)}
[main:176] p, t: (4, "x") (1, true, "ok")
[main:177] pt, e: (2, 3) {from=0, to=1, weight=5}
"plain output" 3.141592654

***/

struct Point {
  int x, y;
};

// Can overload dbg_repr to dbg your custom structs.
std::string dbg_repr(const Point &p) {
  return "(" + dbg_repr(p.x) + ", " + dbg_repr(p.y) + ")";
}

struct Edge {
  int from, to, weight;
};

std::string dbg_repr(const Edge &e) {
  return "{from=" + dbg_repr(e.from) + ", to=" + dbg_repr(e.to) + ", weight=" + dbg_repr(e.weight) +
         "}";
}

int main() {
  std::vector<int> v = {1, 2, 3};
  std::map<std::string, int> mp = {{"a", 1}, {"b", 2}};
  auto p = std::make_pair(4, std::string("x"));
  auto t = std::make_tuple(1, true, std::string("ok"));
  Point pt{2, 3};
  Edge e{0, 1, 5};
  assert(dbg_repr(v) == "{1, 2, 3}");
  assert(dbg_repr(p) == "(4, \"x\")");
  assert(dbg_repr(t) == "(1, true, \"ok\")");
  assert(dbg_repr(mp) == "{(\"a\", 1), (\"b\", 2)}");
  assert(dbg_repr(pt) == "(2, 3)");
  assert(dbg_repr(e) == "{from=0, to=1, weight=5}");
  double d = 3.141592653589793;
  dbg(v, mp);
  dbg(p, t);
  dbg(pt, e);
  pr("plain output", d);
  assert(dbg_repr(1.0 / 3.0) == "0.3333333333");
  assert(dbg_repr(d) == "3.141592654");
  return 0;
}
