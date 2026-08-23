/*

Fast input and output helpers. For most cases, it is enough to speed up standard I/O simply with
`ios::sync_with_stdio(false); cin.tie(nullptr);`. The following classes only come into play when
input is genuinely huge or `iostream` overhead is measurable.

- `set_in(name)`, `set_out(name)`, and `set_io(iname, oname)` redirect standard input/output to
  files. For example, `set_io("task.in", "task.out")` is convenient for USACO-style problems. An
  empty name leaves that stream untouched, so `set_io("", "")` is a no-op that can stay in place
  when the same solution is submitted to a judge that pipes its input.

File-redirection failures throw `std::runtime_error`.

*/

#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

void set_in(const std::string &name) {
  if (!name.empty() && !freopen(name.c_str(), "r", stdin)) {
    throw std::runtime_error("Failed to open input file: " + name);
  }
}

void set_out(const std::string &name) {
  if (!name.empty() && !freopen(name.c_str(), "w", stdout)) {
    throw std::runtime_error("Failed to open output file: " + name);
  }
}

void set_io(const std::string &iname, const std::string &oname) {
  set_in(iname);
  set_out(oname);
}

/*

When only integers are read, one loop over a character reader is the whole technique and is short
enough to copy or retype: skip anything that starts no number, then fold digits in with
`x * 10 + d`. Which character reader it uses decides everything, since the per-character mutex that
the locked `getchar()` takes costs more than the parsing does. The example below times the
alternatives against each other on the same input.

- `read_int(x)` reads one integer from `stdin` into `x`, skipping any leading character that cannot
  start a number. A leading `-` is honored, so `Int` must be signed for negative input. Behavior is
  undefined at end of file.

*/

// The unlocked reader is where nearly all of the speed comes from. It is POSIX, not standard C++.
inline int read_char() {
#if defined(_WIN32)
  return _getchar_nolock();
#elif defined(__unix__) || defined(__APPLE__)
  return getchar_unlocked();
#else
  return getchar();
#endif
}

template<typename Int>
void read_int(Int &x) {
  int c;
  while ((c = read_char()) != '-' && (c < '0' || c > '9')) {
  }
  bool neg = c == '-';
  if (neg) {
    c = read_char();
  }
  for (x = 0; '0' <= c && c <= '9'; c = read_char()) {
    x = x * 10 - (c - '0');  // Accumulate negatively to prevent INT_MIN overflow.
  }
  x = neg ? x : -x;
}

/*

`FastInput` reads large byte blocks with `fread()` and parses tokens from an internal buffer.

- `FastInput in(file = stdin)` constructs a faster reader from a non-null `FILE*`.
- `in >> x` reads a non-whitespace token into `char`, `std::string`, integral types, or
  floating-point types.

Detected read failures throw `std::runtime_error`. End-of-file is not an error, and the parser
otherwise assumes valid input. Floating-point input is provided for convenience, not as the main
performance path.

*/

class FastInput {
  static constexpr int BUF_SIZE = 1 << 20;
  FILE *file;
  char buf[BUF_SIZE];
  int pos = 0, len = 0;

  char get_char() {
    if (pos == len) {
      len = static_cast<int>(fread(buf, 1, BUF_SIZE, file));
      pos = 0;
      if (len == 0) {
        if (ferror(file)) {
          throw std::runtime_error("Failed to read input.");
        }
        return 0;
      }
    }
    return buf[pos++];
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

 public:
  explicit FastInput(FILE *file_ = stdin) : file(file_) { assert(file != nullptr); }

  FastInput(const FastInput &) = delete;
  FastInput &operator=(const FastInput &) = delete;

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

  template<typename T>
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
    T val = 0;
    while (c && !std::isspace(static_cast<unsigned char>(c))) {
      val = T{10} * val - T(c - '0');  // Accumulate negatively to prevent INT_MIN overflow.
      c = get_char();
    }
    x = neg ? val : -val;
    return *this;
  }

  template<typename T>
  typename std::enable_if<std::is_floating_point<T>::value, FastInput &>::type operator>>(T &x) {
    std::string s;
    *this >> s;
    x = static_cast<T>(std::strtold(s.c_str(), nullptr));
    return *this;
  }
};

/*

`FastOutput` formats values into an internal buffer and writes them in large blocks with `fwrite()`.

- `FastOutput out(file = stdout)` constructs a faster writer to a non-null `FILE*`.
- `out << x` writes `char`, C strings, `std::string`, integral types, or floating-point types.
- `out.flush()` writes any buffered output immediately.

Detected write failures throw `std::runtime_error`. Destruction flushes on a best-effort basis; call
`flush()` explicitly to detect a final write failure. Floating-point output is provided for
convenience, not as the main performance path.

*/

class FastOutput {
  static constexpr int BUF_SIZE = 1 << 20;
  FILE *file;
  char buf[BUF_SIZE];
  int pos = 0;

  bool flush_buf() {
    int n = pos;
    pos = 0;
    return fwrite(buf, 1, n, file) == static_cast<std::size_t>(n);
  }

  void put_char(char c) {
    if (pos == BUF_SIZE) {
      flush();
    }
    buf[pos++] = c;
  }

 public:
  explicit FastOutput(FILE *file_ = stdout) : file(file_) { assert(file != nullptr); }

  ~FastOutput() { flush_buf(); }
  FastOutput(const FastOutput &) = delete;
  FastOutput &operator=(const FastOutput &) = delete;

  void flush() {
    if (!flush_buf()) {
      throw std::runtime_error("Failed to write output.");
    }
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

  template<typename T>
  typename std::enable_if<
      std::is_integral<T>::value && !std::is_same<T, bool>::value, FastOutput &>::type
  operator<<(T x) {
    using U = typename std::make_unsigned<T>::type;
    U val = U(x);
    if constexpr (std::is_signed<T>::value) {
      if (x < 0) {
        put_char('-');
        val = U{0} - val;
      }
    }
    char s[std::numeric_limits<U>::digits10 + 1];
    int n = 0;
    do {
      s[n++] = char('0' + val % 10);
      val /= 10;
    } while (val);
    while (n--) {
      put_char(s[n]);
    }
    return *this;
  }

  template<typename T>
  typename std::enable_if<std::is_floating_point<T>::value, FastOutput &>::type operator<<(T x) {
    char s[64];
    std::snprintf(
        s, sizeof(s), "%.*Lg", std::numeric_limits<T>::max_digits10, static_cast<long double>(x)
    );
    return *this << s;
  }
};

/*** Example Usage and Output:

Reading 1000000 integers:
cin                     0.959 s
scanf                   0.061 s
read_int (unlocked)     0.010 s
FastInput               0.021 s

Writing 1000000 integers:
ofstream                0.037 s
fprintf                 0.071 s
FastOutput              0.009 s

***/

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace std;

int main() {
  // set_io("file.in", "file.out");

  FILE *input = tmpfile();
  fputs("42 hello 3.5 -2147483648 1 Z 1234567890123\n", input);
  rewind(input);

  FastInput in(input);
  int x;
  string s;
  double y;
  int z;
  bool flag;
  char ch;
  long long big;
  in >> x >> s >> y >> z >> flag >> ch >> big;
  assert(x == 42 && s == "hello" && y == 3.5 && z == -2147483648);
  assert(flag && ch == 'Z' && big == 1234567890123LL);
  fclose(input);

  FILE *output = tmpfile();
  {
    FastOutput out(output);
    out << x << ' ' << s << ' ' << y << ' ' << z << ' ' << flag << ' ' << ch << ' ' << big << '\n';
    out.flush();
  }
  rewind(output);
  char buf[96] = {};
  assert(fgets(buf, sizeof(buf), output));
  assert(string(buf) == "42 hello 3.5 -2147483648 1 Z 1234567890123\n");
  fclose(output);

  // Benchmark various ways of reading the same million integers from a scratch file.
  const int n = 1000000;
  string text;
  for (int i = 0; i < n; i++) {
    text += to_string(i) + ' ';
  }
  string path = (filesystem::temp_directory_path() / "fast_io.tmp").string();
  ofstream(path) << text;
  auto time_reads = [&](const char *name, auto read_one) {
    set_in(path);
    clock_t start = clock();
    for (int i = 0, v = 0; i < n; i++) read_one(v);
    printf("%-22s %6.3f s\n", name, double(clock() - start) / CLOCKS_PER_SEC);
  };
  printf("Reading %d integers:\n", n);
  time_reads("cin", [](int &v) { cin >> v; });
  time_reads("scanf", [](int &v) { scanf("%d", &v); });
  time_reads("read_int (unlocked)", [](int &v) { read_int(v); });
  time_reads("FastInput", [](int &v) {
    static FastInput in;
    in >> v;
  });

  auto time_writes = [&](const char *name, auto write_all) {
    clock_t start = clock();
    write_all(n);
    printf("%-22s %6.3f s\n", name, double(clock() - start) / CLOCKS_PER_SEC);
  };
  printf("\nWriting %d integers:\n", n);
  time_writes("ofstream", [&](int k) {
    ofstream out(path);
    for (int i = 0; i < k; i++) out << i << ' ';
  });
  time_writes("fprintf", [&](int k) {
    FILE *f = fopen(path.c_str(), "w");
    for (int i = 0; i < k; i++) fprintf(f, "%d ", i);
    fclose(f);
  });
  time_writes("FastOutput", [&](int k) {
    FILE *f = fopen(path.c_str(), "w");
    {
      FastOutput out(f);
      for (int i = 0; i < k; i++) out << i << ' ';
    }
    fclose(f);
  });
  remove(path.c_str());
  return 0;
}
