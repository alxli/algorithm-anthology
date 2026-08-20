/*

A de Bruijn sequence $B(k, n)$ is a cyclic string over the alphabet $[0, k)$ of length $k^n$ in
which every one of the $k^n$ possible length-$n$ strings appears exactly once as a (cyclic)
substring. Reading a window of $n$ symbols and advancing one position at a time therefore cycles
through all length-$n$ strings without repetition, which makes the sequence a compact way to probe
every short pattern: cracking a rotating lock, addressing a shift register, or building the
perfect-hash table that maps an isolated set bit to its index.

The construction here is the Fredricksen-Kessler-Maiorana (FKM) algorithm. It concatenates, in
lexicographic order, the Lyndon words (see the Lyndon factorization section) whose length divides
$n$. The result is the lexicographically smallest de Bruijn sequence. The recursion enumerates these
necklace representatives directly, appending the first $p$ symbols of each candidate whenever its
period $p$ divides $n$.

- `de_bruijn(k, n)` returns a vector of length $k^n$ holding one period of $B(k, n)$. Each entry
  lies in $[0, k)$. Both $k$ and $n$ must be at least $1$. Treat the result as cyclic: the
  length-$n$ windows that wrap past the end are exactly the ones still needed to cover all strings.

Time Complexity:
- O(k^n) per call, which is optimal since the output itself has length $k^n$.

Space Complexity:
- O(k^n) for the returned sequence, plus O(n) auxiliary stack space for the recursion.

*/

#include <cassert>
#include <vector>

std::vector<int> de_bruijn(int k, int n) {
  assert(k >= 1 && n >= 1);
  std::vector<int> word(n + 1), seq;
  auto dfs = [&](auto &&dfs, int t, int p) {
    if (t > n) {
      if (n % p == 0) {
        for (int i = 1; i <= p; i++) {
          seq.push_back(word[i]);
        }
      }
      return;
    }
    word[t] = word[t - p];
    dfs(dfs, t + 1, p);
    for (int j = word[t - p] + 1; j < k; j++) {
      word[t] = j;
      dfs(dfs, t + 1, t);
    }
  };
  dfs(dfs, 1, 1);
  return seq;
}

/*** Example Usage ***/

#include <set>
#include <vector>
using namespace std;

int main() {
  // B(2, 3) has length 8 and every 3-bit string appears once cyclically.
  assert((de_bruijn(2, 3) == vector<int>{0, 0, 0, 1, 0, 1, 1, 1}));

  for (int k = 2; k <= 4; k++) {
    for (int n = 1; n <= 4; n++) {
      vector<int> s = de_bruijn(k, n);
      int len = 1;
      for (int i = 0; i < n; i++) {
        len *= k;
      }
      assert(static_cast<int>(s.size()) == len);

      // Collect every length-n window, wrapping cyclically, and confirm all k^n are distinct.
      set<vector<int>> windows;
      for (int i = 0; i < len; i++) {
        vector<int> w(n);
        for (int j = 0; j < n; j++) {
          w[j] = s[(i + j) % len];
        }
        windows.insert(w);
      }
      assert(static_cast<int>(windows.size()) == len);
    }
  }
  return 0;
}
