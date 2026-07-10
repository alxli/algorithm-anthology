#!/usr/bin/env python3

from dataclasses import dataclass
import re
import signal
import subprocess
import sys
from pathlib import Path


LIMIT = 100
ROOT = Path(__file__).resolve().parent
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


def scan_docstrings(paths):
    issues = []
    for path in paths:
        rel = path.relative_to(ROOT)
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            for i in range(start, end):
                line = lines[i]
                next_line = lines[i + 1]
                token = first_wrapped_token(next_line)
                if (
                    line.strip()
                    and next_line.startswith("  ")
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
    issues.extend(scan_example_markers(paths))
    issues.extend(scan_known_style_drift(paths))
    issues.extend(scan_default_argument_docs(paths))

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
