#!/usr/bin/env python3
"""Build and run all chapter .cpp files, verifying they compile and exit cleanly.

Usage:
  python3 tools/run_examples.py              # build + run, show failures only
  python3 tools/run_examples.py --output     # also print stdout from each file
  python3 tools/run_examples.py -c 2         # run one chapter
  python3 tools/run_examples.py -c 4.2       # run one section
  python3 tools/run_examples.py -c 4.2.4     # run one subsection
  python3 tools/run_examples.py 2-Data-Structures/2.3.2_Treap.cpp  # single file
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Optional

CXX = os.environ.get('CXX', '/opt/homebrew/bin/g++-15')
CXXFLAGS = ['-std=c++17', '-O2', '-Wall', '-pedantic', '-Wno-psabi']
TIMEOUT = 30  # seconds per file


def section_key(path: Path) -> Optional[str]:
    """Return the leading numeric section key from a filename, e.g. '4.2.4'."""
    match = re.match(r'(\d+(?:\.\d+)*)_', path.name)
    return match.group(1) if match else None


def matches_chapter_filter(path: Path, chapter: str) -> bool:
    key = section_key(path)
    return key == chapter or (key is not None and key.startswith(f'{chapter}.'))


def compile_and_run(src: Path, show_output: bool) -> tuple[bool, str, str]:
    """Compile and run one file. Returns (ok, message, warnings)."""
    with tempfile.NamedTemporaryFile(suffix='', delete=True) as tmp:
        exe = tmp.name

    # Compile (capture warnings even on success).
    compile_cmd = [CXX, *CXXFLAGS, str(src), '-o', exe]
    comp = subprocess.run(compile_cmd, capture_output=True, text=True)
    warnings = comp.stderr.strip()
    if comp.returncode != 0:
        return False, f'COMPILE ERROR:\n{warnings}', ''

    # Run
    try:
        run = subprocess.run(
            [exe], capture_output=True, text=True, timeout=TIMEOUT
        )
    except subprocess.TimeoutExpired:
        return False, f'TIMEOUT after {TIMEOUT}s', warnings
    finally:
        Path(exe).unlink(missing_ok=True)

    if run.returncode != 0:
        msg = f'RUNTIME ERROR (exit {run.returncode})'
        if run.stderr.strip():
            msg += f':\n{run.stderr.strip()}'
        return False, msg, warnings

    if show_output and run.stdout.strip():
        return True, run.stdout.rstrip(), warnings
    return True, '', warnings


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('files', nargs='*', help='Specific .cpp files (default: all chapters)')
    parser.add_argument('--chapter', '-c', help='Run a chapter/section/subsection, e.g. 4, 4.2, 4.2.4')
    parser.add_argument('--output', '-o', action='store_true', help='Print stdout from each file')
    args = parser.parse_args()

    root = Path(__file__).parent.parent

    if args.files and args.chapter is not None:
        parser.error('--chapter cannot be combined with explicit files')
    if args.chapter is not None and not re.fullmatch(r'\d+(?:\.\d+)*', args.chapter):
        parser.error('--chapter must be a numeric prefix like 4, 4.2, or 4.2.4')

    chapter_dirs = [
        d
        for d in root.iterdir()
        if re.match(r'\d+-', d.name) and d.is_dir()
    ]

    if args.chapter is not None:
        chapter_num = args.chapter.split('.')[0]
        dirs = [d for d in chapter_dirs if d.name.startswith(f'{chapter_num}-')]
        if not dirs:
            parser.error(f'chapter {chapter_num} directory not found')
        paths = sorted(
            p
            for d in dirs
            for p in d.glob('*.cpp')
            if matches_chapter_filter(p, args.chapter)
        )
        if not paths:
            parser.error(f'no .cpp files found for chapter filter {args.chapter}')
    elif args.files:
        paths = [Path(f).resolve() for f in args.files]
    else:
        paths = sorted(
            p
            for d in chapter_dirs
            for p in sorted(d.glob('*.cpp'))
        )

    passed = failed = warned = 0
    failures = []

    for p in paths:
        rel = p.relative_to(root)
        ok, msg, warnings = compile_and_run(p, args.output)
        if ok:
            passed += 1
            status = 'WARN' if warnings else 'PASS'
            if msg:
                print(f'\n{rel}:')
                print(msg)
            else:
                print(f'  {status}  {rel}')
        else:
            failed += 1
            failures.append((rel, msg))
            print(f'  FAIL  {rel}')
            for line in msg.splitlines():
                print(f'        {line}')

        if warnings:
            warned += 1
            for line in warnings.splitlines():
                print(f'        {line}')

    total = passed + failed
    print(f'\n{passed}/{total} passed', end='')
    if failed:
        print(f', {failed} failed', end='')
    if warned:
        print(f', {warned} with warnings', end='')
    print()

    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
