#!/usr/bin/env python
# Run this script to generate chapter*.tex with the latest code.

import os
import re
from os.path import abspath, dirname, isdir, join


PROJ_PATH = dirname(dirname(abspath(__file__)))
BOOK_PATH = os.path.join(PROJ_PATH, 'Book')
DIR_RE = re.compile(r'^(\d+)-(\S+)$')
FILE_RE = re.compile(r'^(\d+)\.(\d+)(?:\.(\d+))?_(\S+)\.cpp$')
SECTION_NAMES = {
    (1, 1): 'Array Transformations',
    (1, 2): 'Array Queries',
    (1, 3): 'Searching',
    (1, 4): 'Cycle Detection',
    (2, 1): 'Heaps',
    (2, 2): 'Dictionaries',
    (2, 3): 'Range Queries in One Dimension',
    (2, 4): 'Range Queries in Two Dimensions',
    (2, 5): 'Fenwick Trees',
    (2, 6): 'Tree Data Structures',
    (3, 1): 'String Utilities',
    (3, 3): 'String Searching',
    (3, 2): 'Expression Parsing',
    (3, 4): 'Dynamic Programming',
    (3, 5): 'Suffix Array and LCP',
    (3, 6): 'String Data Structures',
    (4, 1): 'Depth-First Search',
    (4, 2): 'Shortest Path',
    (4, 3): 'Connectivity',
    (4, 4): 'Minimum Spanning Tree',
    (4, 5): 'Maximum Flow',
    (4, 6): 'Maximum Matching',
    (4, 7): 'Hard Problems',
    (5, 1): 'Math Utilities',
    (5, 2): 'Combinatorics',
    (5, 3): 'Number Theory',
    (5, 4): 'Arbitrary Precision Arithmetic',
    (5, 5): 'Linear Algebra',
    (5, 6): 'Root Finding and Calculus',
    (6, 1): 'Geometric Classes',
    (6, 2): 'Elementary Geometric Calculations',
    (6, 3): 'Intermediate Geometric Calculations',
    (6, 4): 'Advanced Geometric Computations',
}


def gen_chapter(dirname, chapter, chapter_name):
    fout = open(join(BOOK_PATH, f'chapter{chapter}.tex'), 'w')
    fout.write('\\chapter{{{}}}\n'.format(chapter_name.replace('-', ' ')))

    entries = []

    for file in os.listdir(dirname):
        match = FILE_RE.match(file)
        if not match:
            continue
        assert chapter == int(match.group(1))
        section = int(match.group(2))
        subsection = int(match.group(3)) if match.group(3) else None
        name = match.group(4)
        entry = (chapter, section, subsection, name, file)
        entries.append(entry)

    prev_section = None
    for (chapter, section, subsection, name, file) in sorted(entries):
        if section < 1:
            continue
        if section != prev_section:
            section_name = SECTION_NAMES.get((chapter, section), "")
            fout.write(f'\n\\section{{{section_name}}}\n')
            fout.write(f'\\setcounter{{section}}{{{section}}}\n')
            if subsection:
                fout.write(f'\\setcounter{{subsection}}{{0}}\n')
        if subsection:
            fout.write('\\subsection{{{}}}\n'.format(name.replace('_', ' ')))

        fout.write("\\begin{lstlisting}\n")
        with open(join(dirname, file), 'r') as srcfile:
            fout.write(srcfile.read())
        fout.write("\\end{lstlisting}\n")
        prev_section = section

    fout.close()


if __name__ == '__main__':
    os.chdir(PROJ_PATH)
    for file in os.listdir('.'):
        match = DIR_RE.match(file)
        if isdir(file) and match:
            chapter = int(match.group(1))
            chapter_name = match.group(2)
            gen_chapter(file, chapter, chapter_name)
