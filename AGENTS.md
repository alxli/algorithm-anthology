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

## Book Builds And Releases

- Run `make -C Book pdf` to regenerate `chapter*.tex` and the canonical `Book/A3C5.pdf` using the
  default `VERSION` and `RELEASE_DATE` in `Book/Makefile`.
- Run `make -C Book release VERSION=v1.1 RELEASE_DATE='October 1, 2026'` for a release. This injects
  the version and date into the title page, updates the canonical PDF, creates
  `Book/A3C5-v1.1.pdf`, and updates the versioned PDF link in `README.md`.
- `Book/A3C5.pdf`, versioned `Book/A3C5-v*.pdf` files, and the generated `chapter*.tex` files are
  tracked. The canonical PDF serves the evergreen URL, while each versioned PDF preserves a stable
  raw GitHub URL and download filename.
- After a release build, verify the title page and commit the canonical PDF, versioned PDF, generated
  TeX files, README link, and build metadata together.

## Coding Philosophy

This is not a minified snippet collection. The style is contest-practical but deliberately readable.
Prefer code that a strong contestant can adapt quickly after skimming:

- Keep implementations self-contained and dependency-light.
- Favor clear names, explicit edge-case handling, and reusable interfaces over shortest possible
  code.
- Avoid unrelated abstractions, inheritance-heavy designs, or framework-like scaffolding.
- Prefer established anthology idioms over isolated STL-generic machinery. Before introducing type
  aliases, traits, forwarding patterns, or additional template parameters, check comparable
  sections first; use them only when they provide a capability the algorithm genuinely needs.
- For common algorithms, preserve the concise, canonical code structure that experienced readers
  expect. Do not replace a standard idiom with a more elaborate or superficially optimized version
  merely to tighten a secondary bound; recognizability, adaptability, and educational value are
  primary goals of this anthology. Deviate when there is a concrete correctness or capability
  benefit, and keep the reason apparent in the code or documentation.
- Do not keep compatibility wrappers or duplicate APIs unless they genuinely teach a variant.
- When two versions are useful, distinguish them by capability or assumptions, not by historical
  accident.
- Use assertions for preconditions that represent caller misuse; return sentinels or empty results
  for ordinary algorithmic failure when that is the section's established API.
- Never put a required side effect inside `assert()`, since defining `NDEBUG` removes the entire
  expression.
- Mark single-argument constructors `explicit` unless implicit conversion is an intentional part of
  a mathematical value type or another established API.
- Delete copying for classes whose buffered or resource-owning state cannot be copied meaningfully.
- Assume nested-vector matrices and grids are rectangular; do not scan every row with assertions to
  validate their shape. Keep assertions for compatibility between separate inputs, such as matrix
  dimensions matching a vector or assignment output.

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
  use `using namespace std;` by default; keep explicit `std::` only where the contrast carries
  meaning, as in 8.6 alongside `__gnu_pbds`.
- Prefer `static_cast<int>(container.size())` before mixing sizes with `int` indices.
- For read-only structured bindings of small pairs or tuples, prefer copying with `auto [...]`;
  use `const auto &` when copying the bound object would be material.
- Prefer a self-passing generic lambda for local recursion in C++17, especially when a file-scope
  helper exists only to thread the caller's locals through by-reference or pointer parameters. Keep
  a free function when the recursion reads only globals and value parameters and its shape is a
  canonical idiom readers expect, such as the augmenting-path `dfs` in the matching sections. Use
  `std::function` only when type erasure or a stored recursive callable is actually needed.
- Use `int` for ordinary sizes, indices, iterator distances, and node IDs unless the algorithm needs
  a wider type. Do not introduce iterator `difference_type` aliases merely for generic formality.
- Adjacent one-line function definitions may remain together. Surround every multiline function
  definition with one empty line, except where an access label or enclosing brace forms the
  boundary.
- Preserve code between `clang-format off` and `clang-format on` markers exactly unless the task
  specifically requires changing it. Such regions intentionally override ordinary formatting and
  line-length rules, including the 100-column limit.
- Use `int64_t`/`long long` for sums, weights, counts, and products that may overflow `int`.
- Use unsigned types for bit masks whose highest bit may be used; signed shifts, negation, and
  successor-mask arithmetic can otherwise invoke undefined behavior.
- Check intermediate arithmetic as well as the final mathematical result for overflow. Rearrange or
  reduce products before division when an intermediate can exceed the result type.
- Keep comments sparse and useful. Explain invariants, tricky transitions, precision choices,
  overflow risks, and non-obvious contest assumptions; do not narrate obvious assignments.

## Naming Conventions

- Functions and variables use `snake_case`.
- Classes and structs use `CamelCase`.
- Template type parameters are conventional short names such as `T`, `U`, `C`, `Fn`, `Compare`.
- Runtime function parameters are lowercase, even when `const` and defaulted:
  `eps`, `iterations`, `maximize`, `include_boundary`.
- Prefer `lo` and `hi` for generic iterator, search, and numeric range bounds. Use more specific
  names such as `tgt_lo`/`tgt_hi` when two ranges coexist. Preserve established domain terms where
  they carry meaning, including scheduling `start`/`finish`, graph `start`, matrix `r`/`c`,
  and STL-style `first`/`last`.
- Compile-time template knobs and named constants are uppercase:
  `VALUES_ON_EDGES`, `CANONICALIZE`, `EXACT`, `EPS`, `INF`, `NAIVE_CUTOFF`.
- For graph code, use `u` and `v` for edge endpoints. Use `w` or `weight` for edge weights, and
  avoid using `v` to mean a vertex value when it can be confused with endpoint `v`.
- For line/geometry examples involving two objects, prefer `l1`, `l2`, `p1`, `p2` over ambiguous
  names like `l` and `m`.
- Avoid bare eponym-only public API names when the operation is not universally recognized at a call
  site. Prefer names that include the computed object or action, such as `frequent_candidates()`,
  `maximum_matching()`, `global_min_cut()`, or `min_assignment_cost()`. Eponyms are fine when they
  are standard search terms, when the function name also names the result, or when they distinguish
  variants, e.g. `dijkstra()`, `smawk_row_minima()`, and `find_cycle_floyd()`.
- Standalone sections may reuse canonical class names when they provide interchangeable
  implementations or conventional domain types. When a variant exposes materially different public
  operations, give its class a distinguishing name; do not rename internal helpers merely to make
  every top-level name globally unique.
- For small two-item returns, prefer `std::pair` or structured bindings over a one-off struct when
  the meaning is obvious from the API bullet. Use `std::tuple` when returning three simple values.

## API Style

- Public APIs should be small, direct, and easy to paste into a solution.
- Do not add one-line forwarding wrappers just to preserve older names.
- If a function can naturally return the useful witness as well as the optimum value, prefer the
  more powerful API when it does not make the section heavy.
- Prefer returning result objects, pairs, tuples, vectors, or sentinels over output pointers for new
  APIs, unless the local section already strongly uses pointer outputs.
- Use pointers for secondary output parameters so mutation remains explicit at the call site, even
  when the output is required. Use `nullptr` only when suppressing that output is supported; keep
  references for a function's primary in-place argument.
- On failure, leave output arguments unchanged: compute into a local and commit it only once the
  operation is known to succeed, and state that guarantee in the API bullet.
- A streaming `operator<<` that changes format state (`showpos`, `setprecision`, `setfill`, ...)
  must restore it before returning; save `flags()`, `precision()`, or `fill()` and put them back.
  `setw` is one-shot and needs no restore.
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
- Keep API bullets synchronized with the callable public signature: include meaningful template
  parameters, current type names, and defaulted runtime arguments exactly as users should write
  them. Parameter names in explicit API signatures must match the code. When they differ, normally
  update the documentation; rename a code parameter only when its existing name is independently
  unclear or inconsistent, not merely to satisfy the scanner.
- Write big-O as plain text: `O(n log n)`, not `$O(n \log n)$`.
- Use single-letter variables inside big-O expressions. Quantify named parameters afterward, e.g.
  `O(r*c*2^c)`, where $r$ and $c$ are the number of rows and columns, and not `O(rows*cols*2^cols)`.
- Keep function-style parentheses when they make plain-text big-O clearer, e.g.
  `O(log(R)*log(C))`, `O(sqrt(n))`, and `O(max(n, m))`.
- For ranges or numeric search intervals, prefer naming the span with one letter in big-O, e.g.
  `O(log n)`, where $n$ is the distance between `lo` and `hi`, rather than `O(log(hi - lo))`.
  In the rare cases where endpoint names appear directly inside a math expression, keep code
  identifiers in backticks so they render as texttt/code.
- Scope callable time bounds with `per call`. In a section with only one exposed API, write plain
  `per call` without repeating the function signature; with multiple APIs, write "per call to
  `foo()` and `bar()`". Use the actual unit instead for non-call aggregates, e.g. `per operation`,
  `for construction`, `in total`, `for any sequence of ...`, or `on first use`.
- Keep names for callbacks or helper operations when they are the thing being counted, e.g. O(n)
  calls to the callback `pred()` per call.
- In complexity blocks, omit function arguments by default. Keep them when they distinguish
  overloads or variants with different bounds, or when a later clause uses an argument to define a
  complexity variable, e.g. ``O(n) per call to `find(s)`, where $n$ is the length of `s`.`` Do not
  repeat arguments merely to mirror the API bullet.
- In space complexity, use plain `O(...) auxiliary` for ordinary temporary memory when there is
  no nearby stack-space comparison. If the same block also mentions recursive or call-stack memory,
  call the non-stack temporary memory `auxiliary heap space` so the two costs are distinct. Use
  `for storage of ...` or `for the returned ...` when describing persistent storage or output size.
  Do not add `per call` to ordinary auxiliary-space bullets; name the relevant function when the
  section contains multiple APIs with different space bounds.
- Use math mode for variables and inequalities in prose: `$n$`, `$\leq$`, `$\geq$`, `$-1$`.
- Prefer half-open interval notation for numbered domains, e.g. nodes in `$[0, n)$`, instead of
  spelling out "from $0$ to `$n - 1`".
- State whether returned endpoint pairs are inclusive or half-open, and write numeric interval
  literals in math mode.
- Code identifiers inside math expressions should remain backticked, e.g. $[`lo`, `hi`)$ or
  $0 \leq `i` < `n`$.
- Do not split math mode, code spans, or backticked identifiers just to satisfy the 100-column
  limit. A long equation or unbreakable expression over 100 columns is better than malformed TeX or
  confusing generated output.
- For ordinary prose, wrap greedily word-by-word up to the 100-column limit. Do not preserve
  subjective phrase units across line breaks; only math mode, code spans, backticked identifiers,
  and plain-text Big-O expressions are atomic.
- Literal operators and function names stay in backticks: `<=`, `operator<`, `combine()`.
- Prefer relative paths for cross-references to other sections.
- Reflow edited prose to the 100-column limit by hand; clang-format does not reflow comments. Do
  not leave very short continuation lines when the text would still fit on the previous line.

## Examples

- Every section should compile as a standalone program through `run_examples.py`.
- Keep example code after the marker. Include only what is needed to exercise the implementation.
- Prefer `assert`-based examples for behavior, including exact container comparisons when the
  returned sequence is compact. Use printed output only when its presentation teaches something,
  such as timings, a matrix or alignment, an encoded representation, an enumeration, or debugging
  behavior; then use `/*** Example Usage and Output:`.
- Examples may define helpers such as `print_range()` or `eval()` when they make the demonstration
  clearer.
- Examples should exercise the behavior a contestant is likely to misunderstand: degenerate inputs,
  disconnected components, non-default flags, node-vs-edge conventions, or the witness returned by
  the API.
- Seed random engines in examples and tests with the fixed seed `1234567` so failures and output
  are reproducible. Reusable randomized implementations should instead seed once from
  `std::random_device{}()`, unless the API accepts a caller-provided seed or engine. Per-run
  steady-clock salts used to harden hash tables are a separate intentional convention.
- When examples rely on exact equality for floating-point-looking objects, either use `EQ()` or add
  a short comment explaining why exact equality is intentional.
- ASCII diagrams are welcome when they make tree/graph/data-structure updates easier to follow.
  Repeat diagrams across updates if that is clearer than describing the mutation in prose.

## Precision And Numeric Code

- Use plain `fabs()` rather than `std::fabs()` to match the anthology's compact convention for
  `<cmath>` functions. In the templated `EQ()` and `LT()` helpers, use `EPS` directly instead of
  `static_cast<C>(EPS)`; the floating-point expression converts it implicitly.
- Floating-point tolerance constants are usually named `EPS` and kept as uppercase constants.
- Runtime tolerance parameters are lowercase `eps`.
- Use `include_boundary` for geometry flags that control whether boundary points or boundary-only
  contact count; use the same name for containment and intersection APIs.
- Geometry `operator==` and `operator<` should remain exact so points and lines work predictably in
  standard containers; use `EQ()` overloads for approximate geometric equality.
- For templated numeric geometry, exact operations should preserve the coordinate type when
  possible. Metric operations that require square roots, trigonometry, or division should promote
  non-floating-point coordinates to `double`.
- Call out overflow and representation assumptions explicitly, especially for products, squared
  norms, modular arithmetic, big integers, and floating-point normalization. Lead such a docstring
  note with `Overflow warning:` so it stays greppable; the in-code comment form is
  `// Overflow warning.`.
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
- Check compiler warnings:
  `clang++ -std=c++17 -fsyntax-only -Wall -Wextra path/to/file.cpp`
- Check docstring wrapping and run the bounded stabilization audit, both when touching comments and
  before declaring the repo stable:
  `python3 scan_quality.py`

Warnings from the compiler should be investigated. If a warning is known and harmless, mention it
in the final report rather than hiding it.

## Stabilization Scans

Use two different modes for quality work:

- Stabilization scan: a bounded audit against existing policy. Run `python3 scan_quality.py`,
  inspect only findings above the threshold below, patch them, and rerun. If the audit is clean, say
  the repo is stable instead of searching for more cosmetic rewrites.
- Exploratory scan: a deliberate search for new conventions or broad cleanup opportunities. Do this
  only when the user asks for exploratory work or when a repeated issue clearly meets the threshold
  below.

During a stabilization scan, do not introduce a new style rule unless the issue is factual,
conflicts with this file, appears in at least three files, or is likely to confuse a reader.
Cosmetic wording below that bar should be left alone or mentioned as backlog, not patched
immediately.

Treat each scanner rule as one repeated family of problems, such as "documented API signatures must
show meaningful default arguments" or "long repeated literal defaults should be named constants."
Pick one family, encode it, fix its findings, and rerun before moving to another.

Code-quality stabilization follows the same rule. Prefer deterministic checks for factual or
repo-wide consistency issues, and keep taste calls out of the default scan until the user accepts
them as a convention. The current code-consistency pass in `scan_quality.py` checks for:

- unused helper functions left in example blocks;
- direct `std::random_device{}` seeding inside examples, where the fixed seed `1234567` keeps
  examples reproducible;
- fixed literal seeds in reusable randomized code, which should seed at runtime instead;
- C `rand()`/`srand()` usage, which should be replaced by a C++ random engine and distribution;
- `std::fabs()` usage, which should follow the anthology's plain `fabs()` convention;
- `static_cast<C>(EPS)` in floating-point comparison helpers, where direct `EPS` is clearer;
- `include_boundary` naming drift in geometry boundary flags;
- `Overflow warning.` comment punctuation.

When scanning for code quality, first run `python3 scan_quality.py` and fix deterministic
findings. Then do a bounded exploratory pass for one new family of smells, such as dead example
helpers, hidden nondeterminism in examples, duplicated constants, ordered containers used only for
membership in implementation code, or inconsistent helper APIs. If a new family is accepted, encode
it in `scan_quality.py` or document the judgment rule here before continuing. Do not turn one-off
preferences into hard rules.

In prose, use math mode for mathematical variables and numeric constants (`$n$`, `$998244353$`) and
backticks for code identifiers or parameters (`n`, `MOD`). If a bound is a purely mathematical
quantity in explanatory prose, use a plain math interval such as `$[0, n)$`. In API bullets, keep
backticks for parameter-bounded domains; for example, a `SegTree<T>(n)` bullet should write the
index domain with backticked `n` inside the math interval. Keep backticks inside tuple notation when
the entries are code-facing return-field names or parameters, but use plain math tuples like
`$(i, j)$` in conceptual prose. If a sentence defines a math symbol from code, such as "$n$ is
`size()`", keep later uses of that symbol in math notation, e.g. `$[0, n]$`.
Do not introduce a code-styled size variable solely to restate a container's size, such as
"$[0, `n`)$, where `n` is `adj.size()`"; say that the indices of `adj` represent the nodes instead.
When a prose sentence is explicitly explaining code-index formulas, code-style intervals such as
$[`l`, `r`)$ may be clearer than plain math intervals.
When an API bullet is primarily describing a mathematical object and already uses the parameters as
math variables throughout, prefer the local mathematical style, e.g. `$[0, k)$` for
`de_bruijn(k, n)`.
In API bullets, show the common class instantiation rather than repeating defaulted comparator or
hasher policies in every constructor signature. For example, write `RollingHash<T>(lo, hi)` and
explain separately that `RollingHash<T, Hash>` accepts a custom hasher. Keep behavior-defining
compile-time modes visible when they are central to the API.

When a new repeated convention is accepted, encode it in one of two places before relying on it in
future scans: `AGENTS.md` for human/agent judgment, or `scan_quality.py` for deterministic
grep-style drift checks. This is the path to a stable "scan" result.

## Editing Discipline

- Preserve unrelated user changes in the dirty worktree. Do not revert files unless explicitly
  asked.
- Use `rg` for searches, and keep manual edits to targeted, single-purpose hunks.
- Keep diffs scoped. Do not reformat or refactor unrelated sections just because you noticed them.
- When changing an API, update the docstring, examples, and all local callers in the same edit.
- When touching docstrings, preserve the generated-book style: 100 columns, ASCII, API bullets, and
  complexity blocks.
- If a change affects a broad convention, scan the repo for similar occurrences and either update
  them or call out why they are intentionally different.
