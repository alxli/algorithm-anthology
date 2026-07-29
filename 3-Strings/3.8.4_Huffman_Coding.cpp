/*

Builds a Huffman code for a string and uses it to encode and decode that string. Huffman coding is a
lossless prefix-code compression algorithm: more frequent characters receive shorter bit strings,
and no code is a prefix of another code. The code tree is built greedily with a priority queue by
repeatedly merging the two lowest-frequency subtrees until one tree remains; each character's bit
string is then read off its root-to-leaf path.

The implementation below stores encoded bits as a string of `'0'` and `'1'` characters for clarity.
For real compression, pack those bits into bytes. The tree is also needed to decode the bit string,
so compressed data normally stores enough metadata to reconstruct the same tree.

- `HuffmanTree(text)` constructs a Huffman tree from the character frequencies in `text`.
- `codes()` returns a table mapping each byte value to its bit string.
- `encode(text)` returns the encoded bit string. Every character in `text` must occur in the string
  used to construct the tree.
- `decode(bits)` returns the decoded text for a bit string produced by this tree.

Time Complexity:
- O(n + m log m) per call to the constructor, where $n$ is the input length and $m$ is the number of
  distinct characters.
- O(n + b) per call to `encode()` for $n$ characters producing $b$ bits, and O(b) per call to
  `decode()`.

Space Complexity:
- O(m) for the tree and code table, where $m$ is the number of distinct characters.
- O(n) for the encoded or decoded output.

*/

#include <functional>
#include <queue>
#include <string>
#include <utility>
#include <vector>
using std::string;

class HuffmanTree {
  struct Node {
    int freq;
    unsigned char ch;
    int left, right;

    Node(int freq = 0, unsigned char ch = 0, int left = -1, int right = -1)
        : freq(freq), ch(ch), left(left), right(right) {}
  };

  int root;
  std::vector<Node> nodes;
  std::vector<string> code;

  void build_codes(int u, const string &path) {
    if (nodes[u].left == -1 && nodes[u].right == -1) {
      code[nodes[u].ch] = path.empty() ? "0" : path;
      return;
    }
    build_codes(nodes[u].left, path + '0');
    build_codes(nodes[u].right, path + '1');
  }

 public:
  explicit HuffmanTree(const string &text) : root(-1), code(256) {
    std::vector<int> freq(256, 0);
    for (unsigned char c : text) {
      freq[c]++;
    }
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> pq;
    for (int c = 0; c < 256; c++) {
      if (freq[c] > 0) {
        nodes.emplace_back(freq[c], static_cast<unsigned char>(c));
        pq.emplace(freq[c], static_cast<int>(nodes.size()) - 1);
      }
    }
    if (pq.empty()) {
      return;
    }
    while (static_cast<int>(pq.size()) > 1) {
      int a = pq.top().second;
      pq.pop();
      int b = pq.top().second;
      pq.pop();
      nodes.emplace_back(nodes[a].freq + nodes[b].freq, 0, a, b);
      pq.emplace(nodes.back().freq, static_cast<int>(nodes.size()) - 1);
    }
    root = pq.top().second;
    build_codes(root, "");
  }

  std::vector<string> codes() const { return code; }

  string encode(const string &text) const {
    string bits;
    for (unsigned char c : text) {
      bits += code[c];
    }
    return bits;
  }

  string decode(const string &bits) const {
    string text;
    if (root == -1) {
      return text;
    }
    // Degenerate tree: the input had a single distinct character, so the root is itself a leaf and
    // each symbol was encoded as one bit. Walking children would dereference node -1.
    if (nodes[root].left == -1 && nodes[root].right == -1) {
      return string(bits.size(), static_cast<char>(nodes[root].ch));
    }
    int u = root;
    for (char bit : bits) {
      u = bit == '0' ? nodes[u].left : nodes[u].right;
      if (nodes[u].left == -1 && nodes[u].right == -1) {
        text.push_back(static_cast<char>(nodes[u].ch));
        u = root;
      }
    }
    return text;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  string text = "banana bandana";
  HuffmanTree h(text);
  string bits = h.encode(text);
  assert(h.decode(bits) == text);
  assert(bits.size() < text.size() * 8);

  HuffmanTree single("aaaa");
  assert(single.encode("aaaa") == "0000");
  assert(single.decode(single.encode("aaaa")) == "aaaa");

  HuffmanTree empty("");
  assert(empty.encode("") == "");
  assert(empty.decode("") == "");
  return 0;
}
