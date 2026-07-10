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

STYLE_PATTERNS = [
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
        re.compile(r"`[^`]*\bcomp = Compare\(\)"),
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

    return text.split()[0]


def is_docstring_continuation(line, next_line):
    stripped = line.strip()
    next_stripped = next_line.strip()
    if not stripped or not next_stripped:
        return False
    if stripped in {"/*", "*/"} or next_stripped in {"/*", "*/"}:
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
                    and (next_line.startswith("  ") or token.startswith(("`", "$")))
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


def scan_known_style_drift(paths):
    issues = []
    for path in paths:
        for line_no, line in enumerate(path.read_text().splitlines(), start=1):
            for pattern, message in STYLE_PATTERNS:
                if pattern.search(line):
                    issues.append(Issue(path, line_no, "style-drift", message, line))
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
    return bool(vars_) and all(re.search(r"`[^`]*\[" + var + r"\][^`]*`", context) for var in vars_)


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
    prose_tuple_re = re.compile(r"\$\(`[a-z]`, `[a-z]`\)\$")
    prose_code_expr_re = re.compile(r"\$`[A-Za-z][A-Za-z0-9_]*`\s*[-+]")
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
    issues.extend(scan_atomic_inline_wrapping(paths))
    issues.extend(scan_example_markers(paths))
    issues.extend(scan_known_style_drift(paths))
    issues.extend(scan_default_argument_docs(paths))
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
        print(f"scan_docstrings: {len(issues)} issue(s) found\n")
        for issue in issues:
            print(issue.format())
            print()
        return 1

    print("scan_docstrings: stable")
    return 0


if __name__ == "__main__":
    sys.exit(main())
