#!/usr/bin/env python3

import signal
from pathlib import Path


LIMIT = 100
signal.signal(signal.SIGPIPE, signal.SIG_DFL)


def cpp_files():
    for path in sorted(Path(".").rglob("*.cpp")):
        if "Book" not in path.parts:
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


def scan_underfull():
    found = False
    for path in cpp_files():
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
                    found = True
                    print(f"{path}:{i + 1}: {len(line.rstrip())} -> {len(line.rstrip() + ' ' + token)}")
                    print(f"  {line.rstrip()}")
                    print(f"  {next_line}")
    return found


def scan_overfull():
    found = False
    for path in cpp_files():
        lines = path.read_text().splitlines()
        for start, end in docstring_ranges(lines):
            for i in range(start, end + 1):
                line = lines[i].rstrip()
                if len(line) > LIMIT:
                    found = True
                    print(f"{path}:{i + 1}: {len(line)}: {line}")
    return found


print("Underfull docstring lines:")
if not scan_underfull():
    print("  none")

print()
print("Overfull docstring lines:")
if not scan_overfull():
    print("  none")
