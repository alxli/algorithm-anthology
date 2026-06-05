import unittest

import gen_chapters


class GeneratorTest(unittest.TestCase):
    def test_escape_latex_special_characters(self):
        text = r'\ % $ _ & # { } ~ ^ < >'
        self.assertEqual(
            gen_chapters.escape_latex(text),
            r'\textbackslash{} \% \$ \_ \& \# \{ \} '
            r'\textasciitilde{} \textasciicircum{} \textless{} \textgreater{}')

    def test_inline_code_escapes_nested_template_brackets(self):
        self.assertEqual(
            gen_chapters.format_text(
                r'Use `std::unordered_set<std::pair<int, int>>`.'),
            r'Use \inlinecode{std::unordered\_set\textless{}std::pair'
            r'\textless{}int, int\textgreater{}\textgreater{}}.')

    def test_inline_code_uses_straight_single_quotes(self):
        self.assertEqual(
            gen_chapters.format_text("Use `s[i] == '\\0'`, then don't panic."),
            r'Use \inlinecode{s[i] == \textquotesingle{}\textbackslash{}0'
            r'\textquotesingle{}}, then don'
            r"'t panic.")

    def test_prose_double_quotes_become_directional(self):
        self.assertEqual(
            gen_chapters.format_text('Updates that "set" a path to a value.'),
            "Updates that ``set'' a path to a value.")

    def test_prose_apostrophes_and_possessives_stay_right_quotes(self):
        self.assertEqual(
            gen_chapters.format_text("Dijkstra's and the iterators' value type."),
            "Dijkstra's and the iterators' value type.")
        self.assertEqual(gen_chapters.format_text("only 0's and 1's"),
                         "only 0's and 1's")

    def test_prose_single_quoted_phrase_becomes_directional(self):
        self.assertEqual(gen_chapters.format_text("the 'word' here"),
                         "the `word' here")

    def test_quote_after_inline_code_closes(self):
        self.assertEqual(
            gen_chapters.format_text('apply `d`". Done.'),
            "apply \\inlinecode{d}''. Done.")

    def test_possessive_after_inline_code_stays_right_quote(self):
        self.assertEqual(
            gen_chapters.format_text("reference to key `k`'s associated value"),
            "reference to key \\inlinecode{k}'s associated value")

    def test_quotes_inside_inline_code_are_untouched(self):
        self.assertEqual(
            gen_chapters.format_text("a `'0'` bit and \"min\" mode"),
            'a \\inlinecode{\\textquotesingle{}0\\textquotesingle{}} bit '
            "and ``min'' mode")

    def test_quotes_inside_emphasis_resolve_at_boundary(self):
        self.assertEqual(
            gen_chapters.format_text('a **the "set" case** here'),
            "a \\textbf{the ``set'' case} here")

    def test_render_comment(self):
        comment = r'''
Soft-wrapped text with a 50% chance and path C:\tmp.
This line belongs to the same paragraph.

Time Complexity (Average): O(n log n).
Stable?: No.

- O(1) for setup.
- O(n^2) for work with foo_bar.
'''
        self.assertEqual(
            gen_chapters.render_comment(comment),
            'Soft-wrapped text with a 50\\% chance and path '
            'C:\\textbackslash{}tmp. This line belongs to the same paragraph.'
            '\n\n'
            '\\textbf{Time Complexity (Average)}: $O(n \\log n)$.\n\n'
            '\\textbf{Stable?}: No.\n\n'
            '\\begin{compactitem}\n'
            '  \\item $O(1)$ for setup.\n'
            '  \\item $O(n^{2})$ for work with foo\\_bar.\n'
            '\\end{compactitem}\n\n')

    def test_multiline_complexity_label_is_kept_with_its_list(self):
        rendered = gen_chapters.render_comment('''
Time Complexity:
- O(n) to build.
- O(log n) per query.
''')
        self.assertEqual(
            rendered,
            '\\Needspace*{3\\baselineskip}\n\\textbf{Time Complexity}:\n'
            '\\begin{compactitem}\n'
            '  \\item $O(n)$ to build.\n'
            '  \\item $O(\\log n)$ per query.\n'
            '\\end{compactitem}\n\n')

    def test_inline_complexity_label_does_not_force_keep(self):
        rendered = gen_chapters.render_comment('Time Complexity: O(n log n).')
        self.assertNotIn('Needspace', rendered)

    def test_example_usage_heading_is_kept_with_listing(self):
        rendered = gen_chapters.render_source('''/*** Example Usage ***/
int main() {}
''')
        self.assertIn('\\Needspace*{4\\baselineskip}\n\\textbf{Example Usage}\\par',
                      rendered)

    def test_single_complexity_item_is_inlined(self):
        comment = '''
Time Complexity:
- O(n log n) per call.

Space Complexity:
- O(1) auxiliary.
'''
        self.assertEqual(
            gen_chapters.render_comment(comment),
            '\\textbf{Time Complexity}: $O(n \\log n)$ per call.\n\n'
            '\\textbf{Space Complexity}: $O(1)$ auxiliary.\n\n')

    def test_format_complexity_square_roots(self):
        self.assertEqual(gen_chapters.format_complexity('sqrt n'),
                         r'$O(\sqrt{n})$')
        self.assertEqual(
            gen_chapters.format_complexity('sqrt(max(MAXR, MAXC))'),
            r'$O(\sqrt{\max\left(\text{MAXR}, \text{MAXC}\right)})$')
        self.assertEqual(
            gen_chapters.format_complexity('m*sqrt(n1 + n2)'),
            r'$O(m \cdot \sqrt{n_{1} + n_{2}})$')

    def test_format_complexity_numeric_subscripts(self):
        self.assertEqual(gen_chapters.format_complexity('n1 + n2'),
                         r'$O(n_{1} + n_{2})$')

    def test_format_complexity_normalizes_multiplication_spacing(self):
        self.assertEqual(gen_chapters.format_complexity('2^n * n^2'),
                         r'$O(2^{n} \cdot n^{2})$')

    def test_format_complexity_inverse_ackermann(self):
        self.assertEqual(gen_chapters.format_complexity('alpha(n)'),
                         r'$O(\alpha\left(n\right))$')

    def test_inline_code_and_math(self):
        self.assertEqual(
            gen_chapters.format_text(
                r'Call `sort(a, a + n)` for $0 \leq i < n$ in O(n log n).'),
            r'Call \inlinecode{sort(a, a + n)} for $0 \leq i < n$ in '
            r'$O(n \log n)$.')

    def test_prose_comparisons_and_emphasis(self):
        self.assertEqual(
            gen_chapters.format_text(
                'Require `lo` <= `mid` <= `hi`, *usually* but **not always**.'),
            r'Require \inlinecode{lo} $\leq$ \inlinecode{mid} $\leq$ '
            r'\inlinecode{hi}, \textit{usually} but \textbf{not always}.')
        self.assertEqual(
            gen_chapters.format_text('Compute a*x + b*y and operators *, *=.'),
            r'Compute a*x + b*y and operators *, *=.')

    def test_unmatched_inline_delimiter_is_literal(self):
        self.assertEqual(gen_chapters.format_text('Use `foo_bar as written.'),
                         r'Use `foo\_bar as written.')

    def test_render_source_interleaves_prose_output_and_code(self):
        source = '''/*
Description.
*/
int answer = 42;
/*** Example Usage and Output:

Answer: 42   

***/
int main() {}
'''
        rendered = gen_chapters.render_source(source)
        self.assertIn('Description.\n\n', rendered)
        self.assertIn('\\begin{lstlisting}[language=C++]\n'
                      'int answer = 42;\n\\end{lstlisting}', rendered)
        self.assertIn('\\textbf{Example Usage}\\par', rendered)
        self.assertIn('\\begin{lstlisting}[language=C++]\n'
                      'int main() {}\n\\end{lstlisting}', rendered)
        self.assertIn('\\begin{exampleoutput}\n'
                      'Answer: 42\n'
                      '\\end{exampleoutput}', rendered)
        self.assertLess(rendered.index('int main() {}'),
                        rendered.index('\\begin{exampleoutput}'))

    def test_usage_only_example_has_no_output_box(self):
        rendered = gen_chapters.render_source('''/*** Example Usage ***/
int main() {}
''')
        self.assertIn('\\textbf{Example Usage}\\par', rendered)
        self.assertNotIn('Example Output', rendered)

    def test_only_allowlisted_labels_become_metadata(self):
        rendered = gen_chapters.render_comment('A normal sentence: still prose.')
        self.assertEqual(rendered, 'A normal sentence: still prose.\n\n')


if __name__ == '__main__':
    unittest.main()
