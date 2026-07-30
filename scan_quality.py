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
    Path("6-Mathematics/6.7.2_Nim_Product.cpp"): {8},
}

EXAMPLE_OUTPUT_WITHOUT_DIRECT_PRINT = {
    Path("8-Miscellany/8.3_Debugging.cpp"),
}

EXAMPLE_HELPER_RE = re.compile(
    r"(?m)^(?:template<[^>\n]+>\n)?"
    r"(?:[A-Za-z_][\w:<>,&* ]+\s+)+"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*\{"
)
C_RAND_RE = re.compile(
    r"((?<![A-Za-z0-9_])::rand\s*\(|(?<!::)\brand\s*\(\s*\)\s*%|\bsrand\s*\()"
)
QUALIFIED_FABS_RE = re.compile(r"\bstd::fabs\b")
CAST_EPS_RE = re.compile(r"\bstatic_cast<C>\(EPS\)")
LEGACY_BOUNDARY_FLAG_RE = re.compile(r"\b(?:edge_is_inside|touch_is_intersect)\b")
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

STYLE_PATTERNS = [
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
    if line.startswith("  ") and stripped.startswith(("$", "`")):
        return False
    if next_line.startswith("  ") and next_stripped.startswith(("$", "`")):
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
        body = text[match.end():]
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


def scan_code_consistency(paths):
    issues = []
    for path in paths:
        text = path.read_text()
        lines = text.splitlines()
        in_block_comment = False
        for line_no, line in enumerate(lines, start=1):
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
            if QUALIFIED_FABS_RE.search(code):
                issues.append(
                    Issue(
                        path,
                        line_no,
                        "code-consistency",
                        "Use plain fabs() to match the anthology's <cmath> convention.",
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
        in_block_comment = False
        for offset, line in enumerate(example.splitlines(), start=0):
            code, in_block_comment = strip_cpp_comments_and_strings(line, in_block_comment)
            if re.search(r"(?:std::)?random_device\s*\{\s*\}", code):
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


def scan_markup_outside_docstrings(paths):
    issues = []
    markup_re = re.compile(r"\$[^$\n]+\$|`[^`\n]+`")
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

            outside_text = "".join(outside)
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

        for line_no, line in enumerate(pre_example.splitlines(), start=1):
            match = code_decl_re.match(line)
            if not match:
                continue
            defaulted_params = [
                param
                for param in split_parameters(match.group(2))
                if top_level_default_expression(param)
            ]
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
    bare_bound_re = re.compile(r"\$\[[0-9]+, ([a-z])[\]\)]\$")
    code_bound_re = re.compile(r"\$\[[0-9]+, `([a-z])`[\]\)]\$")
    prose_bound_re = re.compile(r"\$\[[^\]]*`[a-z]`[^\]]*[\]\)]\$")
    prose_math_name = r"(?:[a-z]|[A-Za-z][A-Za-z0-9]*_[A-Za-z0-9_]+)"
    prose_tuple_re = re.compile(r"\$\(`" + prose_math_name + r"`, `" + prose_math_name + r"`\)\$")
    prose_code_expr_re = re.compile(r"\$`[A-Za-z][A-Za-z0-9_]*`\s*[-+]")
    inferred_size_re = re.compile(r"where `([a-z])` is `[^`]+\.size\(\)`")
    default_policy_re = re.compile(r"<(?:[^`>]|<[^`>]*>)*(?:Compare|Hash)\s*=")
    for path in paths:
        lines = path.read_text().splitlines()
        in_doc = False
        in_api_bullet = False
        api_params = set()
        for index, line in enumerate(lines):
            line_no = index + 1
            if "/*" in line:
                in_doc = True
                in_api_bullet = False
                api_params = set()
            if not in_doc:
                continue

            stripped = line.lstrip()
            if stripped.startswith("- `"):
                in_api_bullet = True
                signature = api_signature_re.search(stripped)
                api_params = set()
                if signature:
                    for param in split_parameters(call_parameters(signature.group(1))):
                        name = re.search(r"\b([a-z])\b(?:\s*=.*)?$", param.strip())
                        if name:
                            api_params.add(name.group(1))
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
                and bare_bound.group(1) in api_params
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


def main():
    paths = list(cpp_files())
    issues = []
    issues.extend(scan_docstrings(paths))
    issues.extend(scan_time_complexity_scope(paths))
    issues.extend(scan_atomic_inline_wrapping(paths))
    issues.extend(scan_example_markers(paths))
    issues.extend(scan_code_consistency(paths))
    issues.extend(scan_function_spacing(paths))
    issues.extend(scan_known_style_drift(paths))
    issues.extend(scan_markup_outside_docstrings(paths))
    issues.extend(scan_result_parameter_name_collisions(paths))
    issues.extend(scan_default_argument_docs(paths))
    issues.extend(scan_api_parameter_names(paths))
    issues.extend(scan_api_output_argument_style(paths))
    issues.extend(scan_documented_api_names(paths))
    issues.extend(scan_repeated_literal_defaults(paths))
    issues.extend(scan_contextual_math_style(paths))

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
