/*

Builds a Huffman code for a string and uses it to encode and decode that string. Huffman coding is a
lossless prefix-code compression algorithm: more frequent characters receive shorter bit strings,
and no code is a prefix of another code.

The implementation below stores encoded bits as a string of `'0'` and `'1'` characters for clarity.
For real compression, pack those bits into bytes. The tree is also needed to decode the bit string,
so compressed data normally stores enough metadata to reconstruct the same tree.

- `HuffmanTree(text)` constructs a Huffman tree from the character frequencies in `text`.
- `codes()` returns a table mapping each byte value to its bit string.
- `encode(text)` returns the encoded bit string.
- `decode(bits)` returns the decoded text.

Time Complexity:
- O(n + A log A) to construct the tree, where $n$ is the input length and $A$ is the number of
  distinct characters.
- O(n) to encode or decode, plus the number of produced or consumed bits.

Space Complexity:
- O(A) for the tree and code table.
- O(n) for the encoded or decoded output.

*/

#include <queue>
#include <string>
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

  struct ByFrequency {
    const std::vector<Node> *nodes;
    explicit ByFrequency(const std::vector<Node> *nodes = nullptr) : nodes(nodes) {}
    bool operator()(int a, int b) const { return (*nodes)[a].freq > (*nodes)[b].freq; }
  };

  std::vector<Node> nodes;
  int root;
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
    for (int i = 0; i < static_cast<int>(text.size()); i++) {
      freq[static_cast<unsigned char>(text[i])]++;
    }
    ByFrequency cmp(&nodes);
    std::priority_queue<int, std::vector<int>, ByFrequency> pq(cmp);
    for (int c = 0; c < 256; c++) {
      if (freq[c] > 0) {
        nodes.push_back(Node(freq[c], static_cast<unsigned char>(c)));
        pq.push(static_cast<int>(nodes.size()) - 1);
      }
    }
    if (pq.empty()) {
      return;
    }
    while (pq.size() > 1) {
      int a = pq.top();
      pq.pop();
      int b = pq.top();
      pq.pop();
      nodes.emplace_back(nodes[a].freq + nodes[b].freq, 0, a, b);
      pq.push(static_cast<int>(nodes.size()) - 1);
    }
    root = pq.top();
    build_codes(root, "");
  }

  std::vector<string> codes() const { return code; }

  string encode(const string &text) const {
    string bits;
    for (int i = 0; i < static_cast<int>(text.size()); i++) {
      bits += code[static_cast<unsigned char>(text[i])];
    }
    return bits;
  }

  string decode(const string &bits) const {
    string text;
    if (root == -1) {
      return text;
    }
    int u = root;
    for (int i = 0; i < static_cast<int>(bits.size()); i++) {
      u = bits[i] == '0' ? nodes[u].left : nodes[u].right;
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
  return 0;
}
