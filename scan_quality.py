#!/usr/bin/env python3

from dataclasses import dataclass
import re
import signal
import subprocess
import sys
from pathlib import Path


LIMIT = 100
MIN_INTERIOR_DOCSTRING_LINE = 72
ROOT = Path(__file__).resolve().parent
REPEATED_LITERAL_DEFAULT_THRESHOLD = 3
signal.signal(signal.SIGPIPE, signal.SIG_DFL)

KNOWN_LONG_DOCSTRING_LINES = {
    Path("6-Mathematics/6.8.2_Nim_Product.cpp"): {8},
}

EXAMPLE_OUTPUT_WITHOUT_DIRECT_PRINT = {
    Path("8-Miscellany/8.3_Debugging.cpp"),
}

EXAMPLE_STD_QUALIFICATION_ALLOWLIST = {
    Path("1-Elementary-Algorithms/1.1.3_Array_Rotation.cpp"): {"rotate"},
    Path("6-Mathematics/6.2.3_Enumerating_Permutations.cpp"): {"next_permutation"},
}

BRACED_PUSH_ALLOWLIST = {
    Path("1-Elementary-Algorithms/1.7.6_Binary_Trie.cpp"): {"child"},
    Path("2-Data-Structures/2.6.3_2D_Range_Tree.cpp"): {"points"},
    Path("3-Strings/3.7.2_Shunting_Yard_and_Postfix_Evaluation.cpp"): {"op_stack"},
    Path("4-Graphs/4.2.5_Articulation_Points,_Biconnected_Components,_Block-Cut_Forest.cpp"):
        {"bcc"},
    Path("4-Graphs/4.2.10_Offline_Dynamic_Connectivity.cpp"): {"ops"},
}

TYPE_PRESERVING_CMATH_FILES = {
    Path("5-Optimization/5.4.3_Polynomial_Root_Finding_(Differentiation).cpp"),
    Path("5-Optimization/5.4.4_Polynomial_Root_Finding_(Laguerre).cpp"),
    Path("5-Optimization/5.4.5_Polynomial_Root_Finding_(Ehrlich-Aberth).cpp"),
    Path("6-Mathematics/6.1.1_Floating-Point_Comparison.cpp"),
    Path("6-Mathematics/6.5.2_Row_Reduction.cpp"),
    Path("6-Mathematics/6.5.3_Determinant_and_Inverse.cpp"),
    Path("6-Mathematics/6.5.4_LU_Decomposition.cpp"),
    Path("7-Geometry/7.1.1_Point.cpp"),
    Path("7-Geometry/7.5.1_3D_Point.cpp"),
}

EXAMPLE_HELPER_RE = re.compile(
    r"(?m)^(?:template<[^>\n]+>\n)?"
    r"(?:[A-Za-z_][\w:<>,&* ]+\s+)+"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*\{"
)
C_RAND_RE = re.compile(
    r"((?<![A-Za-z0-9_])::rand\s*\(|(?<!::)\brand\s*\(\s*\)\s*%|\bsrand\s*\()"
)
RANDOM_DEVICE_RE = re.compile(r"\b(?:std::)?random_device\b")
UNQUALIFIED_CMATH_RE = re.compile(
    r"(?<![A-Za-z0-9_:])(?:fabs|sqrt|hypot|atan2|acos|asin|cos|sin|floor|ceil|pow|exp|log|"
    r"modf|fmod)l?\s*\("
)
UNQUALIFIED_STD_CMATH_RE = re.compile(
    r"(?<![A-Za-z0-9_:])"
    r"(sqrt|cbrt|hypot|atan2|acos|asin|atan|cos|sin|tan|floor|ceil|round|llround|pow|"
    r"exp|log|log2|modf|fmod)\s*\("
)
UNQUALIFIED_CMATH_NAME_ALLOWLIST = {
    Path("6-Mathematics/6.1.2_Rounding.cpp"): {"round"},
    Path("6-Mathematics/6.3.3_Modular_Integer_and_Combinatorics.cpp"): {"pow"},
    Path("6-Mathematics/6.4.2_Big_Integer.cpp"): {"pow", "sqrt"},
    Path("6-Mathematics/6.4.3_Big_Decimal.cpp"): {"pow", "sqrt", "floor", "ceil"},
    Path("6-Mathematics/6.4.4_Rational_Numbers.cpp"): {"floor", "ceil"},
    Path("6-Mathematics/6.6.3_Polynomial_Operations.cpp"): {"log", "exp", "sqrt"},
}
UNQUALIFIED_GENERIC_FABS_RE = re.compile(r"(?<![A-Za-z0-9_:])fabs\(C\(")
CAST_EPS_RE = re.compile(r"\bstatic_cast<C>\(EPS\)")
FUNCTION_LITERAL_CAST_RE = re.compile(
    r"\b(?:T|U|W|C|Double|Mask|mask_t|dbl|cdbl|fp_t|u?int(?:8|16|32|64)_t|"
    r"int128_t|uint128_t)\s*\(\s*-?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?"
    r"[uUlLfF]*\s*\)"
)
STATIC_LITERAL_CAST_RE = re.compile(
    r"\bstatic_cast<(?:T|U|W|C|Mask|mask_t|Mint|dbl|cdbl|fp_t|"
    r"u?int(?:8|16|32|64)_t|int128_t|uint128_t)>\([01]\)"
)
EMPTY_DEPENDENT_CONSTRUCTION_RE = re.compile(
    r"\b(?:T|U|W|C|K|V|Mask|mask_t|Mint|dbl|cdbl|fp_t|Compare|Hash|KeyEqual|"
    r"Hasher|AbsHash|AbsEqual)\(\)|\b(?:std::hash|GenericHasher)<[^>]+>\(\)"
)
AUTO_DEREF_COPY_RE = re.compile(r"\bauto\s+[A-Za-z_][A-Za-z0-9_]*\s*=\s*\*")
COPY_LIST_CONTAINER_RE = re.compile(
    r"^\s*(?:std::)?(?:vector|array|deque|list|forward_list|set|multiset|map|multimap|"
    r"unordered_set|unordered_multiset|unordered_map|unordered_multimap|string)"
    r"(?:\s*<.*>)?\s+[A-Za-z_][A-Za-z0-9_]*\s*=\s*\{"
)
LOWERCASE_LITERAL_SUFFIX_RE = re.compile(
    r"(?<![A-Za-z0-9_.])(?:"
    r"(?:0[xX][0-9A-Fa-f]+|0[bB][01]+)[uUlL]*[ul][uUlL]*|"
    r"(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?"
    r"[uUlLfF]*[ulf][uUlLfF]*)"
    r"(?![A-Za-z0-9_])"
)
EXAMPLE_CONTAINER_DECL_RE = re.compile(
    r"^(\s*)((?:std::)?(?:vector|array|deque|list|forward_list|set|multiset|map|multimap|"
    r"unordered_set|unordered_multiset|unordered_map|unordered_multimap|string)\s*<.*>|"
    r"(?:std::)?string)\s+([A-Za-z_][A-Za-z0-9_]*)\s*(\{.*\}|=\s*.*);\s*$"
)
VECTOR_TYPE_RE = re.compile(r"(?:std::)?vector\s*<")
DEFAULT_VECTOR_FILL_RE = re.compile(
    r"(?:0(?:\.0+|[uUlLfF]+)?|false|nullptr|'\\0'|[A-Za-z_][A-Za-z0-9_]*\{\}|\{\})"
)
POST_API_CONTRACT_RE = re.compile(
    r"\b(?:must|requires?|may not|cannot|nonnegative|nonempty|positive|distinct|sorted)\b",
    re.IGNORECASE,
)
SHARED_POST_API_RE = re.compile(
    r"\b(?:functions|operations|helpers|overloads|implementations?|value type|element type|"
    r"callback|objects|replace(?:d)?|adapt)\b",
    re.IGNORECASE,
)
ORDINAL_API_NAME_RE = re.compile(
    r"(?:^|_)kth(?:_|$)|(?:^|_)rank(?:_|$)|(?:^|_)by_rank(?:_|$)|"
    r"(?:^|_)rank_by(?:_|$)|^find_by_order$|^(?:inverse_)?gray_code$"
)
ORDINAL_BASE_RE = re.compile(
    r"\b[01]-based\b|`k`?\s*={1,2}\s*[01]|\brank\b.{0,120}\$?\[0,",
    re.IGNORECASE,
)
OPTIONAL_RECONSTRUCTION_RE = re.compile(r"^\s*// Optional: reconstruct one .+\.$")
LEGACY_BOUNDARY_FLAG_RE = re.compile(r"\b(?:edge_is_inside|touch_is_intersect)\b")
CONST_VALUE_PARAMETER_RE = re.compile(
    r"(?:\(|,)\s*const\s+[A-Za-z_][A-Za-z0-9_:<>]*\s+"
    r"[A-Za-z_][A-Za-z0-9_]*\s*(?==|,|\))"
)
FIXED_ENGINE_SEED_RE = re.compile(
    r"(?:std::)?(?:mt19937|mt19937_64)\s+[A-Za-z_][A-Za-z0-9_]*"
    r"\s*\(\s*([0-9]+)[uUlL]*\s*\)"
)
FIXED_RANDOM_WRAPPER_SEED_RE = re.compile(
    r"(?:\bRNG|\bZobristHash<[^;()]+>)\s+[A-Za-z_][A-Za-z0-9_]*"
    r"\s*\(\s*([0-9]+)[uUlL]*\s*\)"
)
FIXED_XORSHIFT_STATE_RE = re.compile(
    r"\bstatic\s+(?:std::)?uint32_t\s+[A-Za-z_][A-Za-z0-9_]*"
    r"\s*=\s*[0-9]+[uUlL]*\s*;"
)
CONTROL_BLOCK_RE = re.compile(r"^(?:if|for|while|switch|catch)\b")
UNWRAPPED_INTERVAL_RE = re.compile(
    r"(?:\b(?:range|window)\s+|\bvalues?\s+in\s+|^\s*in\s+)"
    r"\[(?=(?:`[A-Za-z_][A-Za-z0-9_]*`|`?[0-9]+`?))"
)


def matching_delimiter(text, start, opening, closing):
    depth = 0
    for i in range(start, len(text)):
        if text[i] == opening:
            depth += 1
        elif text[i] == closing and not (closing == ">" and i > 0 and text[i - 1] == "-"):
            depth -= 1
            if depth == 0:
                return i
    return -1


def vector_has_redundant_fill(code):
    for match in VECTOR_TYPE_RE.finditer(code):
        type_end = matching_delimiter(code, match.end() - 1, "<", ">")
        if type_end == -1:
            continue
        pos = type_end + 1
        while pos < len(code) and code[pos].isspace():
            pos += 1
        name = re.match(r"[A-Za-z_][A-Za-z0-9_]*", code[pos:])
        if name:
            pos += name.end()
            while pos < len(code) and code[pos].isspace():
                pos += 1
        if pos >= len(code) or code[pos] != "(":
            continue
        call_end = matching_delimiter(code, pos, "(", ")")
        if call_end == -1:
            continue
        args = code[pos + 1 : call_end]
        depth = 0
        commas = []
        for i, char in enumerate(args):
            if char in "([{<":
                depth += 1
            elif char in ")]}>":
                depth -= 1
            elif char == "," and depth == 0:
                commas.append(i)
        if len(commas) == 1 and DEFAULT_VECTOR_FILL_RE.fullmatch(args[commas[0] + 1 :].strip()):
            return True
    return False

STYLE_PATTERNS = [
    (
        re.compile(r"static_cast<fp_t>\s*\("),
        "Use compact function-style conversion for the local arithmetic alias, e.g. fp_t(x).",
    ),
    (
        re.compile(r"\(fp_t\)\s*[A-Za-z_(]"),
        "Replace the C-style arithmetic cast with function-style conversion, e.g. fp_t(x).",
    ),
    (
        re.compile(
            r"if constexpr \(std::is_floating_point_v<C>\) return "
            r"fabs\(C\(a\) - C\(b\)\) <= EPS;"
        ),
        "Check exact equality before subtracting so same-signed infinities compare equal.",
    ),
    (
        re.compile(
            r"from [`$]?[A-Za-z_][A-Za-z0-9_]*[`$]? "
            r"to [`$]?[A-Za-z_][A-Za-z0-9_]*[`$]?, inclusive"
        ),
        "Prefer closed interval notation such as $[`lo`, `hi`]$.",
    ),
    (
        re.compile(
            r"from [`$]?[0-9]+[`$]? "
            r"to [`$]?[A-Za-z_][A-Za-z0-9_]*[`$]?, inclusive"
        ),
        "Prefer interval notation for numeric ranges.",
    ),
    (
        re.compile(
            r"from [`$]?(0|\$0\$)[`$]? "
            r"to [`$]?[A-Za-z_][A-Za-z0-9_]*[`$]? - 1"
        ),
        "Prefer half-open interval notation such as $[0, n)$.",
    ),
    (
        re.compile(r"(numbered|indices|integers) from [`$]?(0|\$0\$)"),
        "Prefer interval notation for numbered domains.",
    ),
    (
        re.compile(r"\bis between \$0\$ and [`$]?[A-Za-z_][A-Za-z0-9_]*[`$]?"),
        "Prefer interval notation for index bounds.",
    ),
    (
        re.compile(r"\bhalf-open range from [`$]?[A-Za-z_][A-Za-z0-9_]*(?:\[[^\]]+\])?[`$]? to"),
        "Prefer half-open interval notation such as $[lo, hi)$.",
    ),
    (
        re.compile(
            r"^-\s+`[^`=]+`.*\b(defaulting to|By default|default:|the default)\b"
        ),
        "Include semantically relevant default arguments in the documented signature.",
    ),
    (
        re.compile(
            r"`(?![^`]*Compare = std::less(?:<[^>]*>)?[^`]*`)[^`]*\bcomp = Compare\(\)"
        ),
        "Document the concrete default comparator, e.g. comp = std::less<>.",
    ),
    (
        re.compile(r"inclusive range \$\[[^\]]+\]\$"),
        "Closed interval notation already implies inclusivity.",
    ),
    (
        re.compile(r"\$\[[^\]]+\]\$, inclusive"),
        "Closed interval notation already implies inclusivity.",
    ),
    (
        re.compile(r"; The empty range"),
        "Do not capitalize after a semicolon.",
    ),
    (
        re.compile(r"\bO\(sqrt [A-Za-z_][A-Za-z0-9_]*\)"),
        "Use function-style parentheses for square roots, e.g. O(sqrt(n)).",
    ),
    (
        re.compile(r"\blog\(N\)"),
        "Use O(log N) for a lone variable; keep parentheses for products or nested functions.",
    ),
    (
        re.compile(r"\bn line inserted\b"),
        "Use the plural form: lines inserted.",
    ),
    (
        re.compile(r"with values set to \$0\$"),
        "Use initialized wording for constructor bullets.",
    ),
    (
        re.compile(r"\b(Link-Cut Tree|Link-Cut Trees)\b"),
        "Use lowercase prose form `link/cut tree(s)` outside titles and code identifiers.",
    ),
    (
        re.compile(r"\bprime ([0-9]{4,})\b"),
        "Wrap large mathematical literals in math mode.",
    ),
    (
        re.compile(r"64-bit integer coordinate type (?:stays exact|is safe) up to"),
        "Use the standard `Overflow warning:` prose for coordinate overflow limits.",
    ),
    (
        re.compile(r"`\(\*[A-Za-z_][A-Za-z0-9_]*\)"),
        "Describe output pointers from the caller's perspective, without dereferencing them.",
    ),
    (
        re.compile(r"^-\s+`(?:add|extend)\([^`]*\)`\s+appends?\b"),
        "Use append() for a public online sequence operation that appends one element.",
    ),
]


@dataclass
class Issue:
    path: Path
    line: int
    kind: str
    message: str
    text: str

    def format(self):
        rel = self.path.relative_to(ROOT)
        return f"{rel}:{self.line}: {self.kind}: {self.message}\n  {self.text.rstrip()}"


def cpp_files():
    for path in sorted(ROOT.rglob("*.cpp")):
        if "Book" not in path.relative_to(ROOT).parts:
            yield path


def docstring_ranges(lines):
    in_block = False
    start = None
    for i, line in enumerate(lines):
        if not in_block and "/*" in line:
            in_block = True
            start = i
        if in_block and "*/" in line:
            yield start, i
            in_block = False
            start = None


def first_wrapped_token(text):
    text = text.strip()
    if not text:
        return ""

    def with_trailing_punctuation(end):
        while end + 1 < len(text) and text[end + 1] in ".,;:)":
            end += 1
        return text[: end + 1]

    if text[0] == "`":
        end = text.find("`", 1)
        if end != -1:
            return with_trailing_punctuation(end)

    if text[0] == "$":
        i = 1
        while i < len(text):
            if text[i] == "$" and text[i - 1] != "\\":
                return with_trailing_punctuation(i)
            i += 1

    if text.startswith("O("):
        depth = 0
        for i, char in enumerate(text):
            if char == "(":
                depth += 1
            elif char == ")":
                depth -= 1
                if depth == 0:
                    return with_trailing_punctuation(i)

    return text.split()[0]


def is_docstring_continuation(line, next_line):
    stripped = line.strip()
    next_stripped = next_line.strip()
    if not stripped or not next_stripped:
        return False
    if line.startswith("  ") and not next_line.startswith("  "):
        return False
    if next_line.startswith("  ") and not (line.startswith("  ") or stripped.startswith("- ")):
        return False
    if stripped in {"/*", "*/"} or next_stripped in {"/*", "*/"}:
        return False
    if re.match(
        r"^(?:Time Complexity(?: \([^)]+\))?|Space Complexity|Stable\?):",
        stripped,
    ):
        return False
    if re.match(
        r"^(?:Time Complexity(?: \([^)]+\))?|Space Complexity|Stable\?):",
        next_stripped,
    ):
        return False
    if stripped.endswith(":"):
        return False
    if next_stripped.endswith(":") and re.fullmatch(r"[A-Za-z ]+:", next_stripped):
        return False
    if next_stripped.startswith("- "):
        return False
    return True


def is_reflowable_prose_line(line):
    stripped = line.strip()
    if not stripped or stripped in {"/*", "*/"}:
        return False
    if line.startswith("  "):
        return False
    if stripped.startswith(("- ", "$", "`")):
        return False
    if stripped.endswith(":") and re.fullmatch(r"[A-Za-z ()]+:", stripped):
        return False
    if re.match(r"^[A-Z][A-Za-z ]+:", stripped):
        return False
    if re.match(r"^[A-Za-z_][\w:() -]*: ", stripped):
        return False
    return True


def scan_atomic_inline_wrapping(paths):
    issues = []
    big_o_open_re = re.compile(r"\bO\([^)]*$")
    for path in paths:
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            for i in range(start, end):
                line = lines[i]
                stripped = line.strip()
                if not stripped or stripped in {"/*", "*/"}:
                    continue
                if stripped.count("`") % 2 == 1:
                    issues.append(
                        Issue(
                            path,
                            i + 1,
                            "split-inline-notation",
                            "Do not split backtick expressions across docstring lines.",
                            line,
                        )
                    )
                if len(re.findall(r"(?<!\\)\$", line)) % 2 == 1:
                    issues.append(
                        Issue(
                            path,
                            i + 1,
                            "split-inline-notation",
                            "Do not split math formulas across docstring lines.",
                            line,
                        )
                    )
                if big_o_open_re.search(line):
                    issues.append(
                        Issue(
                            path,
                            i + 1,
                            "split-inline-notation",
                            "Do not split Big-O expressions across docstring lines.",
                            line,
                        )
                    )
    return issues


def scan_docstrings(paths):
    issues = []
    for path in paths:
        rel = path.relative_to(ROOT)
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            if lines[start].startswith("/***"):
                continue
            for i in range(start, end):
                line = lines[i]
                next_line = lines[i + 1]
                token = first_wrapped_token(next_line)
                if (
                    is_docstring_continuation(line, next_line)
                    and token
                    and len(line.rstrip() + " " + token) <= LIMIT
                ):
                    issues.append(
                        Issue(
                            path,
                            i + 1,
                            "underfull-docstring",
                            f"next token fits on this line ({len(line.rstrip())} -> "
                            f"{len(line.rstrip() + ' ' + token)})",
                            line,
                        )
                    )
            for i in range(start + 1, end):
                line = lines[i].rstrip()
                if (
                    len(line) < MIN_INTERIOR_DOCSTRING_LINE
                    and is_reflowable_prose_line(lines[i - 1])
                    and is_reflowable_prose_line(lines[i])
                    and is_reflowable_prose_line(lines[i + 1])
                ):
                    issues.append(
                        Issue(
                            path,
                            i + 1,
                            "underfull-docstring",
                            f"interior paragraph line has only {len(line)} columns",
                            lines[i],
                        )
                    )
            for i in range(start, end + 1):
                if i + 1 in KNOWN_LONG_DOCSTRING_LINES.get(rel, set()):
                    continue
                line = lines[i].rstrip()
                if len(line) > LIMIT:
                    issues.append(
                        Issue(
                            path,
                            i + 1,
                            "overfull-docstring",
                            f"line has {len(line)} columns",
                            line,
                        )
                    )
    return issues


def scan_docstring_wording(paths):
    issues = []
    bare_constant_re = re.compile(r"\b(?:EPS|INF|MOD|PI|MASK_BITS)\b")
    for path in paths:
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            if lines[start].startswith("/***"):
                continue
            for i in range(start, end + 1):
                line = lines[i]
                prose = re.sub(r"`[^`]*`|\$[^$]*\$", "", line)
                if re.search(r"\bstandard STL\b", prose, re.IGNORECASE):
                    issues.append(
                        Issue(
                            path,
                            i + 1,
                            "docstring-wording",
                            'Write "standard library" instead of the redundant "standard STL".',
                            line,
                        )
                    )
                if bare_constant_re.search(prose):
                    issues.append(
                        Issue(
                            path,
                            i + 1,
                            "docstring-code-style",
                            "Put named code constants such as `EPS` or `MOD` in backticks.",
                            line,
                        )
                    )
    return issues


def scan_docstring_structure(paths):
    """Enforce the stable layout of leading section docstrings."""
    issues = []
    api_bullet_re = re.compile(r"^- `")
    one_line_complexity_re = re.compile(r"^(?:Time|Space) Complexity:\s+\S")
    for path in paths:
        lines = path.read_text().splitlines()
        ranges = list(docstring_ranges(lines))
        for block_start, block_end in ranges:
            if lines[block_start].startswith("/***"):
                continue
            first = next(
                (i for i in range(block_start + 1, block_end) if lines[i].strip()), None
            )
            if first is not None and api_bullet_re.match(lines[first]):
                issues.append(
                    Issue(
                        path,
                        first + 1,
                        "docstring-structure",
                        "Begin each documentation block with an orienting prose sentence.",
                        lines[first],
                    )
                )
        if not ranges or ranges[0][0] != 0 or lines[0].startswith("/***"):
            continue
        start, end = ranges[0]
        body = lines[start + 1 : end]
        api_indices = [i for i, line in enumerate(body, start + 1) if api_bullet_re.match(line)]

        for i, line in enumerate(body, start + 1):
            stripped = line.strip()
            if one_line_complexity_re.match(stripped):
                issues.append(
                    Issue(
                        path,
                        i + 1,
                        "docstring-structure",
                        "Put the complexity heading on its own line and the bound in a bullet.",
                        line,
                    )
                )
            if re.match(r"^Preconditions?:", stripped):
                issues.append(
                    Issue(
                        path,
                        i + 1,
                        "detached-api-contract",
                        "Put preconditions in the applicable API bullet or a shared paragraph.",
                        line,
                    )
                )

        if not api_indices:
            continue
        first_api = min(api_indices)
        for i in range(start + 1, first_api):
            if lines[i].strip().startswith("Overflow warning:"):
                issues.append(
                    Issue(
                        path,
                        i + 1,
                        "docstring-structure",
                        "Place labeled warnings after the API list.",
                        lines[i],
                    )
                )

        first_complexity = next(
            (
                i
                for i in range(start + 1, end)
                if lines[i].strip() in {"Time Complexity:", "Space Complexity:"}
            ),
            end,
        )
        intro_end = min(first_api, first_complexity)
        intro = " ".join(lines[i].strip() for i in range(start + 1, intro_end))
        intro_words = re.findall(r"[A-Za-z0-9_]+", re.sub(r"[`$]", "", intro))
        if len(api_indices) >= 4 and len(intro_words) < 20:
            issues.append(
                Issue(
                    path,
                    start + 3,
                    "thin-docstring-intro",
                    "Introduce the common idea connecting this section's APIs.",
                    lines[start + 2] if start + 2 < len(lines) else lines[start],
                )
            )

        signatures = [lines[i].split("`", 2)[1] for i in api_indices]
        if (
            path.relative_to(ROOT).parts[0] != "8-Miscellany"
            and all("(" not in signature for signature in signatures)
        ):
            code = "\n".join(lines[end + 1 :])
            if re.search(r"^(?:template<[^\n]+>\n)?(?:class|struct)\s+", code, re.MULTILINE):
                issues.append(
                    Issue(
                        path,
                        api_indices[0] + 1,
                        "docstring-structure",
                        "Document the type's callable API or coherent major-operation groups.",
                        lines[api_indices[0]],
                    )
                )
    return issues


def scan_overflow_warning_labels(paths):
    issues = []
    representability_re = re.compile(r"\b(?:must fit|must be representable)\b", re.IGNORECASE)
    exactness_re = re.compile(r"\bdo(?:es)? not overflow\b", re.IGNORECASE)
    for path in paths:
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            if lines[start].startswith("/***"):
                continue
            has_labeled_warning = any(
                line.strip().startswith("Overflow warning:") for line in lines[start + 1 : end]
            )
            paragraph = []
            paragraph_line = 0

            def check_paragraph():
                if not paragraph:
                    return
                text = " ".join(line.strip() for line in paragraph)
                if (
                    not text.startswith("- ")
                    and not text.startswith("Overflow warning:")
                    and (
                        representability_re.search(text)
                        or (exactness_re.search(text) and not has_labeled_warning)
                    )
                ):
                    issues.append(
                        Issue(
                            path,
                            paragraph_line,
                            "overflow-warning-label",
                            "Lead standalone arithmetic limits with `Overflow warning:`.",
                            lines[paragraph_line - 1],
                        )
                    )

            for i in range(start + 1, end):
                stripped = lines[i].strip()
                if not stripped:
                    check_paragraph()
                    paragraph = []
                    continue
                if not paragraph:
                    paragraph_line = i + 1
                paragraph.append(lines[i])
            check_paragraph()
    return issues


def scan_post_api_contracts(paths):
    """Flag narrow contracts detached from the one API bullet they describe."""
    issues = []
    bullet_re = re.compile(r"^- `([^`]+)`")
    for path in paths:
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            if lines[start].startswith("/***"):
                continue
            bullets = []
            for i in range(start + 1, end):
                match = bullet_re.match(lines[i].strip())
                if not match:
                    continue
                signature = match.group(1)
                name_match = re.search(
                    r"\b([A-Za-z_][A-Za-z0-9_]*)(?:<[^`]*>)?\s*\(", signature
                )
                if not name_match:
                    continue
                params = []
                for param in split_parameters(call_parameters(signature)):
                    default = top_level_default_expression(param)
                    if default:
                        param = param[: len(param) - len(default) - 1].rstrip()
                    param_match = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*$", param)
                    if param_match and len(param_match.group(1)) >= 4:
                        params.append(param_match.group(1))
                bullets.append((i, name_match.group(1), params))
            if not bullets:
                continue

            param_owners = {}
            for _, name, params in bullets:
                for param in params:
                    param_owners.setdefault(param, set()).add(name)

            paragraph_start = bullets[-1][0] + 1
            while paragraph_start < end and lines[paragraph_start].strip():
                paragraph_start += 1
            while paragraph_start < end and not lines[paragraph_start].strip():
                paragraph_start += 1

            paragraph = []
            line_no = 0

            def check_paragraph():
                if not paragraph:
                    return
                text = " ".join(line.strip() for line in paragraph)
                if (
                    text.startswith(("Overflow warning:", "Time Complexity:", "Space Complexity:"))
                    or not POST_API_CONTRACT_RE.search(text)
                    or SHARED_POST_API_RE.search(text)
                    or len(re.findall(r"[.!?](?:\s|$)", text)) > 2
                ):
                    return
                owners = set()
                for _, name, _ in bullets:
                    if re.search(rf"\b{re.escape(name)}(?:\(\))?\b", text):
                        owners.add(name)
                for param, names in param_owners.items():
                    if len(names) == 1 and re.search(rf"\b{re.escape(param)}\b", text):
                        owners.update(names)
                if len(owners) == 1:
                    name = next(iter(owners))
                    issues.append(
                        Issue(
                            path,
                            line_no,
                            "detached-api-contract",
                            f"Move this operation-specific contract into the `{name}()` API bullet.",
                            lines[line_no - 1],
                        )
                    )
                elif owners:
                    issues.append(
                        Issue(
                            path,
                            line_no,
                            "detached-api-contract",
                            "Distribute these operation-specific contracts among their API bullets.",
                            lines[line_no - 1],
                        )
                    )

            for i in range(paragraph_start, end):
                stripped = lines[i].strip()
                if re.match(r"^(?:Time|Space) Complexity:", stripped):
                    check_paragraph()
                    break
                if stripped:
                    if not paragraph:
                        line_no = i + 1
                    paragraph.append(lines[i])
                else:
                    check_paragraph()
                    paragraph = []
            else:
                check_paragraph()
    return issues


def scan_ordinal_base_docs(paths):
    """Require a base convention where an ordinal API could naturally use either one."""
    issues = []
    non_ordinal_rank_apis = {"matrix_rank", "sparse_rank"}
    bullet_re = re.compile(r"^- `([^`]+)`")
    for path in paths:
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            i = start + 1
            while i < end:
                match = bullet_re.match(lines[i].strip())
                if not match:
                    i += 1
                    continue
                signature = match.group(1)
                text = lines[i].strip()
                j = i + 1
                while j < end and lines[j].startswith("  "):
                    text += " " + lines[j].strip()
                    j += 1
                name_match = re.search(
                    r"\b([A-Za-z_][A-Za-z0-9_]*)(?:<[^`]*>)?\s*\(", signature
                )
                if not name_match:
                    i = j
                    continue
                name = name_match.group(1)
                has_kth_ordinal = re.search(r"(?:`k`|\$k\$|\bk\b)-th\b", text) is not None
                is_ordinal_api = (
                    name not in non_ordinal_rank_apis
                    and (ORDINAL_API_NAME_RE.search(name) is not None or has_kth_ordinal)
                )
                if is_ordinal_api and not ORDINAL_BASE_RE.search(text):
                    issues.append(
                        Issue(
                            path,
                            i + 1,
                            "ordinal-base",
                            "State whether this rank or ordinal is 0-based or 1-based.",
                            lines[i],
                        )
                    )
                i = j
    return issues


def scan_size_declaration_order(paths):
    issues = []
    size_decl_re = re.compile(r"^\s+int n\s*=.*\.size\(\).*;$")
    for path in paths:
        lines = path.read_text().splitlines()
        for i, line in enumerate(lines):
            if not size_decl_re.match(line):
                continue
            validation_before_use = False
            for j in range(i + 1, len(lines)):
                code = lines[j].split("//", 1)[0]
                if re.search(r"\bn\b", code):
                    if validation_before_use:
                        issues.append(
                            Issue(
                                path,
                                i + 1,
                                "precondition-order",
                                "Declare n after validation unless a precondition uses it.",
                                line,
                            )
                        )
                    break
                if "assert(" in code:
                    validation_before_use = True
                if lines[j].startswith("}"):
                    break
    return issues


def scan_time_complexity_scope(paths):
    issues = []
    aggregate_scope_re = re.compile(
        r"\b(?:in total|for construction|to process|to check|for the augmenting paths|"
        r"calls to maximum flow|for any|constructor time|total table growth|"
        r"for sparse elimination|on first use)\b"
    )
    for path in paths:
        lines = path.read_text().splitlines()
        in_time = False
        bullet = ""
        bullet_line = 0

        def check_bullet():
            if (
                bullet.startswith("- O(")
                and " per " not in bullet
                and not aggregate_scope_re.search(bullet)
            ):
                issues.append(
                    Issue(
                        path,
                        bullet_line,
                        "complexity-scope",
                        "State the unit of work, usually with `per call` or `per operation`.",
                        lines[bullet_line - 1],
                    )
                )

        for line_no, line in enumerate(lines, start=1):
            stripped = line.strip()
            if stripped == "Time Complexity:":
                in_time = True
                bullet = ""
                continue
            if not in_time:
                continue
            if stripped == "Space Complexity:" or stripped == "*/":
                if bullet:
                    check_bullet()
                in_time = False
                bullet = ""
            elif stripped.startswith("- "):
                if bullet:
                    check_bullet()
                bullet = stripped
                bullet_line = line_no
            elif bullet and line.startswith("  "):
                bullet += " " + stripped
            elif not stripped and bullet:
                check_bullet()
                bullet = ""
    return issues


def scan_grid_dimension_symbols(paths):
    issues = []
    lowercase_grid_re = re.compile(r"O\([^)]*\br\b[^)]*\bc\b|O\([^)]*\bc\b[^)]*\br\b")
    for path in paths:
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            bullet = ""
            bullet_line = 0

            def check_bullet():
                if not bullet.startswith("- O("):
                    return
                if lowercase_grid_re.search(bullet):
                    issues.append(
                        Issue(
                            path,
                            bullet_line,
                            "grid-dimension-style",
                            "Use R and C when r and c would collide with cell coordinates.",
                            lines[bullet_line - 1],
                        )
                    )

            for i in range(start, end):
                stripped = lines[i].strip()
                if stripped.startswith("- "):
                    if bullet:
                        check_bullet()
                    bullet = stripped
                    bullet_line = i + 1
                elif bullet and lines[i].startswith("  "):
                    bullet += " " + stripped
                elif bullet:
                    check_bullet()
                    bullet = ""
            if bullet:
                check_bullet()
    return issues


def scan_example_markers(paths):
    issues = []
    marker_re = re.compile(r"/\*\*\* Example Usage.*?\*\*\*/", re.DOTALL)
    print_re = re.compile(r"\b(cout|cerr|printf|puts)\b")
    for path in paths:
        rel = path.relative_to(ROOT)
        text = path.read_text()
        match = marker_re.search(text)
        if not match:
            continue
        marker = match.group(0).splitlines()[0]
        body = strip_block_comments_and_strings(text[match.end():])
        has_print = bool(print_re.search(body))
        has_output_marker = "Output" in marker
        line = text[: match.start()].count("\n") + 1
        if has_print and not has_output_marker:
            issues.append(
                Issue(
                    path,
                    line,
                    "example-marker",
                    "example prints but marker does not mention output",
                    marker,
                )
            )
        if has_output_marker and not has_print and rel not in EXAMPLE_OUTPUT_WITHOUT_DIRECT_PRINT:
            issues.append(
                Issue(
                    path,
                    line,
                    "example-marker",
                    "output marker has no direct print call",
                    marker,
                )
            )
    return issues


def scan_inlineable_example_variables(paths):
    issues = []
    for path in paths:
        text = path.read_text()
        marker = text.find("/*** Example Usage")
        if marker == -1:
            continue
        example = text[marker:]
        lines = example.splitlines()
        code_lines = []
        in_block_comment = False
        for line in lines:
            code, in_block_comment = strip_cpp_comments_and_strings(line, in_block_comment)
            code_lines.append(code)

        declarations = {}
        all_code = "\n".join(code_lines)
        for i, code in enumerate(code_lines):
            match = EXAMPLE_CONTAINER_DECL_RE.match(code)
            if match is not None and len(re.findall(rf"\b{re.escape(match.group(3))}\b", all_code)) == 2:
                declarations[match.group(3)] = i

        for i, code in enumerate(code_lines):
            match = EXAMPLE_CONTAINER_DECL_RE.match(code)
            if match is None:
                continue
            indent, type_name, name, initializer = match.groups()
            if name not in declarations:
                continue
            j = i + 1
            while j < len(code_lines) and not code_lines[j].strip():
                j += 1
            if (
                j == len(code_lines)
                or not re.search(rf"\b{re.escape(name)}\b", code_lines[j])
                or not re.match(r"\s*auto\s*\[", code_lines[j])
                or not initializer.startswith("{")
            ):
                continue
            other_fixtures = [
                other
                for other in declarations
                if other != name and re.search(rf"\b{re.escape(other)}\b", code_lines[j])
            ]
            if other_fixtures:
                continue
            expression = (
                type_name + initializer if initializer.startswith("{") else initializer[1:].strip()
            )
            inlined = re.sub(rf"\b{re.escape(name)}\b", expression, code_lines[j], count=1)
            if len(inlined) > LIMIT:
                continue
            issues.append(
                Issue(
                    path,
                    text[:marker].count("\n") + i + 1,
                    "inline-example-variable",
                    "Inline this one-use example container into the following statement.",
                    lines[i],
                )
            )
    return issues


def scan_code_consistency(paths):
    issues = []
    for path in paths:
        rel = path.relative_to(ROOT)
        text = path.read_text()
        lines = text.splitlines()
        in_block_comment = False
        in_example = False
        for line_no, line in enumerate(lines, start=1):
            if line.startswith("/*** Example Usage"):
                in_example = True
            code, in_block_comment = strip_cpp_comments_and_strings(line, in_block_comment)
            if C_RAND_RE.search(line):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Avoid C rand()/srand(); use a C++ random engine and distribution.",
                        line,
                    )
                )
            if UNQUALIFIED_GENERIC_FABS_RE.search(code) or (
                not in_example
                and rel in TYPE_PRESERVING_CMATH_FILES
                and UNQUALIFIED_CMATH_RE.search(code)
            ):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Qualify <cmath> overloads when the function must preserve long double.",
                        line,
                    )
                )
            unqualified_cmath = {
                match.group(1) for match in UNQUALIFIED_STD_CMATH_RE.finditer(code)
            }
            allowed_cmath = UNQUALIFIED_CMATH_NAME_ALLOWLIST.get(rel, set())
            if not in_example and unqualified_cmath - allowed_cmath:
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Qualify standard <cmath> calls in reusable code with std::.",
                        line,
                    )
                )
            if CAST_EPS_RE.search(code):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Use EPS directly; floating-point comparisons convert it implicitly.",
                        line,
                    )
                )
            if FUNCTION_LITERAL_CAST_RE.search(code) or STATIC_LITERAL_CAST_RE.search(code):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Use braces to construct typed numeric literals, e.g. T{10} or U{1}.",
                        line,
                    )
                )
            if EMPTY_DEPENDENT_CONSTRUCTION_RE.search(code):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Use braces to value-initialize dependent types, e.g. T{} or Compare{}.",
                        line,
                    )
                )
            if LOWERCASE_LITERAL_SUFFIX_RE.search(code):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Capitalize all letters in numeric literal suffixes, e.g. 1U or 1LL.",
                        line,
                    )
                )
            if AUTO_DEREF_COPY_RE.search(code):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Materialize iterator value_type; auto may retain a proxy reference.",
                        line,
                    )
                )
            if vector_has_redundant_fill(code):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Omit a vector fill argument that merely repeats value initialization.",
                        line,
                    )
                )
            if (
                "Optional:" in line
                and "reconstruct" in line
                and not OPTIONAL_RECONSTRUCTION_RE.match(line)
            ):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Use `// Optional: reconstruct one ... .` for optional witness extraction.",
                        line,
                    )
                )
            if LEGACY_BOUNDARY_FLAG_RE.search(line):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Use include_boundary consistently for geometry boundary flags.",
                        line,
                    )
                )
            if CONST_VALUE_PARAMETER_RE.search(code):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Omit top-level const from by-value parameters.",
                        line,
                    )
                )
            if "Overflow warning!" in line:
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Use the standard `Overflow warning.` comment punctuation.",
                        line,
                    )
                )

        marker = text.find("/*** Example Usage")
        if marker == -1:
            continue

        implementation = text[:marker]
        implementation_start = []
        in_block_comment = False
        for line in implementation.splitlines():
            code, in_block_comment = strip_cpp_comments_and_strings(line, in_block_comment)
            implementation_start.append(code)
        engine_members = set()
        for code in implementation_start:
            engine_members.update(
                re.findall(
                    r"(?:std::)?(?:mt19937|mt19937_64)\s+([A-Za-z_][A-Za-z0-9_]*)\s*;",
                    code,
                )
            )
        for offset, (line, code) in enumerate(
            zip(implementation.splitlines(), implementation_start), start=1
        ):
            if RANDOM_DEVICE_RE.search(code):
                issues.append(
                    Issue(
                        path,
                        offset,
                        "code-consistency",
                        "Seed reusable randomized code from the full steady-clock tick count.",
                        line,
                    )
                )
            if FIXED_ENGINE_SEED_RE.search(code) or FIXED_XORSHIFT_STATE_RE.search(code) or any(
                re.search(rf"\b{re.escape(name)}\s*\(\s*[0-9]+[uUlL]*\s*\)", code)
                for name in engine_members
            ):
                issues.append(
                    Issue(
                        path,
                        offset,
                        "code-consistency",
                        "Seed reusable randomized implementations at runtime, "
                        "not with a fixed literal.",
                        line,
                    )
                )

        example = text[marker:]
        example_start_line = text[:marker].count("\n") + 1
        imports_std = "using namespace std;" in example
        allowed = EXAMPLE_STD_QUALIFICATION_ALLOWLIST.get(path.relative_to(ROOT), set())
        in_block_comment = False
        for offset, line in enumerate(example.splitlines(), start=0):
            code, in_block_comment = strip_cpp_comments_and_strings(line, in_block_comment)
            for name in re.findall(r"\bstd::([A-Za-z_][A-Za-z0-9_]*)", code):
                if name not in allowed:
                    message = (
                        "Drop redundant std:: qualification in an example that imports std."
                        if imports_std
                        else "Import std in the example and drop the explicit qualification."
                    )
                    issues.append(
                        Issue(
                            path,
                            example_start_line + offset,
                            "code-consistency",
                            message,
                            line,
                        )
                    )
            if RANDOM_DEVICE_RE.search(code):
                issues.append(
                    Issue(
                        path,
                        example_start_line + offset,
                        "code-consistency",
                        "Use a fixed seed in examples so output and assertions are reproducible.",
                        line,
                    )
                )
            fixed_seed = FIXED_ENGINE_SEED_RE.search(code)
            wrapper_seed = FIXED_RANDOM_WRAPPER_SEED_RE.search(code)
            if (
                fixed_seed is not None and fixed_seed.group(1) != "1234567"
            ) or (
                wrapper_seed is not None and wrapper_seed.group(1) != "1234567"
            ):
                issues.append(
                    Issue(
                        path,
                        example_start_line + offset,
                        "code-consistency",
                        "Use fixed seed 1234567 consistently in examples and tests.",
                        line,
                    )
                )
        for match in EXAMPLE_HELPER_RE.finditer(example):
            name = match.group(1)
            if name == "main":
                continue
            if len(re.findall(r"\b" + re.escape(name) + r"\b", example)) == 1:
                issues.append(
                    Issue(
                        path,
                        example_start_line + example[: match.start()].count("\n"),
                        "code-consistency",
                        "Remove unused helper functions from example blocks.",
                        match.group(0).splitlines()[-1],
                    )
                )
    return issues


def strip_block_comments_and_strings(text):
    """Strip comments and string literals from a multi-line block of C++ source.

    Detectors that search for identifiers must run on code only. Prose in a comment can otherwise
    match a keyword by accident, as "puts both columns on one scale" once matched `puts`.
    """
    lines = []
    in_block_comment = False
    for line in text.splitlines():
        code, in_block_comment = strip_cpp_comments_and_strings(line, in_block_comment)
        lines.append(code)
    return "\n".join(lines)


def strip_cpp_comments_and_strings(line, in_block_comment):
    result = []
    i = 0
    while i < len(line):
        if in_block_comment:
            end = line.find("*/", i)
            if end == -1:
                return "".join(result), True
            in_block_comment = False
            i = end + 2
        elif line.startswith("//", i):
            break
        elif line.startswith("/*", i):
            in_block_comment = True
            i += 2
        elif line[i] in "\"'":
            quote = line[i]
            result.append(" ")
            i += 1
            while i < len(line):
                if line[i] == "\\":
                    i += 2
                elif line[i] == quote:
                    i += 1
                    break
                else:
                    i += 1
        else:
            result.append(line[i])
            i += 1
    return "".join(result), in_block_comment


def function_name_from_header(header):
    header = " ".join(header.split())
    if ") :" in header:
        header = header.rsplit(") :", 1)[0] + ")"
    if not header or "[" in header or "(" not in header:
        return ""
    if re.match(r"^(?:class|struct|union|enum|namespace)\b", header) or CONTROL_BLOCK_RE.match(
        header
    ):
        return ""
    matches = []
    for match in re.finditer(r"(?:operator\s*[^\s(]+|~?[A-Za-z_]\w*)\s*\(", header):
        prefix = header[: match.end() - 1]
        if prefix.count("(") == prefix.count(")"):
            matches.append(match)
    if not matches:
        return ""
    name = matches[-1].group(0).rsplit("(", 1)[0].strip()
    if CONTROL_BLOCK_RE.match(name):
        return ""
    return name


def scan_function_spacing(paths):
    issues = []
    for path in paths:
        lines = path.read_text().splitlines()
        cleaned = []
        in_block_comment = False
        for line in lines:
            code, in_block_comment = strip_cpp_comments_and_strings(line, in_block_comment)
            cleaned.append(code)

        brace_stack = []
        statement_start = None
        statement_parts = []
        functions = []
        for line_index, code in enumerate(cleaned):
            stripped_code = code.strip()
            if stripped_code.startswith("#") or stripped_code in {
                "public:",
                "protected:",
                "private:",
            }:
                statement_start = None
                statement_parts = []
                continue
            for char_index, char in enumerate(code):
                if statement_start is None and not char.isspace() and char not in "};":
                    statement_start = line_index
                if statement_start is not None:
                    statement_parts.append(char)

                if char == "{":
                    header = "".join(statement_parts[:-1]).strip()
                    starts_body = char_index == 0 or code[char_index - 1].isspace()
                    inside_function = any(kind == "function" for kind, _, _, _ in brace_stack)
                    if not inside_function and not starts_body:
                        brace_stack.append(("initializer", None, "", line_index))
                        continue
                    name = "" if inside_function else function_name_from_header(header)
                    brace_stack.append(
                        ("function" if name else "scope", statement_start, name, line_index)
                    )
                    statement_start = None
                    statement_parts = []
                elif char == "}":
                    if brace_stack:
                        kind, start, name, open_line = brace_stack.pop()
                        if kind == "initializer":
                            continue
                        if kind == "function" and start is not None:
                            functions.append((start, open_line, line_index, name))
                    statement_start = None
                    statement_parts = []
                elif char == ";":
                    statement_start = None
                    statement_parts = []

        for start, open_line, end, name in functions:
            if open_line == end:
                continue
            spacing_start = start
            while spacing_start > 0 and lines[spacing_start - 1].lstrip().startswith("//"):
                spacing_start -= 1
            if spacing_start > 0:
                previous = lines[spacing_start - 1].strip()
                if previous and not previous.endswith(("{", "public:", "protected:", "private:")):
                    issues.append(
                        Issue(
                            path,
                            start + 1,
                            "function-spacing",
                            f"Add an empty line before multiline function `{name}()`.",
                            lines[start],
                        )
                    )
            if end + 1 < len(lines):
                following = lines[end + 1].strip()
                if following and not following.startswith("}") and not following.endswith(
                    ("public:", "protected:", "private:")
                ):
                    issues.append(
                        Issue(
                            path,
                            end + 1,
                            "function-spacing",
                            f"Add an empty line after multiline function `{name}()`.",
                            lines[end],
                        )
                    )
    return issues


def scan_known_style_drift(paths):
    issues = []
    for path in paths:
        lines = path.read_text().splitlines()
        doc_lines = {
            i + 1
            for start, end in docstring_ranges(lines)
            for i in range(start, end + 1)
        }
        for line_no, line in enumerate(lines, start=1):
            if line_no not in doc_lines and COPY_LIST_CONTAINER_RE.search(line):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "style-drift",
                        "Use direct-list initialization for explicitly typed standard containers.",
                        line,
                    )
                )
            for pattern, message in STYLE_PATTERNS:
                if pattern.search(line):
                    issues.append(Issue(path, line_no, "style-drift", message, line))
            if line_no not in doc_lines:
                continue
            for match in UNWRAPPED_INTERVAL_RE.finditer(line):
                if line[:match.start()].count("$") % 2 == 0:
                    issues.append(
                        Issue(
                            path,
                            line_no,
                            "style-drift",
                            "Wrap numeric interval notation in math mode.",
                            line,
                        )
                    )
    return issues


def scan_emplace_opportunities(paths):
    issues = []
    braced_push_re = re.compile(
        r"\b([A-Za-z_][A-Za-z0-9_]*)\.(?:push|push_back|push_front)\s*\(\s*\{"
    )
    temporary_push_re = re.compile(
        r"\.(?:push|push_back|push_front)\s*\(\s*(?:std::)?[A-Z][A-Za-z0-9_:<>]*\s*\("
    )
    for path in paths:
        allowed = BRACED_PUSH_ALLOWLIST.get(path.relative_to(ROOT), set())
        for line_no, line in enumerate(path.read_text().splitlines(), start=1):
            braced = braced_push_re.search(line)
            if braced and braced.group(1) not in allowed:
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "emplace-opportunity",
                        "Construct the element in place unless brace insertion is required.",
                        line,
                    )
                )
            elif temporary_push_re.search(line):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "emplace-opportunity",
                        "Replace an explicitly constructed temporary with emplace arguments.",
                        line,
                    )
                )
    return issues


def scan_markup_outside_docstrings(paths):
    issues = []
    markup_re = re.compile(r"\$[^$\n]+\$|`[^`\n]+`")
    regex_capture_ref_re = re.compile(r"\$[0-9]+(?![0-9$])")
    for path in paths:
        in_block_comment = False
        for line_no, line in enumerate(path.read_text().splitlines(), start=1):
            outside = []
            pos = 0
            while pos < len(line):
                if in_block_comment:
                    end = line.find("*/", pos)
                    if end == -1:
                        break
                    in_block_comment = False
                    pos = end + 2
                    continue
                start = line.find("/*", pos)
                if start == -1:
                    outside.append(line[pos:])
                    break
                outside.append(line[pos:start])
                in_block_comment = True
                pos = start + 2

            outside_text = regex_capture_ref_re.sub("", "".join(outside))
            if markup_re.search(outside_text):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "markup-outside-docstring",
                        "Use math/code markup only in block docstrings; "
                        "listings render it literally.",
                        line,
                    )
                )
    return issues


def scan_result_parameter_name_collisions(paths):
    issues = []
    bullet_re = re.compile(
        r"(?ms)^- `([A-Za-z_][A-Za-z0-9_]*)\(([^\n`]*)\)`(.*?)(?=^- `|^\s*$)"
    )
    result_names_re = re.compile(
        r"\breturns?\s+(?:an?\s+|the\s+)?(?:analogous\s+)?(?:pair|tuple)"
        r"(?:\s+of\s+[A-Za-z-]+)?\s+"
        r"\((`[A-Za-z_][A-Za-z0-9_]*`(?:, `[A-Za-z_][A-Za-z0-9_]*`)+)\)",
        re.IGNORECASE,
    )
    for path in paths:
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            doc = "\n".join(lines[start + 1:end])
            for bullet in bullet_re.finditer(doc):
                description = bullet.group(3)
                result = result_names_re.search(description)
                if result is None:
                    continue
                params = bullet.group(2)
                for name in re.findall(r"`([A-Za-z_][A-Za-z0-9_]*)`", result.group(1)):
                    if re.search(rf"(?<![A-Za-z0-9_]){re.escape(name)}(?=\s*(?:=|,|$))", params):
                        line_no = start + 2 + doc[:bullet.start()].count("\n")
                        issues.append(
                            Issue(
                                path,
                                line_no,
                                "result-parameter-name",
                                f"Result field `{name}` repeats a parameter name; "
                                "make its role distinct.",
                                lines[line_no - 1],
                            )
                        )
    return issues


def scan_default_argument_docs(paths):
    issues = []
    bullet_call_re = re.compile(r"-\s+`([^`]+)`")
    code_decl_re = re.compile(
        r"^\s*(?:template<[^>]+>\s*)?"
        r"(?:explicit\s+)?(?:[A-Za-z_][\w:<>,\s*&~]*\s+)?"
        r"([A-Za-z_][A-Za-z0-9_]*)\s*\(([^{};]*=[^{};]*)\)"
        r"\s*(?:const\s*)?(?:\{|;|:)"
    )
    for path in paths:
        text = path.read_text()
        pre_example = text.split("/*** Example Usage", 1)[0]
        doc_text = "\n".join(re.findall(r"/\*(.*?)\*/", pre_example, flags=re.DOTALL))
        documented = {}
        for line_no, line in enumerate(text.splitlines(), start=1):
            match = bullet_call_re.search(line)
            if not match:
                continue
            signature = match.group(1)
            name_match = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)(?:<[^`]*>)?\s*\(", signature)
            if name_match:
                documented.setdefault(name_match.group(1), []).append((line_no, signature, line))

        internal_api_parameters = {
            "add_edge": {"rev_cap", "reverse_capacity"},
            "alpha_beta": {"alpha", "beta"},
            "find_centroid": {"u", "p", "parent"},
        }
        for name, forbidden in internal_api_parameters.items():
            for doc_line_no, signature, doc_line in documented.get(name, []):
                exposed = {
                    param
                    for param in forbidden
                    if re.search(rf"\b{re.escape(param)}\b", call_parameters(signature))
                }
                if exposed:
                    names = ", ".join(f"`{param}`" for param in sorted(exposed))
                    issues.append(
                        Issue(
                            path,
                            doc_line_no,
                            "api-internal-state",
                            f"Keep implementation parameter {names} out of `{name}()`'s public API.",
                            doc_line,
                        )
                    )

        for line_no, line in enumerate(pre_example.splitlines(), start=1):
            match = code_decl_re.match(line)
            if not match:
                continue
            params = split_parameters(match.group(2))
            defaulted_params = [
                param
                for param in params
                if top_level_default_expression(param)
            ]
            defaulted_names = []
            for param in defaulted_params:
                name_match = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*=", param)
                if name_match:
                    defaulted_names.append(name_match.group(1))
            internal_names = internal_api_parameters.get(match.group(1), set())
            if defaulted_names and set(defaulted_names) <= internal_names:
                continue
            internal_traversal_defaults = (
                len(defaulted_params) == len(params)
                and len(defaulted_names) >= 2
                and all(name in {"u", "v", "p", "parent"} for name in defaulted_names)
            )
            if internal_traversal_defaults:
                for doc_line_no, signature, doc_line in documented.get(match.group(1), []):
                    if "=" in signature:
                        issues.append(
                            Issue(
                                path,
                                doc_line_no,
                                "api-default",
                                "Omit defaulted traversal-state parameters from the public API bullet.",
                                doc_line,
                            )
                        )
                continue
            optional_outputs = True
            for param in defaulted_params:
                param_name = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*=", param)
                if (
                    "*" not in param
                    or top_level_default_expression(param) != "nullptr"
                    or not param_name
                    or not any(
                        re.search(r"&\s*" + re.escape(param_name.group(1)) + r"\b", signature)
                        for _, signature, _ in documented.get(match.group(1), [])
                    )
                ):
                    optional_outputs = False
                    break
            if defaulted_params and optional_outputs:
                continue
            if defaulted_params and all(
                re.search(r"\b(?:comp|hasher)\s*=", param) for param in defaulted_params
            ):
                continue
            name = match.group(1)
            if name not in documented:
                continue
            if re.search(
                r"`[^`]*\b" + re.escape(name) + r"(?:<[^`]*>)?\s*\([^`)]*=", doc_text
            ):
                continue
            doc_line_no, _, doc_line = documented[name][0]
            issues.append(
                Issue(
                    path,
                    doc_line_no,
                    "api-default",
                    f"`{name}()` has a default argument in code but not in the API bullet",
                    doc_line,
                )
            )
    return issues


def scan_api_parameter_names(paths):
    issues = []
    bullet_call_re = re.compile(r"-\s+`([^`]+)`")

    def parameter_names(params):
        names = []
        for param in split_parameters(params):
            default = top_level_default_expression(param)
            if default:
                param = param[: len(param) - len(default) - 1].rstrip()
            match = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^]]*\])?$", param)
            if not match or match.group(1) in {"void", "args"}:
                return None
            names.append(match.group(1))
        return tuple(names)

    for path in paths:
        text = path.read_text()
        pre_example = text.split("/*** Example Usage", 1)[0]
        declarations = {}
        for name, params in function_declarations(pre_example):
            names = parameter_names(params)
            if names is not None:
                declarations.setdefault(name, []).append(names)

        for line_no, line in enumerate(pre_example.splitlines(), start=1):
            match = bullet_call_re.search(line)
            if not match:
                continue
            if line[match.end() :].lstrip().startswith(":"):
                continue
            signature = match.group(1)
            name_match = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)(?:<[^`]*>)?\s*\(", signature)
            if not name_match:
                continue
            name = name_match.group(1)
            documented = parameter_names(call_parameters(signature))
            if documented is None:
                continue
            same_arity = [
                params for params in declarations.get(name, []) if len(params) == len(documented)
            ]
            if same_arity and documented not in same_arity:
                expected = " or ".join("(" + ", ".join(params) + ")" for params in same_arity)
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "api-params",
                        f"`{name}()` documents parameters {documented}, but code uses {expected}",
                        line,
                    )
                )
    return issues


def scan_api_output_argument_style(paths):
    issues = []
    bullet_call_re = re.compile(r"-\s+`([^`]+)`")

    def argument_name(param):
        default = top_level_default_expression(param)
        if default:
            param = param[: len(param) - len(default) - 1].rstrip()
        match = re.fullmatch(r"\s*&?\s*([A-Za-z_][A-Za-z0-9_]*)\s*", param)
        return match.group(1) if match else None

    def parameter_name(param):
        default = top_level_default_expression(param)
        if default:
            param = param[: len(param) - len(default) - 1].rstrip()
        match = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^]]*\])?$", param)
        return match.group(1) if match else None

    for path in paths:
        text = path.read_text()
        pre_example = text.split("/*** Example Usage", 1)[0]
        declarations = {}
        for name, params in function_declarations(pre_example):
            declarations.setdefault(name, []).append(split_parameters(params))
        for line_no, line in enumerate(pre_example.splitlines(), start=1):
            bullet = bullet_call_re.search(line)
            if not bullet:
                continue
            signature = bullet.group(1)
            doc_params = split_parameters(call_parameters(signature))
            addressed = [i for i, param in enumerate(doc_params) if param.lstrip().startswith("&")]
            if not addressed:
                continue
            name_match = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(", signature)
            if not name_match:
                continue
            name = name_match.group(1)
            doc_names = [argument_name(param) for param in doc_params]
            for code_params in declarations.get(name, []):
                if len(code_params) != len(doc_params):
                    continue
                if [parameter_name(param) for param in code_params] != doc_names:
                    continue
                wrong_address = []
                for i in addressed:
                    param = code_params[i]
                    param_name = parameter_name(param)
                    prefix = param[: param.rfind(param_name)]
                    if "*" not in prefix:
                        wrong_address.append(param_name)
                if wrong_address:
                    joined = ", ".join(f"`{param}`" for param in wrong_address)
                    issues.append(
                        Issue(
                            path,
                            line_no,
                            "api-address",
                            f"Do not take the address of reference parameter(s) {joined} in the "
                            "documented call.",
                            line,
                        )
                    )
                break
    return issues


def scan_complexity_call_parameter_names(paths):
    issues = []
    api_signature_re = re.compile(r"^-\s+`([^`]+)`")
    call_re = re.compile(r"`([A-Za-z_][A-Za-z0-9_]*)\(([^`]*)\)`")

    def argument_names(params):
        names = []
        for param in split_parameters(params):
            param = param.split("=", 1)[0].strip().lstrip("&")
            if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", param):
                return None
            names.append(param)
        return tuple(names)

    for path in paths:
        text = path.read_text()
        pre_example = text.split("/*** Example Usage", 1)[0]
        documented = {}
        for line in pre_example.splitlines():
            match = api_signature_re.search(line)
            if not match:
                continue
            signature = match.group(1)
            name_match = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(", signature)
            if not name_match:
                continue
            params = argument_names(call_parameters(signature[name_match.end() - 1 :]))
            if params is not None:
                documented.setdefault(name_match.group(1), []).append(params)

        in_complexity = False
        for line_no, line in enumerate(pre_example.splitlines(), start=1):
            stripped = line.strip()
            if stripped in {"Time Complexity:", "Space Complexity:"}:
                in_complexity = True
                continue
            if in_complexity and stripped == "*/":
                in_complexity = False
                continue
            if not in_complexity:
                continue
            for call in call_re.finditer(line):
                params = argument_names(call.group(2))
                if not params:
                    continue
                same_arity = [
                    expected
                    for expected in documented.get(call.group(1), [])
                    if len(expected) == len(params)
                ]
                if same_arity and params not in same_arity:
                    expected = " or ".join(
                        "(" + ", ".join(names) + ")" for names in same_arity
                    )
                    issues.append(
                        Issue(
                            path,
                            line_no,
                            "complexity-params",
                            f"`{call.group(1)}()` uses parameters {params}, but its API documents "
                            f"{expected}",
                            line,
                        )
                    )
    return issues


def function_declarations(text):
    header_re = re.compile(
        r"^\s*(?:template<[^\n]+>\s*\n\s*)?"
        r"(?:(?:static|inline|constexpr|virtual|friend|explicit)\s+)*"
        r"(?:[A-Za-z_][\w:<>, *&~]*\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*\(",
        flags=re.MULTILINE,
    )
    for header in header_re.finditer(text):
        name = header.group(1)
        if name in {"if", "while", "for", "switch", "return"}:
            continue
        open_paren = header.end() - 1
        params = call_parameters(text[open_paren:])
        close_paren = open_paren + len(params) + 1
        tail = text[close_paren + 1 :]
        definition = re.match(
            r"\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?"
            r"(?:->[^;{]+)?([:{])",
            tail,
        )
        if definition:
            yield name, params


def scan_documented_api_names(paths):
    issues = []
    bullet_call_re = re.compile(r"-\s+`([^`]+)`")
    for path in paths:
        text = path.read_text()
        pre_example = text.split("/*** Example Usage", 1)[0]
        doc_end = pre_example.find("*/")
        if doc_end == -1:
            continue
        code = pre_example[doc_end + 2 :]
        for line_no, line in enumerate(pre_example[: doc_end + 2].splitlines(), start=1):
            match = bullet_call_re.search(line)
            if not match:
                continue
            name_match = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)(?:<[^`]*>)?\s*\(", match.group(1))
            if not name_match:
                continue
            name = name_match.group(1)
            if match.group(1)[: name_match.start()].strip():
                continue
            if re.search(r"\b" + re.escape(name) + r"\s*\(", code):
                continue
            if re.search(r"\b(?:class|struct)\s+" + re.escape(name) + r"\b", code):
                continue
            issues.append(
                Issue(
                    path,
                    line_no,
                    "api-missing",
                    f"`{name}()` is documented but has no matching declaration",
                    line,
                )
            )
    return issues


def split_parameters(params):
    pieces = []
    start = 0
    depth = 0
    in_string = None
    escape = False
    for i, ch in enumerate(params):
        if in_string:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == in_string:
                in_string = None
            continue
        if ch in "\"'":
            in_string = ch
        elif ch in "([{<":
            depth += 1
        elif ch in ")]}>":
            depth = max(0, depth - 1)
        elif ch == "," and depth == 0:
            pieces.append(params[start:i].strip())
            start = i + 1
    tail = params[start:].strip()
    if tail:
        pieces.append(tail)
    return pieces


def top_level_default_expression(param):
    depth = 0
    in_string = None
    escape = False
    for i, ch in enumerate(param):
        if in_string:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == in_string:
                in_string = None
            continue
        if ch in "\"'":
            in_string = ch
        elif ch in "([{<":
            depth += 1
        elif ch in ")]}>":
            depth = max(0, depth - 1)
        elif ch == "=" and depth == 0:
            return param[i + 1:].strip()
    return ""


def repeated_literal_default_key(expr):
    if re.fullmatch(r'"(?:\\.|[^"\\])*"', expr) and len(expr) >= 6:
        return expr
    if re.fullmatch(r"[0-9][0-9'._A-Za-z]*", expr) and sum(ch.isdigit() for ch in expr) >= 6:
        return expr
    return ""


def call_parameters(signature):
    start = signature.find("(")
    if start == -1:
        return ""
    depth = 0
    in_string = None
    escape = False
    for i in range(start, len(signature)):
        ch = signature[i]
        if in_string:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == in_string:
                in_string = None
            continue
        if ch in "\"'":
            in_string = ch
        elif ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return signature[start + 1 : i]
    return ""


def code_interval_bounds_are_indices(line, context):
    interval = re.search(r"\$\[[^\]]*`[a-z]`[^\]]*[\]\)]\$", line)
    if not interval:
        return False
    vars_ = re.findall(r"`([a-z])`", interval.group(0))
    array_indices = bool(vars_) and all(
        re.search(r"`[^`]*\[" + var + r"\][^`]*`", context) for var in vars_
    )
    partition_invariant = "maintains, in order" in context and "unclassified" in context
    return array_indices or partition_invariant


def api_param_bound_is_math_dominant(var, context):
    math_spans = re.findall(r"\$([^$]*)\$", context)
    math_mentions = sum(1 for span in math_spans if re.search(r"\b" + re.escape(var) + r"\b", span))
    code_mentions = len(re.findall(r"`" + re.escape(var) + r"`", context))
    return math_mentions >= 2 and code_mentions == 0


def scan_repeated_literal_defaults(paths):
    issues = []
    code_decl_re = re.compile(
        r"^\s*(?:template<[^>]+>\s*)?"
        r"(?:explicit\s+)?(?:[A-Za-z_][\w:<>,\s*&~]*\s+)?"
        r"([A-Za-z_][A-Za-z0-9_]*)\s*\(([^{};]*=[^{};]*)\)"
        r"\s*(?:const\s*)?(?:\{|;|:)"
    )
    for path in paths:
        text = path.read_text()
        pre_example = text.split("/*** Example Usage", 1)[0]
        defaults = {}
        for line_no, line in enumerate(pre_example.splitlines(), start=1):
            match = code_decl_re.match(line)
            if not match:
                continue
            for param in split_parameters(match.group(2)):
                key = repeated_literal_default_key(top_level_default_expression(param))
                if key:
                    defaults.setdefault(key, []).append((line_no, line))

        for key, occurrences in defaults.items():
            if len(occurrences) < REPEATED_LITERAL_DEFAULT_THRESHOLD:
                continue
            line_no, line = occurrences[0]
            issues.append(
                Issue(
                    path,
                    line_no,
                    "literal-default",
                    f"default literal {key} appears in {len(occurrences)} signatures; "
                    "prefer a named constant",
                    line,
                )
            )
    return issues


def scan_contextual_math_style(paths):
    issues = []
    api_signature_re = re.compile(r"-\s+`([^`]*)`")
    indexed_code_re = re.compile(
        r"`([A-Za-z_][A-Za-z0-9_]*)((?:\[[A-Za-z_][A-Za-z0-9_]*\])+)`"
    )
    math_span_re = re.compile(r"(?<!\\)\$(?!\$)(.*?)(?<!\\)\$")
    bare_bound_re = re.compile(r"\$\[[0-9]+, ([A-Za-z_][A-Za-z0-9_]*)[\]\)]\$")
    code_bound_re = re.compile(r"\$\[[0-9]+, `([A-Za-z_][A-Za-z0-9_]*)`[\]\)]\$")
    prose_bound_re = re.compile(r"\$\[[^\]]*`[a-z]`[^\]]*[\]\)]\$")
    prose_math_name = r"(?:[a-z]|[A-Za-z][A-Za-z0-9]*_[A-Za-z0-9_]+)"
    prose_tuple_re = re.compile(r"\$\(`" + prose_math_name + r"`, `" + prose_math_name + r"`\)\$")
    prose_code_expr_re = re.compile(r"\$`[A-Za-z][A-Za-z0-9_]*`\s*[-+]")
    inferred_size_re = re.compile(r"where `([a-z])` is `[^`]+\.size\(\)`")
    default_policy_re = re.compile(r"<(?:[^`>]|<[^`>]*>)*(?:Compare|Hash)\s*=")

    def big_o_spans(text):
        spans = []
        pos = 0
        while True:
            start = text.find("O(", pos)
            if start == -1:
                return spans
            depth = 1
            end = start + 2
            while end < len(text) and depth:
                if text[end] == "(":
                    depth += 1
                elif text[end] == ")":
                    depth -= 1
                end += 1
            if depth == 0:
                spans.append((start, end, text[start + 2 : end - 1]))
            pos = max(end, start + 2)

    def documented_parameter_names(text):
        params_by_name = {}
        for line in text.splitlines():
            match = re.search(r"^-\s+`([^`]+)`", line)
            if not match:
                continue
            signature = match.group(1)
            name_match = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(", signature)
            if not name_match:
                continue
            names = set()
            for param in split_parameters(call_parameters(signature[name_match.end() - 1 :])):
                param = param.split("=", 1)[0].strip().lstrip("&")
                if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", param):
                    names.add(param)
            params_by_name.setdefault(name_match.group(1), set()).update(names)
        return params_by_name

    for path in paths:
        lines = path.read_text().splitlines()
        params_by_name = documented_parameter_names("\n".join(lines))
        for start, end in docstring_ranges(lines):
            if lines[start].startswith("/***"):
                continue
            i = start + 1
            while i < end:
                while i < end and not lines[i].strip():
                    i += 1
                paragraph_start = i
                while i < end and lines[i].strip():
                    i += 1
                paragraph_lines = lines[paragraph_start:i]
                paragraph = "\n".join(paragraph_lines)
                math_names = set()
                for math_match in math_span_re.finditer(paragraph):
                    if "`" not in math_match.group(1):
                        math_names.update(
                            re.findall(
                                r"(?<![A-Za-z_])([A-Za-z_][A-Za-z0-9_]*)",
                                math_match.group(1),
                            )
                        )
                for match in indexed_code_re.finditer(paragraph):
                    indices = tuple(re.findall(r"\[([A-Za-z_][A-Za-z0-9_]*)\]", match.group(2)))
                    explicit_bridge = re.search(
                        r"(?:Write|Let)\s+\$[^$]+\$\s+(?:for|as)\s+"
                        + re.escape(match.group(0)),
                        paragraph,
                    )
                    if not set(indices) <= math_names or explicit_bridge:
                        continue
                    issue_line = paragraph_start + paragraph[: match.start()].count("\n") + 1
                    issues.append(
                        Issue(
                            path,
                            issue_line,
                            "math-code-style",
                            "Keep an indexed state and its indices in one notation; use a math "
                            "state such as $dp(r, c)$ or code-style indices throughout.",
                            lines[issue_line - 1],
                        )
                    )
                spans = big_o_spans(paragraph)
                if not spans:
                    continue
                outside = paragraph
                for span_start, span_end, _ in reversed(spans):
                    outside = outside[:span_start] + outside[span_end:]
                code_names = set(re.findall(r"`([A-Za-z][A-Za-z0-9_]*)`", outside))
                math_names = set()
                for _, _, expression in spans:
                    expression = re.sub(r"`[^`]*`", "", expression)
                    math_names.update(
                        re.findall(
                            r"(?<![A-Za-z_])([A-Za-z][A-Za-z0-9_]*)", expression
                        )
                    )
                mentioned = set(
                    re.findall(r"`([A-Za-z_][A-Za-z0-9_]*)\s*\(", paragraph)
                )
                parameter_names = set()
                for name in mentioned:
                    parameter_names.update(params_by_name.get(name, set()))
                candidates = (
                    math_names
                    & code_names
                    - parameter_names
                    - {"alpha", "log", "max", "min", "sqrt"}
                )
                for name in sorted(candidates):
                    issue_line = next(
                        (
                            paragraph_start + offset + 1
                            for offset, line in enumerate(paragraph_lines)
                            if f"`{name}`" in line
                        ),
                        paragraph_start + 1,
                    )
                    issues.append(
                        Issue(
                            path,
                            issue_line,
                            "math-code-style",
                            f"Use math mode for analytical variable `{name}` introduced by the "
                            "Big-O expression.",
                            lines[issue_line - 1],
                        )
                    )

        in_doc = False
        in_api_bullet = False
        api_params = set()
        constructor_params = set()
        for index, line in enumerate(lines):
            line_no = index + 1
            if "/*" in line:
                in_doc = True
                in_api_bullet = False
                api_params = set()
                constructor_params = set()
            if not in_doc:
                continue

            stripped = line.lstrip()
            if stripped.startswith("- `"):
                in_api_bullet = True
                signature = api_signature_re.search(stripped)
                api_params = set()
                if signature:
                    signature_params = set()
                    for param in split_parameters(call_parameters(signature.group(1))):
                        name = param.split("=", 1)[0].strip().lstrip("&*")
                        if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
                            signature_params.add(name)
                    api_params.update(signature_params)
                    template_prefix = signature.group(1).split("(", 1)[0]
                    api_params.update(
                        re.findall(r"(?<![A-Za-z0-9_])([A-Z][A-Z0-9_]*)\b", template_prefix)
                    )
                    call_name = re.search(
                        r"([A-Za-z_][A-Za-z0-9_]*)(?:<[^`]*>)?\s*\(", signature.group(1)
                    )
                    if call_name and call_name.group(1)[0].isupper():
                        constructor_params.update(signature_params)
            elif stripped.startswith("- "):
                in_api_bullet = False
                api_params = set()
            elif stripped and not line.startswith("  "):
                in_api_bullet = False
                api_params = set()

            prev_line = lines[index - 1] if index > 0 else ""
            next_line = lines[index + 1] if index + 1 < len(lines) else ""
            local_context = prev_line.rstrip() + " " + line + " " + next_line.lstrip()

            if in_api_bullet and default_policy_re.search(local_context):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "api-signature-style",
                        "Move default comparator or hasher policy details out of API signatures.",
                        line,
                    )
                )

            if "where `" in line and inferred_size_re.search(local_context):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "math-code-style",
                        "Avoid introducing a code variable solely to restate a container's size.",
                        line,
                    )
                )

            bare_bound = bare_bound_re.search(line)
            if (
                in_api_bullet
                and bare_bound
                and (bare_bound.group(1) in api_params or bare_bound.group(1)[0].isupper())
                and not api_param_bound_is_math_dominant(bare_bound.group(1), local_context)
            ):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "math-code-style",
                        "Use backticks for API parameter bounds, e.g. $[0, `n`)$.",
                        line,
                    )
                )
            code_bound = code_bound_re.search(line)
            code_bound_is_explained = (
                code_bound
                and (
                    f"where `{code_bound.group(1)}` is" in local_context
                    or f"where `{code_bound.group(1)}` =" in local_context
                )
            )
            if (
                in_api_bullet
                and code_bound
                and code_bound.group(1) not in api_params
                and code_bound.group(1) not in constructor_params
                and not code_bound.group(1)[0].isupper()
                and not code_bound_is_explained
            ):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "math-code-style",
                        "Use plain math for non-parameter interval bounds, e.g. $[0, k)$.",
                        line,
                    )
                )
            if in_doc and not in_api_bullet and prose_tuple_re.search(line):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "math-code-style",
                        "Use plain math variables in conceptual prose tuples, e.g. $(i, j)$.",
                        line,
                    )
                )
            if (
                in_doc
                and not in_api_bullet
                and prose_bound_re.search(line)
                and not code_interval_bounds_are_indices(line, local_context)
            ):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "math-code-style",
                        "Use plain math variables in conceptual prose intervals, e.g. $[0, n)$.",
                        line,
                    )
                )
            if in_doc and not in_api_bullet and prose_code_expr_re.search(line):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "math-code-style",
                        "Use plain math variables in conceptual prose expressions, e.g. $n - 1$.",
                        line,
                    )
                )

            if "*/" in line:
                in_doc = False
                in_api_bullet = False
    return issues


def run_git_diff_check():
    proc = subprocess.run(
        ["git", "diff", "--check"],
        cwd=ROOT,
        capture_output=True,
        text=True,
    )
    output = (proc.stdout + proc.stderr).strip()
    return proc.returncode == 0, output


def scan_section_cross_references(paths):
    """Flags prose that points at a section number which does not exist, or at its own section.

    References are matched against the flattened docstring rather than line by line, since a
    reference may wrap onto the next line or end a sentence, and both forms hid real breakage
    when sections were renumbered.
    """
    issues = []
    sections = {}
    for path in paths:
        match = re.match(r"^(\d+\.\d+(?:\.\d+)?)_", path.name)
        if match:
            sections[match.group(1)] = path
    # Either a keyword-introduced reference or a bare parenthesized one, which is how several
    # stale references survived an earlier renumbering. The parenthesized form requires all three
    # parts, since a bare decimal in parentheses is far more often a number than a section.
    reference_re = re.compile(
        r"\b(?:see|See|sections?|in|from|of)\s+([1-8]\.\d+(?:\.\d+)?)(?![\d.])"
        r"|\(([1-8]\.\d+\.\d+)\)"
    )
    for path in paths:
        own = re.match(r"^(\d+\.\d+(?:\.\d+)?)_", path.name)
        own = own.group(1) if own else None
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            block = lines[start : end + 1]
            flat = " ".join(" ".join(block).split())
            for match in reference_re.finditer(flat):
                ref = match.group(1) or match.group(2)
                if ref not in sections:
                    issues.append(
                        Issue(
                            path,
                            start + 1,
                            "cross-reference",
                            f"Section {ref} does not exist; update the reference.",
                            f"-> {ref}",
                        )
                    )
                elif ref == own:
                    issues.append(
                        Issue(
                            path,
                            start + 1,
                            "cross-reference",
                            f"Section {ref} refers to itself; the target was probably renumbered.",
                            f"-> {ref}",
                        )
                    )
    return issues


def main():
    paths = list(cpp_files())
    issues = []
    issues.extend(scan_docstrings(paths))
    issues.extend(scan_docstring_wording(paths))
    issues.extend(scan_docstring_structure(paths))
    issues.extend(scan_overflow_warning_labels(paths))
    issues.extend(scan_post_api_contracts(paths))
    issues.extend(scan_ordinal_base_docs(paths))
    issues.extend(scan_size_declaration_order(paths))
    issues.extend(scan_time_complexity_scope(paths))
    issues.extend(scan_grid_dimension_symbols(paths))
    issues.extend(scan_atomic_inline_wrapping(paths))
    issues.extend(scan_example_markers(paths))
    issues.extend(scan_inlineable_example_variables(paths))
    issues.extend(scan_code_consistency(paths))
    issues.extend(scan_function_spacing(paths))
    issues.extend(scan_known_style_drift(paths))
    issues.extend(scan_emplace_opportunities(paths))
    issues.extend(scan_markup_outside_docstrings(paths))
    issues.extend(scan_result_parameter_name_collisions(paths))
    issues.extend(scan_default_argument_docs(paths))
    issues.extend(scan_api_parameter_names(paths))
    issues.extend(scan_api_output_argument_style(paths))
    issues.extend(scan_complexity_call_parameter_names(paths))
    issues.extend(scan_documented_api_names(paths))
    issues.extend(scan_repeated_literal_defaults(paths))
    issues.extend(scan_contextual_math_style(paths))
    issues.extend(scan_section_cross_references(paths))

    ok, diff_output = run_git_diff_check()
    if not ok:
        issues.append(
            Issue(
                ROOT / "git-diff",
                0,
                "whitespace",
                "git diff --check reported whitespace errors",
                diff_output,
            )
        )

    if issues:
        print(f"scan_quality: {len(issues)} issue(s) found\n")
        for issue in issues:
            print(issue.format())
            print()
        return 1

    print("scan_quality: stable")
    return 0


if __name__ == "__main__":
    sys.exit(main())
