/*

Encodes binary data as printable ASCII using Base64, and decodes Base64 text back to bytes. Base64
is an encoding, not encryption or compression: it preserves all input data exactly, but usually
increases size by about one third.

This implementation uses the standard alphabet with `+`, `/`, and `=` padding. Whitespace is not
skipped by the decoder; clean the input first if line breaks or spaces may appear.

- `base64_encode(bytes)` returns the Base64 representation of string `bytes`.
- `base64_decode(text)` returns the decoded byte string. It assumes `text` is valid padded Base64.

Time Complexity:
- O(n) per call, where $n$ is the input length.

Space Complexity:
- O(n) for the output string.

*/

#include <string>
#include <vector>
using std::string;

const char BASE64_ALPHABET[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

string base64_encode(const string &bytes) {
  string res;
  int val = 0, bits = -6;
  for (int i = 0; i < (int)bytes.size(); i++) {
    val = (val << 8) + (unsigned char)bytes[i];
    bits += 8;
    while (bits >= 0) {
      res.push_back(BASE64_ALPHABET[(val >> bits) & 63]);
      bits -= 6;
    }
  }
  if (bits > -6) {
    res.push_back(BASE64_ALPHABET[((val << 8) >> (bits + 8)) & 63]);
  }
  while (res.size() % 4) {
    res.push_back('=');
  }
  return res;
}

string base64_decode(const string &text) {
  std::vector<int> table(256, -1);
  for (int i = 0; i < 64; i++) {
    table[(unsigned char)BASE64_ALPHABET[i]] = i;
  }
  string res;
  int val = 0, bits = -8;
  for (int i = 0; i < (int)text.size(); i++) {
    if (text[i] == '=') {
      break;
    }
    int x = table[(unsigned char)text[i]];
    if (x < 0) {
      break;
    }
    val = (val << 6) + x;
    bits += 6;
    if (bits >= 0) {
      res.push_back((char)((val >> bits) & 255));
      bits -= 8;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(base64_encode("hello") == "aGVsbG8=");
  assert(base64_decode("aGVsbG8=") == "hello");
  assert(base64_decode(base64_encode("abc123!?")) == "abc123!?");
  return 0;
}
