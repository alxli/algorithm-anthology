#!/usr/bin/env python3
"""Reflow /* */ block comments in C++ files to 100 characters wide.

Reflows paragraph text and bullet-list items. Preserves:
  - Blank lines exactly as they are (never added or removed).
  - Indented content (displayed math, code fragments, etc.) verbatim.
  - Example Usage / Output blocks entirely.
  - Single-line block comments (e.g. /* Wrapper */).

Backtick-quoted spans such as `func(a, b)` are treated as atomic tokens
and are never split across a line break.

Usage:
  python3 tools/reflow_comments.py [file.cpp ...]   # named files
  python3 tools/reflow_comments.py                  # all chapter .cpp files
"""

import re
import sys
from pathlib import Path

WIDTH = 100

# /*** Example Usage ***/ uses /* ... */ syntax, so group(1) starts with '**'.
# Strip any leading asterisks (and whitespace) before matching.
_EXAMPLE_RE = re.compile(r'Example Usage', re.I)

# Lines that are metadata headers (parsed specially by gen_chapters.py) must
# never be joined with adjacent lines — each is its own standalone unit.
_META_LINE_RE = re.compile(
    r'^(?:Time Complexity(?:\s*\([^)]*\))?|Space Complexity(?:\s*\([^)]*\))?|Stable\?)'
)


def tokenize(text):
    """Split into words; backtick spans (with any trailing punctuation) are atomic."""
    return re.findall(r'`[^`]*`[^\s`]*|\S+', text)


def wrap(words, first_indent, cont_indent):
    """Word-wrap a list of tokens to WIDTH, yielding lines."""
    if not words:
        return []
    lines = []
    cur = first_indent + words[0]
    for w in words[1:]:
        if len(cur) + 1 + len(w) <= WIDTH:
            cur += ' ' + w
        else:
            lines.append(cur)
            cur = cont_indent + w
    lines.append(cur)
    return lines


def reflow_body(text):
    """Reflow the content lines inside a /* */ block (without the delimiters)."""
    lines = text.split('\n')
    out = []
    i = 0
    while i < len(lines):
        line = lines[i]

        # ── blank line: preserve exactly ──────────────────────────────────────
        if not line.strip():
            out.append(line)
            i += 1
            continue

        # ── bullet item: '- ...' with '  ' continuation lines ────────────────
        if line.startswith('- '):
            body = line[2:]
            i += 1
            while i < len(lines):
                nxt = lines[i]
                # A continuation is indented by exactly 2 spaces, is not blank,
                # and does not start a new bullet when stripped.
                if (nxt.startswith('  ') and not nxt.startswith('   ')
                        and nxt.strip()
                        and not nxt.lstrip().startswith('- ')):
                    body += ' ' + nxt.strip()
                    i += 1
                else:
                    break
            out.extend(wrap(tokenize(body), '- ', '  '))
            continue

        # ── metadata header (Time/Space Complexity, Stable?): always standalone ─
        if _META_LINE_RE.match(line):
            out.extend(wrap(tokenize(line), '', ''))
            i += 1
            continue

        # ── regular 0-indent paragraph ────────────────────────────────────────
        if not line[0:1].isspace():
            parts = [line]
            i += 1
            while i < len(lines):
                nxt = lines[i]
                # Stop at blank lines, bullets, indented content, or metadata headers.
                if (not nxt.strip() or nxt.startswith('- ')
                        or nxt[0:1].isspace() or _META_LINE_RE.match(nxt)):
                    break
                parts.append(nxt)
                i += 1
            text = ' '.join(p.strip() for p in parts)
            out.extend(wrap(tokenize(text), '', ''))
            continue

        # ── indented verbatim content (displayed math, code, etc.) ───────────
        out.append(line)
        i += 1

    return '\n'.join(out)


def process_comment(m):
    content = m.group(1)
    # Skip Example Usage / Output blocks. /*** ... ***/ produces content that
    # starts with leading '*' chars, so strip those before matching.
    if _EXAMPLE_RE.search(content.lstrip('*\n').split('\n')[0]):
        return m.group(0)
    # Skip single-line comments like /* Wrapper */.
    if '\n' not in content.strip():
        return m.group(0)
    # Standard form: content = '\n...\n' (newline right after /* and before */).
    if content.startswith('\n') and content.endswith('\n'):
        new_body = reflow_body(content[1:-1])
        return '/*\n' + new_body + '\n*/'
    # Fallback for unusual delimiter placement.
    return '/*' + reflow_body(content) + '*/'


def process_file(path):
    src = path.read_text(encoding='utf-8')
    new_src = re.sub(r'/\*(.*?)\*/', process_comment, src, flags=re.DOTALL)
    if new_src != src:
        path.write_text(new_src, encoding='utf-8')
        return True
    return False


def main():
    if len(sys.argv) > 1:
        paths = [Path(p).resolve() for p in sys.argv[1:]]
    else:
        root = Path(__file__).parent.parent
        paths = sorted(
            p
            for d in root.iterdir()
            if re.match(r'\d+-', d.name) and d.is_dir()
            for p in sorted(d.glob('*.cpp'))
        )

    changed = 0
    for p in paths:
        try:
            if process_file(p):
                rel = p.relative_to(p.parent.parent)
                print(f'  {rel}')
                changed += 1
        except Exception as e:
            print(f'ERROR: {p}: {e}', file=sys.stderr)
            raise

    print(f'\n{changed} / {len(paths)} files changed.')


if __name__ == '__main__':
    main()
