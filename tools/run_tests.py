#!/usr/bin/env python3
"""Build and run all chapter .cpp files, verifying they compile and exit cleanly.

Usage:
  python3 tools/run_tests.py              # build + run, show failures only
  python3 tools/run_tests.py --output     # also print stdout from each file
  python3 tools/run_tests.py --chapter 2  # run one chapter
  python3 tools/run_tests.py 2-Data-Structures/2.3.2_Treap.cpp  # single file
"""

import argparse
import re
import subprocess
import sys
import tempfile
from pathlib import Path

CXX = '/opt/homebrew/bin/g++-15'
CXXFLAGS = ['-std=c++17', '-O2', '-Wall', '-Wno-unused-variable']
TIMEOUT = 30  # seconds per file


def compile_and_run(src: Path, show_output: bool) -> tuple[bool, str]:
    """Compile and run one file. Returns (ok, message)."""
    with tempfile.NamedTemporaryFile(suffix='', delete=True) as tmp:
        exe = tmp.name

    # Compile
    compile_cmd = [CXX, *CXXFLAGS, str(src), '-o', exe]
    comp = subprocess.run(compile_cmd, capture_output=True, text=True)
    if comp.returncode != 0:
        return False, f'COMPILE ERROR:\n{comp.stderr.strip()}'

    # Run
    try:
        run = subprocess.run(
            [exe], capture_output=True, text=True, timeout=TIMEOUT
        )
    except subprocess.TimeoutExpired:
        return False, f'TIMEOUT after {TIMEOUT}s'
    finally:
        Path(exe).unlink(missing_ok=True)

    if run.returncode != 0:
        msg = f'RUNTIME ERROR (exit {run.returncode})'
        if run.stderr.strip():
            msg += f':\n{run.stderr.strip()}'
        return False, msg

    if show_output and run.stdout.strip():
        return True, run.stdout.rstrip()
    return True, ''


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('files', nargs='*', help='Specific .cpp files (default: all chapters)')
    parser.add_argument('--chapter', '-c', type=int, help='Run all .cpp files in one chapter')
    parser.add_argument('--output', '-o', action='store_true', help='Print stdout from each file')
    args = parser.parse_args()

    root = Path(__file__).parent.parent

    if args.files and args.chapter is not None:
        parser.error('--chapter cannot be combined with explicit files')

    chapter_dirs = [
        d
        for d in root.iterdir()
        if re.match(r'\d+-', d.name) and d.is_dir()
    ]

    if args.chapter is not None:
        prefix = f'{args.chapter}-'
        dirs = [d for d in chapter_dirs if d.name.startswith(prefix)]
        if not dirs:
            parser.error(f'chapter {args.chapter} directory not found')
        paths = sorted(p for d in dirs for p in d.glob('*.cpp'))
    elif args.files:
        paths = [Path(f).resolve() for f in args.files]
    else:
        paths = sorted(
            p
            for d in chapter_dirs
            for p in sorted(d.glob('*.cpp'))
        )

    passed = failed = 0
    failures = []

    for p in paths:
        rel = p.relative_to(root)
        ok, msg = compile_and_run(p, args.output)
        if ok:
            passed += 1
            status = 'PASS'
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

    total = passed + failed
    print(f'\n{passed}/{total} passed', end='')
    if failed:
        print(f', {failed} failed')
    else:
        print()

    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
