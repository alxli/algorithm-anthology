/*

Fast input and output wrappers based on `fread()` and `fwrite()`. For most problems,
`ios::sync_with_stdio(false); cin.tie(nullptr);` is simpler and fast enough. Use these classes when
input is huge or when iostream overhead is measurable.

- `FastInput in(file)` reads tokens from a `FILE*`, defaulting to `stdin`.
- `in >> x` reads a non-whitespace token into `char`, `std::string`, integral types, or floating
  point types.
- `FastOutput out(file)` writes to a `FILE*`, defaulting to `stdout`.
- `out << x` writes `char`, C strings, `std::string`, integral types, or floating point types.
- `out.flush()` writes any buffered output immediately.

The parser assumes valid input. Floating point input/output is provided for convenience, not as the
main performance path.

*/

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>

struct FastInput {
  static constexpr int BUF_SIZE = 1 << 20;
  FILE *file;
  char buf[BUF_SIZE];
  int pos = 0, len = 0;

  explicit FastInput(FILE *file_ = stdin) : file(file_) {}

  char get_char() {
    if (pos == len) {
      len = fread(buf, 1, BUF_SIZE, file);
      pos = 0;
      if (len == 0) {
        return 0;
      }
    }
    return buf[pos++];
  }

  char peek_char() {
    char c = get_char();
    if (c) {
      --pos;
    }
    return c;
  }

  void skip_blanks() {
    while (true) {
      char c = get_char();
      if (!c) {
        return;
      }
      if (!std::isspace(static_cast<unsigned char>(c))) {
        --pos;
        return;
      }
    }
  }

  FastInput &operator>>(char &c) {
    skip_blanks();
    c = get_char();
    return *this;
  }

  FastInput &operator>>(std::string &s) {
    skip_blanks();
    s.clear();
    while (true) {
      char c = get_char();
      if (!c || std::isspace(static_cast<unsigned char>(c))) {
        break;
      }
      s += c;
    }
    return *this;
  }

  FastInput &operator>>(bool &x) {
    int val;
    *this >> val;
    x = val != 0;
    return *this;
  }

  template<class T>
  typename std::enable_if<
      std::is_integral<T>::value && !std::is_same<T, bool>::value, FastInput &>::type
  operator>>(T &x) {
    skip_blanks();
    char c = get_char();
    bool neg = false;
    if (c == '-') {
      neg = true;
      c = get_char();
    }
    using U = typename std::make_unsigned<T>::type;
    U val = 0;
    while (c && !std::isspace(static_cast<unsigned char>(c))) {
      val = U(10) * val + U(c - '0');
      c = get_char();
    }
    if constexpr (std::is_signed<T>::value) {
      x = neg ? T(U(0) - val) : T(val);
    } else {
      x = T(val);
    }
    return *this;
  }

  template<class T>
  typename std::enable_if<std::is_floating_point<T>::value, FastInput &>::type operator>>(T &x) {
    std::string s;
    *this >> s;
    x = static_cast<T>(std::strtod(s.c_str(), nullptr));
    return *this;
  }
};

struct FastOutput {
  static constexpr int BUF_SIZE = 1 << 20;
  FILE *file;
  char buf[BUF_SIZE];
  int pos = 0;

  explicit FastOutput(FILE *file_ = stdout) : file(file_) {}
  ~FastOutput() { flush(); }

  void flush() {
    if (pos) {
      fwrite(buf, 1, pos, file);
      pos = 0;
    }
  }

  void put_char(char c) {
    if (pos == BUF_SIZE) {
      flush();
    }
    buf[pos++] = c;
  }

  FastOutput &operator<<(char c) {
    put_char(c);
    return *this;
  }

  FastOutput &operator<<(const char *s) {
    while (*s) {
      put_char(*s++);
    }
    return *this;
  }

  FastOutput &operator<<(const std::string &s) {
    for (char c : s) {
      put_char(c);
    }
    return *this;
  }

  FastOutput &operator<<(bool x) {
    put_char(x ? '1' : '0');
    return *this;
  }

  template<class T>
  typename std::enable_if<
      std::is_integral<T>::value && !std::is_same<T, bool>::value, FastOutput &>::type
  operator<<(T x) {
    if (x == 0) {
      put_char('0');
      return *this;
    }
    using U = typename std::make_unsigned<T>::type;
    U val;
    if constexpr (std::is_signed<T>::value) {
      if (x < 0) {
        put_char('-');
        val = U(0) - U(x);
      } else {
        val = U(x);
      }
    } else {
      val = x;
    }
    char s[32];
    int n = 0;
    while (val > 0) {
      s[n++] = char('0' + val % 10);
      val /= 10;
    }
    while (n--) {
      put_char(s[n]);
    }
    return *this;
  }

  template<class T>
  typename std::enable_if<std::is_floating_point<T>::value, FastOutput &>::type operator<<(T x) {
    char s[64];
    std::snprintf(s, sizeof(s), "%.17g", static_cast<double>(x));
    return *this << s;
  }
};

/*** Example Usage ***/

int main() {
  FILE *input = tmpfile();
  std::fputs("42 hello 3.5 -2147483648\n", input);
  std::rewind(input);

  FastInput in(input);
  int x;
  std::string s;
  double y;
  int z;
  in >> x >> s >> y >> z;
  assert(x == 42 && s == "hello" && y == 3.5);
  assert(z == std::numeric_limits<int>::min());
  std::fclose(input);

  FILE *output = tmpfile();
  {
    FastOutput out(output);
    out << x << ' ' << s << ' ' << y << ' ' << z << '\n';
    out.flush();
  }
  std::rewind(output);
  char buf[64] = {};
  std::fgets(buf, sizeof(buf), output);
  assert(std::strncmp(buf, "42 hello 3.5 -2147483648", 24) == 0);
  std::fclose(output);
  return 0;
}
