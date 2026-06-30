# AGENTS.md

Guidance for agents and contributors working in this repository. This project is a printable and
website-generated C++17 algorithm anthology, so every `.cpp` file is both source code and a
documentation artifact. Preserve that dual purpose: edits should make the implementation more
correct, more reusable, and easier to study under contest pressure.

## Project Shape

- Chapter directories are numbered (`1-Elementary-Algorithms`, `2-Data-Structures`, ...). Each
  section is usually one self-contained `.cpp` file.
- A section file is organized as documentation, reusable implementation, then example usage.
- The reusable portion ends at `/*** Example Usage ***/` or `/*** Example Usage and Output:`.
  Everything after that marker exists to compile, assert behavior, and optionally show output.
- The book and website are generated from these source files. Treat top-level and mid-file
  docstrings as public API documentation, not incidental comments.
- Do not regenerate the PDF/TeX/book unless the user explicitly asks. For normal code/doc edits,
  compile examples and keep the source files clean.

## Coding Philosophy

This is not a minified snippet collection. The style is contest-practical but deliberately readable.
Prefer code that a strong contestant can adapt quickly after skimming:

- Keep implementations self-contained and dependency-light.
- Favor clear names, explicit edge-case handling, and reusable interfaces over shortest possible
  code.
- Avoid unrelated abstractions, inheritance-heavy designs, or framework-like scaffolding.
- Do not keep compatibility wrappers or duplicate APIs unless they genuinely teach a variant.
- When two versions are useful, distinguish them by capability or assumptions, not by historical
  accident.
- Use assertions for preconditions that represent caller misuse; return sentinels or empty results
  for ordinary algorithmic failure when that is the section's established API.

## Formatting And Language

- Target ISO C++17 with the repository `.clang-format`:
  - Google base style
  - 2-space indentation
  - 100-column limit
  - right-aligned pointers
  - no automatic comment reflow
- Keep source ASCII unless a file already has a clear reason not to.
- Add only the headers required by pedantic C++ compilation. Remove unused includes when touching a
  file.
- In reusable code, use `std::` qualifications rather than `using namespace std;`. Example blocks
  may use `using namespace std;` when that matches the local style.
- Prefer `static_cast<int>(container.size())` before mixing sizes with `int` indices.
- Use `int` for ordinary indices and node IDs unless the algorithm needs a wider type.
- Use `int64_t`/`long long` for sums, weights, counts, and products that may overflow `int`.
- Keep comments sparse and useful. Explain invariants, tricky transitions, precision choices,
  overflow risks, and non-obvious contest assumptions; do not narrate obvious assignments.

## Naming Conventions

- Functions and variables use `snake_case`.
- Classes and structs use `CamelCase`.
- Template type parameters are conventional short names such as `T`, `U`, `C`, `Fn`, `Compare`.
- Runtime function parameters are lowercase, even when `const` and defaulted:
  `eps`, `iterations`, `maximize`, `edge_is_inside`, `touch_is_intersect`.
- Compile-time template knobs and named constants are uppercase:
  `VALUES_ON_EDGES`, `CANONICALIZE`, `EXACT`, `EPS`, `INF`, `NAIVE_CUTOFF`.
- For graph code, use `u` and `v` for edge endpoints. Use `w` or `weight` for edge weights, and
  avoid using `v` to mean a vertex value when it can be confused with endpoint `v`.
- For line/geometry examples involving two objects, prefer `l1`, `l2`, `p1`, `p2` over ambiguous
  names like `l` and `m`.
- For small two-item returns, prefer `std::pair` or structured bindings over a one-off struct when
  the meaning is obvious from the API bullet. Use `std::tuple` when returning three simple values.

## API Style

- Public APIs should be small, direct, and easy to paste into a solution.
- Do not add one-line forwarding wrappers just to preserve older names.
- If a function can naturally return the useful witness as well as the optimum value, prefer the
  more powerful API when it does not make the section heavy.
- Prefer returning result objects, pairs, tuples, vectors, or sentinels over output pointers for new
  APIs, unless the local section already strongly uses pointer outputs.
- For configurable data structures, expose the operations contestants actually customize:
  `combine`, `apply_delta`, `compose_deltas`, comparator types, identity values, or template flags.
- Template flags should be uppercase and reserved for compile-time behavior choices. Runtime flags
  should be lowercase function parameters.
- Keep generic code as generic as the implementation truly supports, but do not contort simple
  sections into concept-heavy code.

## Docstring Layout

Most algorithm sections begin with one leading `/* ... */` docstring using this four-zone shape:

1. Problem statement and algorithm insight. Lead with what is computed and why the algorithm works,
   not with iterator mechanics or parameter trivia.
2. One bullet per exposed function, constructor, class, or major operation.
3. Implementation-specific callouts after the API list: assumptions, overflow notes, multigraph
   support, precision behavior, optional variants, and cross-references.
4. `Time Complexity:` and `Space Complexity:` labelled blocks.

Utility grab-bags may instead use mid-file `/* Section: ... */` blocks with local API bullets. Do
not duplicate every bullet in the top docstring for those files; update the relevant local block.

Chapter 8 tooling sections may omit complexity blocks when they are not algorithms. For algorithmic
sections elsewhere, include both `Time Complexity:` and `Space Complexity:`.

## Docstring Voice And Math

- Keep prose concise but explanatory. A thin intro that only says what the function returns is not
  enough for a single-algorithm section; include the key invariant, recurrence, greedy choice, or
  structural idea.
- Use bullets like ``- `func(args)` returns ...`` for APIs. Include default parameters in the bullet
  when they are semantically relevant.
- Write big-O as plain text: `O(n log n)`, not `$O(n \log n)$`.
- Use single-letter variables inside big-O expressions. Quantify named parameters afterward, e.g.
  `O(r*c*2^c)`, where $r$ and $c$ are the number of rows and columns, and not `O(rows*cols*2^cols)`.
- In a complexity block for a section with only one exposed API, write `per call` without repeating
  the function signature. Keep names for callbacks or helper operations when they are the thing
  being counted, e.g. O(n) calls to the callback `pred()` per call.
- In space complexity, use plain `O(...) auxiliary` for ordinary temporary memory when there is
  no nearby stack-space comparison. If the same block also mentions recursive or call-stack memory,
  call the non-stack temporary memory `auxiliary heap space` so the two costs are distinct. Use
  `for storage of ...` or `for the returned ...` when describing persistent storage or output size.
- Use math mode for variables and inequalities in prose: `$n$`, `$\leq$`, `$\geq$`, `$-1$`.
- Code identifiers inside math expressions should remain backticked, e.g. $[`lo`, `hi`)$ or
  $0 \leq `i` < `n`$.
- Do not split math mode, code spans, or backticked identifiers just to satisfy the 100-column
  limit. A long equation or unbreakable expression over 100 columns is better than malformed TeX or
  confusing generated output.
- Literal operators and function names stay in backticks: `<=`, `operator<`, `combine()`.
- Prefer relative paths for cross-references to other sections.
- Reflow edited prose to the 100-column limit by hand; clang-format does not reflow comments. Do
  not leave very short continuation lines when the text would still fit on the previous line.

## Examples

- Every section should compile as a standalone program through `run_examples.py`.
- Keep example code after the marker. Include only what is needed to exercise the implementation.
- Prefer `assert`-based examples for behavior. Use printed output only when it teaches the result;
  then use `/*** Example Usage and Output:`.
- Examples may define helpers such as `print_range()` or `eval()` when they make the demonstration
  clearer.
- When examples rely on exact equality for floating-point-looking objects, either use `EQ()` or add
  a short comment explaining why exact equality is intentional.
- ASCII diagrams are welcome when they make tree/graph/data-structure updates easier to follow.
  Repeat diagrams across updates if that is clearer than describing the mutation in prose.

## Precision And Numeric Code

- Floating-point tolerance constants are usually named `EPS` and kept as uppercase constants.
- Runtime tolerance parameters are lowercase `eps`.
- Geometry `operator==` and `operator<` should remain exact so points and lines work predictably in
  standard containers; use `EQ()` overloads for approximate geometric equality.
- For templated numeric geometry, exact operations should preserve the coordinate type when
  possible. Metric operations that require square roots, trigonometry, or division should promote
  non-floating-point coordinates to `double`.
- Call out overflow and representation assumptions explicitly, especially for products, squared
  norms, modular arithmetic, big integers, and floating-point normalization.
- Avoid `std::is_integral_v<T>` as a proxy for "exact integer-like" when custom types such as
  `BigInt`, `Rational`, or `Modular` may be relevant. Prefer branching on floating-point behavior
  when the issue is really precision/metric support.

## Data Structures And Graphs

- Prefer one clear class with a simple public interface. Avoid inheritance unless it is already the
  local pattern and genuinely simplifies the code.
- Store graph edges with IDs when IDs matter; otherwise adjacency lists of vertices are fine.
- State whether graph helpers support directed graphs, undirected graphs, parallel edges, or
  multigraphs when that affects correctness.
- For tree/path data structures, be explicit about whether values live on nodes or edges and what
  the root convention is.
- Boundary-search helpers such as `max_right()` and `min_left()` should be documented once per
  section when included; if omitted because identical to an earlier section, say so in the prose.

## Verification

Before finishing meaningful edits, run checks scaled to the touched files:

- Format touched C++ files:
  `clang-format -i path/to/file.cpp`
- Verify formatting:
  `clang-format --dry-run --Werror path/to/file.cpp`
- Compile and run examples:
  `python3 run_examples.py -s 4.2.3`
  or pass explicit file paths.
- Check whitespace:
  `git diff --check -- path/to/file.cpp`
- Check line length for touched files:
  `awk 'length($0) > 100 { print FILENAME ":" FNR ":" length($0) ":" $0 }' path/to/file.cpp`

Warnings from the compiler should be investigated. If a warning is known and harmless, mention it
in the final report rather than hiding it.

## Editing Discipline

- Preserve unrelated user changes in the dirty worktree. Do not revert files unless explicitly
  asked.
- Use `rg` for searches and `apply_patch` for manual edits.
- Keep diffs scoped. Do not reformat or refactor unrelated sections just because you noticed them.
- When changing an API, update the docstring, examples, and all local callers in the same edit.
- When touching docstrings, preserve the generated-book style: 100 columns, ASCII, API bullets, and
  complexity blocks.
- If a change affects a broad convention, scan the repo for similar occurrences and either update
  them or call out why they are intentionally different.
