#!/usr/bin/env python3
"""Build and run all chapter .cpp files, verifying they compile and exit cleanly.

Usage:
  python3 run_examples.py              # build + run, show failures only
  python3 run_examples.py --output     # also print stdout from each file
  python3 run_examples.py -s 2         # run one chapter
  python3 run_examples.py -s 4.2       # run one section
  python3 run_examples.py -s 4.2.4     # run one subsection
  python3 run_examples.py -s 4.3.4 6.3.7  # run multiple sections
  python3 run_examples.py 2-Data-Structures/2.3.2_Treap.cpp  # single file
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Optional

DEFAULT_CXX_CANDIDATES = ('/opt/homebrew/bin/g++-15', 'g++')
CXXFLAGS = ['-std=c++17', '-O2', '-Wall', '-pedantic']
TIMEOUT = 30  # seconds per file


def find_compiler() -> Optional[str]:
    """Return the configured compiler or the first available default."""
    if cxx := os.environ.get('CXX'):
        return cxx
    return next((path for candidate in DEFAULT_CXX_CANDIDATES
                 if (path := shutil.which(candidate))), None)


def section_key(path: Path) -> Optional[str]:
    """Return the leading numeric section key from a filename, e.g. '4.2.4'."""
    match = re.match(r'(\d+(?:\.\d+)*)_', path.name)
    return match.group(1) if match else None


def matches_section_filter(path: Path, section: str) -> bool:
    key = section_key(path)
    return key == section or (key is not None and key.startswith(f'{section}.'))


def compile_and_run(src: Path, show_output: bool, cxx: str) -> tuple[bool, str, str]:
    """Compile and run one file. Returns (ok, message, warnings)."""
    with tempfile.NamedTemporaryFile(suffix='', delete=True) as tmp:
        exe = tmp.name

    # Compile (capture warnings even on success).
    compile_cmd = [cxx, *CXXFLAGS, str(src), '-o', exe]
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
    parser.add_argument(
        '--section', '-s', nargs='+',
        help='Run one or more chapter/section/subsection prefixes, e.g. 4.2 6.3.7',
    )
    parser.add_argument('--output', '-o', action='store_true', help='Print stdout from each file')
    args = parser.parse_args()

    root = Path(__file__).parent
    cxx = find_compiler()
    if cxx is None:
        parser.error('no C++ compiler found; install g++ or set CXX')

    if args.files and args.section:
        parser.error('--section cannot be combined with explicit files')
    invalid_sections = [
        section
        for section in args.section or []
        if not re.fullmatch(r'\d+(?:\.\d+)*', section)
    ]
    if invalid_sections:
        parser.error(
            '--section values must be numeric prefixes like 4, 4.2, or 4.2.4: '
            + ', '.join(invalid_sections)
        )

    chapter_dirs = [
        d
        for d in root.iterdir()
        if re.match(r'\d+-', d.name) and d.is_dir()
    ]

    if args.section:
        matching_paths = {
            p
            for d in chapter_dirs
            for p in d.glob('*.cpp')
            if any(matches_section_filter(p, section) for section in args.section)
        }
        missing_sections = [
            section
            for section in args.section
            if not any(matches_section_filter(p, section) for p in matching_paths)
        ]
        if missing_sections:
            parser.error(
                'no .cpp files found for section filter(s): '
                + ', '.join(missing_sections)
            )
        paths = sorted(matching_paths)
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
        ok, msg, warnings = compile_and_run(p, args.output, cxx)
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

    failed_sections = list(dict.fromkeys(
        section
        for rel, _ in failures
        if (section := section_key(rel)) is not None
    ))
    if failed_sections:
        print('\nRerun failed sections with:')
        print(f'  python3 run_examples.py -s {" ".join(failed_sections)}')

    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
