Alex's Anthology of Algorithms: Common Code for Contests in Concise C++ (A<sup>3</sup>C<sup>5</sup>)
==================

*Important Note*: This repository is undergoing a major rewrite. The PDF and work-in-progress sections are outdated and buggy. Use at your own risk.

## Introduction

This project aims to implement common algorithms in the most concise and "vanilla" way possible. The primary goal is to have code that is easily adaptable for direct usage in programming contests. To that end, the style of this codebase is founded on the following core traits:

* Clarity: The code should be self-documenting, assuming you already understand the algorithm at a high level. Consistency in naming conventions is emphasized.

* Genericness: The more code that must be changed during the contest, the more room there is for mistakes. It should be easy to adapt these implementations for different operations, data types, etc. (where possible). C++ templates are often used to accomplish this at the slight cost of readability.

* Portability: Different contest environments use different versions of C++ (though almost all of them use GCC). In order to maximize compatibility, non-standard features are typically avoided. The decision is to adhere to C++98 standards as many constest systems remain stuck on older versions of the language. After all, minimizing C++ specific features will make this anthology friendlier to users of other languages. Refer to the "Portability" section below for more information.

* Efficiency: Since the code is meant to be used in real contests, it is nice to maintain a reasonably low constant overhead. This is sometimes challenging in the face of clarity and genericness, but we can typically hope for contest setters to be liberal with time limits. If an implementation does not pass in time, chances are you are choosing the wrong algorithm with a wrong big-O time.

* Concision: Code chunks are often shifted around the file in the frenzy of timed contests. To minimize the amount of scrolling and searching, code compactness is highly valued. It is a given that implementations are given in singleton ".cpp" files, since nearly all contest systems demand submissions in single files.

A good trade-off between these five factors is key.

## Portability

All programs are tested with the GNU Compiler Collection (GCC) compiled for a 32-bit target system. Compiler switches:

```
g++ -std=gnu++98 -pedantic -Wall -Wno-long-long -O2
```

This means that the following assumptions are made about data types:
* `bool` and `char` are 8 bits
* `int` and `float` are 32 bits
* `double` and `long long` are 64 bits
* `long double` is 96 bits

Programs are highly portable (ISO C++ 1998 compliant), __except__ in the following regards:
* Usage of long long and dependent features `[-Wlong-long]`, which are compliant in C99/C++0x or later. 64-bit integers are a must for many programming contest problems, so it is necessary to include them.
* Usage of GCC's built-in functions like `__builtin_popcount()` and `__builtin_clz()`. These can be extremely convenient, and are easily implemented if they're not available. See here for a reference: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
* Usage of compound-literals, e.g. `myvector.push_back((mystruct){a, b, c})`. This adds a little more concision by not requiring a constructor definition.
* Rare cases where bitwise hacks are intentionally used, such as in functions for getting the signbit with type-puned pointers. If you are looking for these features, chances are you don't care about portability anyway.

## Usage Notes

The primary purpose of this project is not to better your understanding of algorithms. To take advantage of this anthology, you must have prior understanding of the algorithms in question. In each source file, you will find brief descriptions and simple examples to clarify how the functions and classes should be used. You will not find rigorous test suites, nor extensive comments detailing how the implementations work. As such, if you truly wish to learn the algorithms, you are better off researching and implementing them independently. You are free to use, modify, and distribute these programs in accordance to the license, but please first examine any corresponding references of each program for more details on usage and authorship.

Cheers.<br>
— Alex

