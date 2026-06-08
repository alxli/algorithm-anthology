#!/usr/bin/env python3
# Run this script to generate chapter*.tex with the latest code.

import os
import re
from os.path import abspath, dirname, isdir, join


PROJ_PATH = dirname(dirname(abspath(__file__)))
BOOK_PATH = os.path.join(PROJ_PATH, 'Book')
DIR_RE = re.compile(r'^(\d+)-(\S+)$')
FILE_RE = re.compile(r'^(\d+)\.(\d+)(?:\.(\d+))?_(\S+)\.cpp$')
BLOCK_COMMENT_RE = re.compile(r'/\*(.*?)\*/', re.DOTALL)
EXAMPLE_RE = re.compile(r'^\s*\*+\s*(Example Usage(?: and Output)?)\s*:?\s*',
                        re.DOTALL)
METADATA_RE = re.compile(
    r'^(Time Complexity(?: \([^)]+\))?|Space Complexity(?: \([^)]+\))?|'
    r'Stable\?):\s*(.*)$')
# Reserve a few lines before a bold heading so it is not stranded at a page
# bottom, away from the listing or list that follows. \Needspace* (capital,
# starred) only forces a break when there genuinely is not enough room; unlike
# lowercase \needspace it adds no stretchy glue, so under \raggedbottom it does
# not push the whole block to the next page when there is space.
KEEP_WITH_LISTING = '\\Needspace*{4\\baselineskip}\n'
KEEP_WITH_LIST = '\\Needspace*{3\\baselineskip}\n'
SECTION_NAMES = {
    (1, 1): 'Array Transformations',
    (1, 2): 'Arrays and Dynamic Programming',
    (1, 3): 'Greedy and Scheduling',
    (1, 4): 'Streaming and Randomized Algorithms',
    (1, 5): 'Cycle Detection',
    (1, 6): 'Bit Manipulation',
    (2, 1): 'Linked Lists',
    (2, 2): 'Heaps and Priority Queues',
    (2, 3): 'Dictionaries and Ordered Sets',
    (2, 4): 'Range Queries in One Dimension',
    (2, 5): 'Range Queries in Two Dimensions',
    (2, 6): 'Fenwick Trees',
    (2, 7): 'Disjoint Sets and Tree Structures',
    (3, 1): 'String Utilities',
    (3, 2): 'Hashing',
    (3, 3): 'Pattern Matching',
    (3, 4): 'Expression Parsing',
    (3, 5): 'Sequence Dynamic Programming',
    (3, 6): 'Suffix Arrays and LCP',
    (3, 7): 'String Data Structures',
    (3, 8): 'Encoding and Compression',
    (4, 1): 'DFS and Tree Algorithms',
    (4, 2): 'Connectivity',
    (4, 3): 'BFS and Shortest Paths',
    (4, 4): 'Spanning Trees',
    (4, 5): 'Flows and Cuts',
    (4, 6): 'Matching and Assignment',
    (4, 7): 'Exponential Graph Problems',
    (5, 1): 'Monotone Predicate Search',
    (5, 2): 'Unimodal and Continuous Search',
    (5, 3): 'Dynamic Programming Optimization',
    (5, 4): 'Numerical Methods',
    (6, 1): 'Math Utilities',
    (6, 2): 'Combinatorics',
    (6, 3): 'Number Theory',
    (6, 4): 'Arbitrary Precision Arithmetic',
    (6, 5): 'Linear Algebra',
    (6, 6): 'Polynomials',
    (6, 7): 'Game Theory',
    (7, 1): 'Geometry Primitives',
    (7, 2): 'Angles, Distances, and Intersections',
    (7, 3): 'Polygons and Planar Searches',
    (7, 4): 'Advanced Planar Geometry',
}


def escape_latex(text):
    replacements = {
        '\\': r'\textbackslash{}',
        '%': r'\%',
        '$': r'\$',
        '_': r'\_',
        '&': r'\&',
        '#': r'\#',
        '{': r'\{',
        '}': r'\}',
        '~': r'\textasciitilde{}',
        '^': r'\textasciicircum{}',
        '<': r'\textless{}',
        '>': r'\textgreater{}',
    }
    return ''.join(replacements.get(char, char) for char in text)


def escape_latex_code(text):
    replacements = {
        "'": r'\textquotesingle{}',
    }
    return ''.join(replacements.get(char, escape_latex(char)) for char in text)


def format_math_expression(expression):
    functions = {'alpha': r'\alpha', 'exp': r'\exp', 'log': r'\log', 'max': r'\max',
                 'min': r'\min'}
    constants = {'MOD': r'\text{MOD}'}
    result = []

    def format_word(word):
        subscript = re.fullmatch(r'([A-Za-z])([0-9]+)', word)
        if subscript is not None:
            return '{}_{{{}}}'.format(*subscript.groups())
        return constants.get(word, functions.get(word, word))

    i = 0
    while i < len(expression):
        match = re.match(r'[A-Za-z_][A-Za-z_0-9]*', expression[i:])
        if match is None:
            result.append(expression[i])
            i += 1
            continue
        word = match.group(0)
        i += len(word)
        if i < len(expression) and expression[i] == '(':
            depth = 1
            end = i + 1
            while end < len(expression) and depth > 0:
                if expression[end] == '(':
                    depth += 1
                elif expression[end] == ')':
                    depth -= 1
                end += 1
            if depth == 0:
                argument = format_math_expression(expression[i + 1:end - 1])
                if word == 'sqrt':
                    result.append(r'\sqrt{' + argument + '}')
                elif word in functions:
                    result.append(functions[word] + r'\left(' + argument +
                                  r'\right)')
                else:
                    result.append(word + '(' + argument + ')')
                i = end
                continue
        if word == 'sqrt':
            bare_arg = re.match(r'\s+([A-Za-z_0-9.]+)', expression[i:])
            if bare_arg is not None:
                result.append(r'\sqrt{' + format_math_expression(bare_arg.group(1)) + '}')
                i += bare_arg.end()
                continue
        result.append(format_word(word))
    return ''.join(result)


def format_complexity(expression):
    expression = format_math_expression(expression)
    expression = re.sub(r'\^\(([^()]*)\)', r'^{\1}', expression)
    expression = re.sub(r'\^([A-Za-z0-9.]+)', r'^{\1}', expression)
    expression = re.sub(r'\s*\*\s*', r' \\cdot ', expression)
    return '$O({})$'.format(expression)


def escape_prose(text, prev_char):
    return escape_latex(curl_quotes(text, prev_char))


def format_prose(text, prev_char=''):
    parts = []
    pos = 0
    while True:
        match = re.search(
            r'<=|>=|(?<![A-Za-z0-9_])\*\*(?=[A-Za-z])|'
            r'(?<![A-Za-z0-9_])\*(?=[A-Za-z])',
            text[pos:])
        if match is None:
            parts.append(escape_prose(text[pos:], text[pos - 1] if pos else prev_char))
            break
        start = pos + match.start()
        delimiter = match.group(0)
        parts.append(escape_prose(text[pos:start], text[pos - 1] if pos else prev_char))
        if delimiter == '<=':
            parts.append(r'$\leq$')
            pos = start + 2
        elif delimiter == '>=':
            parts.append(r'$\geq$')
            pos = start + 2
        else:
            end_match = re.search(
                re.escape(delimiter) + r'(?![A-Za-z0-9_])',
                text[start + len(delimiter):])
            if end_match is None:
                parts.append(escape_prose(text[start:], text[start - 1] if start else prev_char))
                break
            end = start + len(delimiter) + end_match.start()
            # The emphasis marker is a boundary, so quotes inside it open fresh.
            contents = format_prose(text[start + len(delimiter):end])
            command = 'textbf' if delimiter == '**' else 'textit'
            parts.append('\\{}{{{}}}'.format(command, contents))
            pos = end + len(delimiter)
    return ''.join(parts)


def format_plain_text(text, prev_char=''):
    parts = []
    pos = 0
    while True:
        match = re.search(r'\bO\(', text[pos:])
        if match is None:
            parts.append(format_prose(text[pos:], text[pos - 1] if pos else prev_char))
            break
        start = pos + match.start()
        depth = 0
        end = start + 1
        while end < len(text):
            if text[end] == '(':
                depth += 1
            elif text[end] == ')':
                depth -= 1
                if depth == 0:
                    end += 1
                    break
            end += 1
        if depth != 0:
            parts.append(format_prose(text[pos:], text[pos - 1] if pos else prev_char))
            break
        parts.append(format_prose(text[pos:start], text[pos - 1] if pos else prev_char))
        parts.append(format_complexity(text[start + 2:end - 1]))
        pos = end
    return ''.join(parts)


def curl_quotes(text, prev_char=''):
    """Turn straight quotes in prose into directional LaTeX quote markup.

    Only prose reaches here: format_text splits out inline code and math
    first, so every quote in `text` is prosaic and safe to curl. `prev_char`
    is the original character immediately preceding `text` (for example the
    closing backtick of a code span), which lets a leading quote resolve as
    opening or closing across the span boundary.
    """
    result = []
    n = len(text)
    for i, char in enumerate(text):
        if char != '"' and char != "'":
            result.append(char)
            continue
        j = i - 1
        while j >= 0 and text[j] == '*':  # emphasis markers act as boundaries
            j -= 1
        before = text[j] if j >= 0 else prev_char
        opening = before == '' or before.isspace() or before in '([{'
        if char == '"':
            result.append('``' if opening else "''")
        else:
            following = text[i + 1] if i + 1 < n else ''
            # A leading apostrophe marking elision ('90s) is still a right quote.
            result.append('`' if opening and not following.isdigit() else "'")
    return ''.join(result)


def format_text(text):
    parts = []
    pos = 0
    while pos < len(text):
        match = re.search(r'[`$]', text[pos:])
        if match is None:
            parts.append(format_plain_text(text[pos:], text[pos - 1] if pos else ''))
            break
        start = pos + match.start()
        delimiter = text[start]
        end = text.find(delimiter, start + 1)
        if end == -1:
            parts.append(format_plain_text(text[pos:], text[pos - 1] if pos else ''))
            break
        parts.append(format_plain_text(text[pos:start], text[pos - 1] if pos else ''))
        contents = text[start + 1:end]
        if delimiter == '`':
            parts.append(r'\inlinecode{' + escape_latex_code(contents) + '}')
        else:
            parts.append('$' + contents + '$')
        pos = end + 1
    return ''.join(parts)


def clean_comment_lines(comment):
    lines = []
    for line in comment.splitlines():
        line = re.sub(r'^\s*\* ?', '', line.rstrip())
        lines.append(line)
    while lines and not lines[0]:
        lines.pop(0)
    while lines and not lines[-1]:
        lines.pop()
    return lines


def render_comment(comment):
    lines = clean_comment_lines(comment)
    result = []
    paragraph = []
    in_list = False

    def flush_paragraph():
        if paragraph:
            result.append(format_text(' '.join(paragraph)) + '\n\n')
            paragraph.clear()

    def close_list():
        nonlocal in_list
        if in_list:
            result.append('\\end{compactitem}\n\n')
            in_list = False

    def read_item(index):
        item = [lines[index].strip()[2:]]
        while index + 1 < len(lines):
            next_line = lines[index + 1].strip()
            if (not next_line or next_line.startswith('- ') or
                    METADATA_RE.match(next_line)):
                break
            item.append(next_line)
            index += 1
        return ' '.join(item), index

    i = 0
    while i < len(lines):
        line = lines[i].strip()
        metadata = METADATA_RE.match(line)
        if not line:
            flush_paragraph()
            close_list()
        elif metadata:
            flush_paragraph()
            close_list()
            label, value = metadata.groups()
            if (not value and label in ('Time Complexity', 'Space Complexity')
                    and i + 1 < len(lines) and
                    lines[i + 1].strip().startswith('- ')):
                item, end = read_item(i + 1)
                next_line = lines[end + 1].strip() if end + 1 < len(lines) else ''
                if not next_line.startswith('- '):
                    value = item
                    i = end
            if value:
                result.append('\\textbf{{{}}}: {}\n\n'.format(
                    escape_latex(label), format_text(value)))
            else:
                # A label with no inline value is followed by a list; reserve
                # space so the heading is not orphaned at a page bottom.
                result.append('{}\\textbf{{{}}}:\n'.format(
                    KEEP_WITH_LIST, escape_latex(label)))
        elif line.startswith('- '):
            flush_paragraph()
            if not in_list:
                result.append('\\begin{compactitem}\n')
                in_list = True
            item, i = read_item(i)
            result.append('  \\item {}\n'.format(format_text(item)))
        else:
            paragraph.append(line)
        i += 1
    flush_paragraph()
    close_list()
    return ''.join(result)


def render_listing(text, language='C++'):
    text = text.strip('\n')
    if not text.strip():
        return ''
    text = '\n'.join(
        line.rstrip() for line in text.splitlines()
        if not re.match(r'\s*//\s*clang-format\s+(off|on)\s*$', line)
    )
    options = 'language={}'.format(language) if language else 'language={},numbers=none'
    return '\\begin{{lstlisting}}[{}]\n{}\n\\end{{lstlisting}}\n'.format(
        options, text)


def parse_example(comment):
    match = EXAMPLE_RE.match(comment)
    assert match is not None
    heading = match.group(1)
    output = re.sub(r'\s*\*+\s*$', '', comment[match.end():]).strip('\n')
    return heading, output


def render_output(output):
    output = '\n'.join(line.rstrip() for line in output.splitlines())
    return ('\\begin{{exampleoutput}}\n'
            '{}\n'
            '\\end{{exampleoutput}}\n\n').format(output)


def render_source(source):
    result = []
    pos = 0
    pending_output = None

    def append_listing(text):
        nonlocal pending_output
        listing = render_listing(text)
        if listing:
            result.append(listing)
            if pending_output is not None:
                result.append(render_output(pending_output))
                pending_output = None

    for match in BLOCK_COMMENT_RE.finditer(source):
        append_listing(source[pos:match.start()])
        comment = match.group(1)
        if EXAMPLE_RE.match(comment):
            heading, output = parse_example(comment)
            result.append(KEEP_WITH_LISTING + '\\textbf{Example Usage}\\par\n')
            if heading == 'Example Usage and Output' and output.strip():
                pending_output = output
        elif re.match(r'^\s*\*+\s*Wrapper\s*\*+\s*$', comment):
            result.append(KEEP_WITH_LISTING + '\\textbf{Wrapper:}\\par\n')
        else:
            result.append(render_comment(comment))
        pos = match.end()
    append_listing(source[pos:])
    if pending_output is not None:
        result.append(render_output(pending_output))
    return ''.join(result).rstrip() + '\n'


def gen_chapter(dirname, chapter, chapter_name):
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

    with open(join(BOOK_PATH, f'chapter{chapter}.tex'), 'w') as fout:
        fout.write('\\chapter{{{}}}\n'.format(chapter_name.replace('-', ' ')))

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

            with open(join(dirname, file), 'r') as srcfile:
                fout.write(render_source(srcfile.read()))
            prev_section = section


if __name__ == '__main__':
    os.chdir(PROJ_PATH)
    for file in os.listdir('.'):
        match = DIR_RE.match(file)
        if isdir(file) and match:
            chapter = int(match.group(1))
            chapter_name = match.group(2)
            gen_chapter(file, chapter, chapter_name)
