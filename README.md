Alex's Anthology of Algorithms: Common Code for Contests in Concise C++ (A<sup>3</sup>C<sup>5</sup>)
==================

Website: [https://algorithms.alexli.ca](https://algorithms.alexli.ca)
Latest PDF: [Download](https://github.com/alxli/algorithm-anthology/raw/master/Book/A3C5.pdf)

Please beware of any older versions in circulation, as they're very buggy.

## Introduction

Welcome to a practical collection of concise C++ implementations for algorithms, data structures, and contest utilities. This is not a first textbook; it is a codebook about implementation details, interfaces, edge cases, and the choices that make familiar algorithms usable in real code.

There are several good ways to use this book:

* as a reference when you know an algorithm conceptually, but want to study a clean implementation;
* as a printed codebook for contests whose rules allow prepared reference material, including ICPC-style contests;
* as a checklist for edge cases, complexity, overflow, indexing conventions, and API choices;
* as a map of related techniques: slower versions, faster versions, variants, and common reductions are often placed near one another.

For learning, do not start by copying. Study the idea, prove the invariant or recurrence, implement a version yourself, then compare it against this one; the differences usually reveal the practical choices. Each section gives only the theory needed to orient the implementation: the problem, public interface, assumptions, and complexity. The source is organized as documentation, reusable implementation, then example usage. The reusable part ends before the `Example Usage` marker; the code after that marker exists only so the file can compile and demonstrate itself in isolation. When adapting a section into a solution file or personal header, remove the example block and keep the implementation above it.

The implementations are written with the following principles in mind:

* *Clarity:* A reader familiar with the algorithm should be able to follow the code without reverse-engineering it. Important tricks, non-obvious invariants, overflow risks, and language-specific assumptions should be documented.
* *Concision:* Contest code has to be found, read, adapted, and typed under pressure. Short code is valuable when it removes noise, but not when it hides the idea. The goal is compactness without cleverness for its own sake, and without discarding useful interfaces. As a result, the code here is often more verbose than compact ICPC codebook-style snippets: the extra code usually buys clearer names, reusable return values, safer defaults, configurable types, edge identifiers, reset functions, or explicit handling of common variants.
* *Efficiency:* The code should have the expected asymptotic behavior and reasonable constants. When a simpler version and a more robust or faster version both teach something useful, both may be included. If an implementation is educational rather than competitive, the section should say so.
* *Adaptability:* Real problems rarely match a template exactly. Interfaces are chosen so that common changes, such as swapping a numeric type, changing a combine function, exposing an edge ID, or choosing whether boundaries are included, are straightforward.
* *Consistency:* Similar structures should expose similar operations where practical. Repeated naming conventions and example styles make it easier to move between chapters quickly.
* *Portability:* The code targets modern C++17 and tries to avoid unnecessary dependencies. A few contest-useful exceptions remain, but are noted with each use.

This project began as a personal contest codebook, so it still favors single-file snippets, predictable APIs, compact examples, and low constants. It has also become a record of design decisions: what to make generic, what to leave explicit, when to include a slow version, and how to make a known algorithm pleasant to use.

Use this anthology actively: annotate it, delete what you dislike, rename functions to match your own habits, and stress-test anything you intend to trust.

Cheers,
— Alex

## Portability Note

All examples are intended for a modern GCC or Clang environment with C++17 support. A typical compile command is:

```
g++ -std=c++17 -O2 -Wall -pedantic
```

Most implementations assume common contest-platform type sizes: 8-bit `char`, 32-bit `int` and `float`, and 64-bit `double` and `long long`. The width and precision of `long double` are implementation-dependent; sections that rely on it should say so explicitly.

The code targets ISO C++17 unless noted. The most common exceptions are:

* GCC and Clang integer extensions such as `__int128` or `__uint128_t`, usually guarded with a portable fallback when overflow safety matters.
* Compiler builtins such as `__builtin_popcount()` and `__builtin_clz()`. These are convenient and widely available, but C++20's `<bit>` header provides standard alternatives for many of them. See https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html for GCC's reference.
* GNU policy-based data structures such as `__gnu_pbds::tree`, in limited sections only to show some useful but non-standard library components.
* A small number of explicitly documented representation assumptions, such as `signbit_()` in the math utilities assuming an IEEE-like floating-point sign bit layout.
