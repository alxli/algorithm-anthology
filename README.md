Alex's Anthology of Algorithms: Common Code for Contests in Concise C++ (A<sup>3</sup>C<sup>5</sup>)
==================

You can browse all sections here on the [website](https://algorithms.alexli.ca) or download the [PDF](https://github.com/alxli/algorithm-anthology/raw/master/Book/A3C5.pdf).

Beware of older versions in circulation which are riddled with bugs.

## Introduction

Welcome to a practical collection of concise C++ implementations of algorithms, data structures, and contest utilities. This is not a first textbook; it is a codebook about implementation details, interfaces, edge cases, and the choices that make familiar algorithms usable in real code. Use it:

* as a reference when you know an algorithm conceptually but want a clean implementation;
* as a printed codebook for contests that allow prepared material, including ICPC-style events;
* as a checklist for edge cases, complexity, overflow, indexing, and API choices;
* as a map of related techniques: slower versions, faster versions, variants, and common reductions sit near one another.

For learning, don't start by copying. Study the idea, prove the invariant or recurrence, implement it yourself, then compare; the differences reveal the practical choices. Each section gives only enough theory to orient the implementation: problem, interface, assumptions, and complexity. Source files are organized as documentation, reusable implementation, then example usage. The reusable part ends at the `Example Usage` marker; everything after exists only so the file compiles and demonstrates itself. When adapting a section, drop the example block and keep the implementation above it.

Guiding principles:

* *Clarity:* A reader familiar with the algorithm should follow the code without reverse-engineering it. Tricks, non-obvious invariants, overflow risks, and language assumptions are documented.
* *Concision:* Contest code is found, read, adapted, and typed under pressure. Short code helps when it removes noise, not when it hides the idea. So the code here is often more verbose than compact ICPC snippets; the extra lines buy clearer names, configurable types, safer defaults, reset functions, explicit handling of variants, and more.
* *Efficiency:* Expected asymptotic behavior and reasonable constants. Both simpler and more robust versions are often included, but educational-only sections are labeled so.
* *Adaptability:* Interfaces make common changes easy, such as swapping a numeric type, changing a combine function, exposing an edge ID, or choosing whether boundaries are included.
* *Consistency:* Similar structures expose similar operations, with repeated naming and example styles, so moving between chapters is easy.
* *Portability:* Targets modern C++17 and avoids unnecessary dependencies; the few contest-useful exceptions are noted at each use.

This began as a personal contest codebook, so it favors single-file snippets, predictable APIs, compact examples, and low constants. Use it actively: annotate it, delete what you dislike, rename functions to match your habits, and stress-test anything you intend to trust. Cheers!

## Portability Note

Examples target a modern GCC or Clang environment with C++17. A typical compile command:

```
g++ -std=c++17 -O2 -Wall -pedantic
```

Most code assumes common contest-platform type sizes: 8-bit `char`, 32-bit `int` and `float`, 64-bit `double` and `long long`. `long double` width and precision are implementation-dependent; sections relying on it say so.

The code targets ISO C++17 unless noted. The most common exceptions:

* GCC/Clang integer extensions like `__int128` or `__uint128_t`, usually guarded with a portable fallback when overflow safety matters.
* Compiler builtins like `__builtin_popcount()` and `__builtin_clz()` — convenient and widely available, though C++20's `<bit>` header standardizes many of them ([GCC reference](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)).
* GNU policy-based data structures like `__gnu_pbds::tree`, used in a few sections to show useful non-standard components.
* A few documented representation assumptions, such as `signbit_()` in the math utilities assuming an IEEE-like floating-point sign bit layout.
