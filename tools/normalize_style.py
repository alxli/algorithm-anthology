#!/usr/bin/env python3
"""
Apply all pending style normalizations to chapter .cpp files.

Changes:
  1. snake_case class names → UpperCamelCase (per-file)
  2. node_t → Node, edge_t → Edge (global)
  3. NULL → nullptr (global)
  4. C-style casts → static_cast (in code sections only)
  5. Range params l/r/h → lo/hi (targeted files)
  6. ans/ret → res/best (targeted files)
  7. INF = 0x3f3f3f3f → static const int INF = INT_MAX / 2 (graph files)
  8. Graph node labels a/b → u/v (targeted files)
  9. Template params Int/Double/Matrix/etc. → T/F (targeted files)
 10. using namespace std in 6.1 example

Usage:  python3 tools/normalize_style.py
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).parent.parent


# ── Helpers ───────────────────────────────────────────────────────────────────

def word_replace(src: str, old: str, new: str) -> str:
    return re.sub(r'\b' + re.escape(old) + r'\b', new, src)


def code_only_replace(src: str, pattern: str, repl, flags: int = 0) -> str:
    """Apply a regex substitution only in non-comment, non-string sections."""
    skip = re.compile(
        r'//[^\n]*|/\*.*?\*|"(?:[^"\\]|\\.)*"|\'(?:[^\'\\]|\\.)*\'',
        re.DOTALL,
    )
    result, pos = [], 0
    for m in skip.finditer(src):
        result.append(re.sub(pattern, repl, src[pos:m.start()], flags=flags))
        result.append(m.group())
        pos = m.end()
    result.append(re.sub(pattern, repl, src[pos:], flags=flags))
    return ''.join(result)


def scan_postfix(src: str, pos: int) -> int:
    """Return the end position of a postfix expression starting at pos."""
    n = len(src)
    original_pos = pos  # saved so we can return it when nothing matches
    # Skip leading whitespace (only once, right after cast)
    while pos < n and src[pos] == ' ':
        pos += 1
    if pos >= n:
        return original_pos
    # Parenthesized expression: (...)
    if src[pos] == '(':
        depth, i = 1, pos + 1
        while i < n and depth > 0:
            if src[i] == '(':
                depth += 1
            elif src[i] == ')':
                depth -= 1
            i += 1
        return i
    # Identifier / numeric literal
    i = pos
    while i < n and (src[i].isalnum() or src[i] == '_'):
        i += 1
    if i == pos:
        return original_pos  # nothing matched; return pre-space position
    # Member access / scope resolution / call / subscript chains
    while i < n:
        if src[i:i+2] == '::':  # namespace/class scope resolution
            j = i + 2
            while j < n and (src[j].isalnum() or src[j] == '_'):
                j += 1
            if j == i + 2:
                break
            i = j
            continue
        if src[i] in '.':
            j = i + 1
            while j < n and (src[j].isalnum() or src[j] == '_'):
                j += 1
            if j == i + 1:
                break
            i = j
        elif src[i:i+2] == '->':
            j = i + 2
            while j < n and (src[j].isalnum() or src[j] == '_'):
                j += 1
            i = j
        elif src[i] == '(':
            depth, j = 1, i + 1
            while j < n and depth > 0:
                if src[j] == '(':
                    depth += 1
                elif src[j] == ')':
                    depth -= 1
                j += 1
            i = j
        elif src[i] == '[':
            depth, j = 1, i + 1
            while j < n and depth > 0:
                if src[j] == '[':
                    depth += 1
                elif src[j] == ']':
                    depth -= 1
                j += 1
            i = j
        else:
            break
    return i


def convert_casts(src: str) -> str:
    """Convert C-style casts to static_cast in code sections only."""
    types = [
        'unsigned long long', 'long long', 'unsigned char', 'unsigned int',
        'long double', 'double', 'float', 'size_t', 'int', 'char', 'unsigned',
    ]
    skip = re.compile(
        r'//[^\n]*|/\*.*?\*/|"(?:[^"\\]|\\.)*"|\'(?:[^\'\\]|\\.)*\'',
        re.DOTALL,
    )
    # Build set of comment/string spans to skip
    skip_spans = [(m.start(), m.end()) for m in skip.finditer(src)]

    def in_skip(pos):
        for s, e in skip_spans:
            if s <= pos < e:
                return True
        return False

    for ctype in sorted(types, key=len, reverse=True):
        pat = re.compile(r'\(' + re.escape(ctype) + r'\)')
        result, pos = [], 0
        for m in pat.finditer(src):
            if in_skip(m.start()):
                continue
            result.append(src[pos:m.start()])
            expr_end = scan_postfix(src, m.end())
            if expr_end > m.end():
                expr = src[m.end():expr_end]
                result.append(f'static_cast<{ctype}>({expr})')
                pos = expr_end
            else:
                result.append(m.group())
                pos = m.end()
        result.append(src[pos:])
        src = ''.join(result)
        # Rebuild skip spans for next type
        skip_spans = [(m.start(), m.end()) for m in skip.finditer(src)]
    return src


# ── 1. Class rename map ───────────────────────────────────────────────────────

CLASS_RENAMES = {
    # Ch1
    'difference_array': 'DifferenceArray',
    'dynamic_mex': 'DynamicMex',
    'online_statistics': 'OnlineStatistics',
    # Ch2 – heaps
    'binary_heap': 'BinaryHeap',
    'skew_heap': 'SkewHeap',
    'pairing_heap': 'PairingHeap',
    # Ch2 – BSTs and friends
    'binary_search_tree': 'BST',
    'avl_tree': 'AVLTree',
    'red_black_tree': 'RedBlackTree',
    'splay_tree': 'SplayTree',
    'treap': 'Treap',
    'size_balanced_tree': 'SBTree',
    'interval_treap': 'IntervalTreap',
    'implicit_treap': 'ImplicitTreap',
    'cartesian_tree': 'CartesianTree',
    'hash_map': 'HashMap',
    'skip_list': 'SkipList',
    'lru_cache': 'LRUCache',
    # Ch2 – range query structures
    'sqrt_decomposition': 'SqrtDecomposition',
    'segment_tree_2d': 'SegTree2D',     # must come before segment_tree
    'segment_tree': 'SegTree',
    'persistent_segment_tree': 'PersistentSegTree',
    'quadtree': 'Quadtree',
    'range_tree': 'RangeTree',
    'kd_tree': 'KDTree',
    'fenwick_tree_2d': 'FenwickTree2D', # must come before fenwick_tree
    'fenwick_tree': 'FenwickTree',
    # Ch2 – DSU / trees
    'disjoint_set_forest': 'DisjointSetForest',
    'rollback_dsu': 'RollbackDSU',
    'slot_dsu': 'SlotDSU',
    'heavy_light': 'HeavyLight',
    'link_cut_forest': 'LinkCutForest',
    # Ch3
    'monotone_queue': 'MonotoneQueue',
    'li_chao_tree': 'LiChaoTree',
    'hull_optimizer': 'HullOptimizer',
    # Ch4
    'rolling_value_hasher': 'RollingValueHasher', # before rolling_hash
    'rolling_hash': 'RollingHash',
    'generic_hasher': 'GenericHasher',
    'int_hasher': 'IntHasher',
    'scalar_hasher': 'ScalarHasher',
    'pair_hasher': 'PairHasher',
    'vector_hasher': 'VectorHasher',
    'point_hasher': 'PointHasher',
    'class_hash': 'ClassHash',
    'zobrist_hash': 'ZobristHash',
    'kmp': 'KMP',
    'aho_corasick': 'AhoCorasick',
    'manacher': 'Manacher',
    'trie': 'Trie',
    'radix_tree': 'RadixTree',
    'suffix_automaton': 'SuffixAutomaton',  # before suffix_array
    'suffix_array': 'SuffixArray',
    'eertree': 'Eertree',
    'huffman_tree': 'HuffmanTree',
    # Ch4 – enumeration helpers
    'abstract_enumerator': 'AbstractEnumerator',
    'arrangement_enumerator': 'ArrangementEnumerator',
    'combination_enumerator': 'CombinationEnumerator',
    'partition_enumerator': 'PartitionEnumerator',
    'permutation_enumerator': 'PermutationEnumerator',
    # Ch5
    'graph': 'Graph',
    'online_bridges': 'OnlineBridges',
    'two_sat': 'TwoSAT',
    'min_cost_max_flow': 'MinCostMaxFlow',
    # Ch6
    'modular': 'Modular',
    'mod_combinatorics': 'ModCombinatorics',
    'bigint': 'BigInt',
    'rational': 'Rational',
    # Ch7 – geometry structs are already named: point, line, circle, triangle,
    # rectangle, segment – rename those too for consistency
    'circle': 'Circle',
    'triangle': 'Triangle',
    'rectangle': 'Rectangle',
    'segment': 'Segment',
}

# ── 2. Inner struct renames (global) ─────────────────────────────────────────
STRUCT_RENAMES = {
    'node_t': 'Node',
    'edge_t': 'Edge',
    'list_node': 'ListNode',
}

# ── 5. Targeted range param fixes ────────────────────────────────────────────
RANGE_FIXES: dict[str, list[tuple[str, str]]] = {
    '1-Elementary-Algorithms/1.2.1_Prefix_Sums.cpp': [
        (r'\brange_sum\(([^,]+),\s*int l,\s*int r\)',
         r'range_sum(\1, int lo, int hi)'),
        (r'\brange_sum\(([^,]+),\s*l,\s*r\)', r'range_sum(\1, lo, hi)'),
        (r'\bpref\[r\]\s*-\s*pref\[l\]', r'pref[hi] - pref[lo]'),
        # The for-loop `for (int r = 0; r < rows; r++)` — `r` is row index here
        # leave it (already named `r` for row, not range end)
    ],
    '1-Elementary-Algorithms/1.2.2_Difference_Array.cpp': [
        (r'\bvoid add\(int l,\s*int r,', r'void add(int lo, int hi,'),
        (r'\bvoid add\(int l,\s*int r\b', r'void add(int lo, int hi'),
        (r'(?<!\w)l(?!\w)(?=\s*[,)]|\s*\+\+|\s*<|\s*<=)', None),  # complex, skip
    ],
    '1-Elementary-Algorithms/1.2.3_Longest_Increasing_Subsequence.cpp': [
        (r'\bint l\s*=\s*-1,\s*h\s*=\s*len', r'int lo = -1, hi = len'),
        (r'\bint l\s*=\s*-1\s*,\s*h\s*=', r'int lo = -1, hi ='),
        (r'(?<!\w)l\s*\+\s*\(\s*h\s*-\s*l\s*\)', r'lo + (hi - lo)'),
        (r'\bwhile\s*\(\s*h\s*-\s*l\s*>', r'while (hi - lo >'),
        (r'(?<=\s)h\b', r'hi'), (r'(?<=\()h\b', r'hi'),
        (r'(?<=,\s)l\b', r'lo'), (r'(?<=\()l\b', r'lo'),
        (r'\bl\s*<\s*h\b', r'lo < hi'),
    ],
    '1-Elementary-Algorithms/1.2.4_Subset_Sum_(Meet-in-the-Middle).cpp': [
        (r'\bint l\s*=\s*0,\s*h\s*=', r'int lo = 0, hi ='),
        (r'\bl\s*\+\s*\(\s*h\s*-\s*l\s*\)\s*/', r'lo + (hi - lo) /'),
        (r'(?<!\w)h(?!\w)', r'hi'), (r'(?<!\w)l(?!\w)', r'lo'),
    ],
    '4-Strings/4.3.4_Palindrome_Searching_(Manacher).cpp': [
        (r'\bbool is_palindrome\(int l,\s*int r\b', r'bool is_palindrome(int lo, int hi'),
        (r'\bis_palindrome\(([^,]+),\s*l\b', r'is_palindrome(\1, lo'),
        (r'\bis_palindrome\(l\b', r'is_palindrome(lo'),
    ],
    '4-Strings/4.2.2_Rolling_Hash.cpp': [
        (r'\buint64 get\(int l,\s*int r\b', r'uint64 get(int lo, int hi'),
        (r'\bpref\[r\]\b', r'pref[hi]'), (r'\bpref\[l\]\b', r'pref[lo]'),
        (r'\bpow_base\[r\s*-\s*l\]', r'pow_base[hi - lo]'),
    ],
}

# ── 6. ans/ret → better names (targeted) ─────────────────────────────────────
ANS_FIXES: dict[str, list[tuple[str, str]]] = {
    '3-Optimization/3.2.3_Simulated_Annealing.cpp': [
        (r'\bdouble ans_x\b', r'double best_x'),
        (r'\bdouble ans_y\b', r'double best_y'), # may be on same line
        (r'(?<=[,\s])ans_x\b', r'best_x'), (r'(?<=[,\s])ans_y\b', r'best_y'),
        (r'(?<=return\s)ans\b', r'best'), (r'\bdouble ans\s*=', r'double best ='),
        (r'\bif\s*\(cur\s*<\s*ans\b', r'if (cur < best'),
        (r'\bans\s*=\s*cur\b', r'best = cur'),
        (r'\breturn ans\b', r'return best'),
    ],
    '5-Graphs/5.7.1_Maximum_Clique_(Bron-Kerbosch).cpp': [
        (r'\bint ans\s*=\s*0\b', r'int best = 0'),
        (r'\bans\s*=\s*std::max\(ans\b', r'best = std::max(best'),
        (r'\breturn ans\b', r'return best'),
        (r'\bint ans\s*=', r'int best ='),
    ],
    '7-Geometry/7.3.2_Point-in-Polygon_(Ray_Casting).cpp': [
        (r'\bbool ans\s*=\s*0\b', r'bool res = false'),
        (r'\bans\s*=\s*!ans\b', r'res = !res'),
        (r'\breturn ans\b', r'return res'),
    ],
    '2-Data-Structures/2.3.8_Hash_Map.cpp': [
        (r'\bV \*ret\b', r'V *ptr'),
        (r'\breturn \*ret\b', r'return *ptr'),
        (r'\bif\s*\(ret\s*!=', r'if (ptr !='),
        (r'\bret\s*!=\s*NULL\b', r'ptr != nullptr'),
        (r'\bret\s*!=\s*nullptr\b', r'ptr != nullptr'),
    ],
    '2-Data-Structures/2.3.9_Skip_List.cpp': [
        (r'\bV \*ret\b', r'V *ptr'),
        (r'\breturn \*ret\b', r'return *ptr'),
        (r'\bif\s*\(ret\s*!=', r'if (ptr !='),
    ],
    '4-Strings/4.4.1_Recursive_Descent_Parsing_(Simple).cpp': [
        (r'\bint ret\b', r'int res'),
        (r'\bret\s*=\s*eval\b', r'res = eval'),
        (r'\bret\s*=\s*10\s*\*\s*ret\b', r'res = 10 * res'),
        (r'\breturn sign\s*\*\s*ret\b', r'return sign * res'),
    ],
    '2-Data-Structures/2.3.6_Size_Balanced_Tree.cpp': [
        (r'\bbool result\b', r'bool found'),
        (r'\bresult\s*=\s*true\b', r'found = true'),
        (r'\breturn result\b', r'return found'),
    ],
}

# ── 7. INF constant style (graph files) ──────────────────────────────────────
INF_FILES = [
    '5-Graphs/5.2.2_Shortest_Path_(0-1_BFS).cpp',
    '5-Graphs/5.2.3_Shortest_Path_(Dijkstra).cpp',
    '5-Graphs/5.2.4_Shortest_Path_(Bellman-Ford).cpp',
    '5-Graphs/5.2.6_Shortest_Path_(Floyd-Warshall).cpp',
    '5-Graphs/5.3.2_Strongly_Connected_Components_(Tarjan).cpp',
    '5-Graphs/5.5.1_Maximum_Flow_(Ford-Fulkerson).cpp',
    '5-Graphs/5.5.2_Maximum_Flow_(Edmonds-Karp).cpp',
    '5-Graphs/5.5.3_Maximum_Flow_(Dinic).cpp',
    '5-Graphs/5.5.5_Min-Cost_Max-Flow.cpp',
]

# ── 8. Graph node label fixes ─────────────────────────────────────────────────
GRAPH_LABEL_FIXES: dict[str, list[tuple[str, str]]] = {
    '5-Graphs/5.3.4_Bridge_Finding_(Online).cpp': [
        (r'\bvoid merge_path\(int a,\s*int b\b', r'void merge_path(int u, int v'),
        (r'\bvoid add_edge\(int a,\s*int b\b', r'void add_edge(int u, int v'),
        (r'\bmerge_path\(a,', r'merge_path(u,'), (r'\bmerge_path\([^,]+,\s*b\b', None),
    ],
    '5-Graphs/5.4.2_Minimum_Spanning_Tree_(Kruskal).cpp': [
        (r'\bfind_root\(int x\b', r'find_root(int u'),
        (r'\bfind_root\(x\b', r'find_root(u'),
        (r'(?<=\s)x\s*!=\s*find_root', r'u != find_root'),
    ],
}

# ── 9. Template param renames (targeted) ─────────────────────────────────────
TEMPLATE_FIXES: dict[str, list[tuple[str, str]]] = {
    '1-Elementary-Algorithms/1.5.1_Cycle_Detection_(Floyd).cpp': [],  # keep IntFunction per user
    '1-Elementary-Algorithms/1.5.2_Cycle_Detection_(Brent).cpp': [],  # same
    '3-Optimization/3.1.1_Binary_Search.cpp': [],     # keep Int, IntPredicate
    '3-Optimization/3.1.2_Exponential_Search.cpp': [],  # keep Int, IntPredicate
    '3-Optimization/3.2.1_Ternary_Search.cpp': [],    # keep UnimodalFunction
    '3-Optimization/3.2.2_Hill_Climbing.cpp': [],     # keep ContinuousFunction
    '3-Optimization/3.2.3_Simulated_Annealing.cpp': [],  # keep ContinuousFunction
    '6-Mathematics/6.5.1_Matrix_Utilities.cpp': [],   # keep Matrix
    '6-Mathematics/6.5.2_Row_Reduction.cpp': [],      # keep Matrix
    '6-Mathematics/6.5.3_Determinant_and_Inverse.cpp': [],  # keep SquareMatrix
    '6-Mathematics/6.5.4_LU_Decomposition.cpp': [],   # keep SquareMatrix
    '6-Mathematics/6.6.1_Root_Finding_(Bracketing).cpp': [],   # keep ContinuousFunction
    '6-Mathematics/6.6.2_Root_Finding_(Iteration).cpp': [],    # keep ContinuousFunction
    '6-Mathematics/6.6.6_Integration_(Simpson).cpp': [],       # keep ContinuousFunction
    '3-Optimization/3.4.1_Linear_Programming_(Simplex).cpp': [],  # keep Matrix
    '4-Strings/4.2.1_Hash_Functions.cpp': [
        (r'\bclass Key\b', r'class K'),
        (r'(?<=<)Key(?=>)', r'K'),
        (r'\bKey\b(?!\s*=)', r'K'),
    ],
    '1-Elementary-Algorithms/1.1.1_Sorting_Algorithms.cpp': [
        (r'\bUnsignedIt\b', r'It'),
    ],
}

# ── 10. using namespace std in 6.1 example section ───────────────────────────
def fix_math_utilities_example(src: str) -> str:
    marker = '/*** Example Usage'
    idx = src.find(marker)
    if idx == -1:
        return src
    after = src[idx:]
    if 'using namespace std' not in after:
        # Insert after the /*** Example Usage ***/ line
        end_of_line = after.find('\n')
        insert_pos = idx + end_of_line + 1
        return src[:insert_pos] + '\nusing namespace std;\n' + src[insert_pos:]
    return src


# ── Row_Reduction size var fix ────────────────────────────────────────────────
def fix_row_reduction(src: str) -> str:
    # int r = a.size(), c = a[0].size() → int rows = ..., cols = ...
    src = re.sub(r'\bint r\s*=\s*(\(int\)|static_cast<int>\()?a\.size\(\)',
                 r'int rows = \1a.size()', src)
    src = re.sub(r'\bint c\s*=\s*(\(int\)|static_cast<int>\()?a\[0\]\.size\(\)',
                 r'int cols = \1a[0].size()', src)
    src = re.sub(r'\bint r\s*=\s*a\.size\(\),\s*c\s*=\s*a\[0\]\.size\(\)',
                 r'int rows = a.size(), cols = a[0].size()', src)
    # Replace subsequent uses of r/c as row/col dimensions in this file
    # (be conservative - only in the row_reduce/determinant/etc functions)
    return src


# ── Main ──────────────────────────────────────────────────────────────────────

def all_cpp_files():
    return sorted(
        p for d in ROOT.iterdir()
        if re.match(r'\d+-', d.name) and d.is_dir()
        for p in sorted(d.glob('*.cpp'))
    )


def build_class_file_map(files):
    """Scan files to find which defines each class."""
    mapping: dict[str, list[Path]] = {}
    for old, new in CLASS_RENAMES.items():
        pat = re.compile(r'\b(?:class|struct)\s+' + re.escape(old) + r'\b')
        for f in files:
            if pat.search(f.read_text()):
                mapping.setdefault(old, []).append(f)
    return mapping


def process(files):
    class_file_map = build_class_file_map(files)

    changed = 0
    for f in files:
        src = original = f.read_text()
        rel = str(f.relative_to(ROOT))

        # 1. Class renames (per-file, code sections only to protect prose)
        for old, new in CLASS_RENAMES.items():
            if f in class_file_map.get(old, []):
                src = code_only_replace(src, r'\b' + re.escape(old) + r'\b', new)

        # 2. Inner struct renames (global)
        for old, new in STRUCT_RENAMES.items():
            src = word_replace(src, old, new)

        # 3. NULL → nullptr (global)
        src = word_replace(src, 'NULL', 'nullptr')

        # 4. C-style casts → static_cast
        src = convert_casts(src)

        # 5. Range param fixes
        for fix_rel, patterns in RANGE_FIXES.items():
            if rel == fix_rel:
                for pat, repl in patterns:
                    if repl is not None:
                        src = re.sub(pat, repl, src)

        # 6. ans/ret fixes
        for fix_rel, patterns in ANS_FIXES.items():
            if rel == fix_rel:
                for pat, repl in patterns:
                    if repl is not None:
                        src = re.sub(pat, repl, src)

        # 7. INF constant
        if rel in INF_FILES:
            src = re.sub(
                r'\bconst\s+int\s+(?:MAXN\s*=\s*\d+,\s*)?INF\s*=\s*0x3f3f3f3f\b',
                lambda m: m.group().replace('0x3f3f3f3f', 'INT_MAX / 2'),
                src,
            )
            src = re.sub(
                r'\bconst\s+int\s+MCMF_INF\s*=\s*0x3f3f3f3f\b',
                r'const int MCMF_INF = INT_MAX / 2',
                src,
            )
            if 'INT_MAX' in src and '#include <climits>' not in src and '#include <limits>' not in src:
                src = src.replace('#include <algorithm>', '#include <algorithm>\n#include <climits>', 1)

        # 8. Graph label fixes
        for fix_rel, patterns in GRAPH_LABEL_FIXES.items():
            if rel == fix_rel:
                for pat, repl in patterns:
                    if repl is not None:
                        src = re.sub(pat, repl, src)

        # 9. Template param renames
        for fix_rel, patterns in TEMPLATE_FIXES.items():
            if rel == fix_rel:
                for pat, repl in patterns:
                    if repl is not None:
                        src = code_only_replace(src, pat, repl)

        # 10. 6.1 Math Utilities example namespace
        if rel == '6-Mathematics/6.1_Math_Utilities.cpp':
            src = fix_math_utilities_example(src)

        if src != original:
            f.write_text(src)
            print(f'  {rel}')
            changed += 1

    print(f'\n{changed} / {len(files)} files changed.')


if __name__ == '__main__':
    files = all_cpp_files()
    process(files)
