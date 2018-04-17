Alex's Anthology of Algorithms: Common Code for Contests in Concise C++ (A<sup>3</sup>C<sup>5</sup>)
==================

*Important Note*: This repository is undergoing a major rewrite. The PDF and work-in-progress sections are outdated and buggy. Use at your own risk.

## Introduction

Welcome to a comprehensive collection of common algorithm and data structure implementations. You will find that the code here is best-suited for programming contests, but are by no means limited to them. I developed this project with following principles in mind:

* Clarity: A reader already familiar with an algorithm should have no problem understanding how its particular implementation works. Consistency in naming conventions should be emphasized, and language-specific tricks or other implementation details should be documented.

* Genericness: Whenever possible, it should be easy to adjust an implementation to achieve slightly different goals. This could be tweaking a constant parameter, or supporting a slightly different data type. In a live contest, the more lines of code that must be changed, the more room there is for mistakes. C++ templates are often used to accomplish this at the slight cost of simplicity.

* Portability: Different contest environments use different versions of C++ (though almost all of them use GCC). In order to maximize compatibility, non-standard features are avoided. The decision to adhere to C++98 standards is due to many contest systems being stuck on older versions of the language. Moreover, minimizing newer, C++ specific features should make this anthology friendlier to users of other languages. See to the "Portability" section below for more info.

* Efficiency: Since the code is meant to be run during real contests, it is nice to maintain a reasonably low constant overhead. This is sometimes challenging in the face of clarity and genericness, but we can hope for contest setters to be liberal with time limits. If an implementation does not pass in time, chances are the wrong algorithm is being chosen with too slow of a big-O running time.

* Concision: Code chunks are often shifted around the file in the frenzy of timed contests. To minimize the amount of scrolling and searching, code compactness is valued. It is no surprise that each section is confined to a singleton source file, since nearly all contest systems demand submissions to be of this format.

A fair trade-off between these factors is the holy grail.

My secondary goal is to have common algorithms modelled in a way simple enough that programmers in other languages may understand as well. This is hopefully achieved for most sections, but C++ specific concepts may still used in certain areas to prioritize the above principles.

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

## Usage Remarks

This project does __not__ aim to introduce and explain algorithms from the ground up. To take advantage of this anthology, you must have prior understanding of the algorithms in question. In each source file, you will find brief descriptions and simple examples to clarify how the functions and classes should be used. You will not find rigorous test suites, nor extensive comments detailing how the implementations work. To properly learn the algorithms, you are better off researching and implementing them independently. You are free to use, modify, and distribute these programs in accordance to the license, but please first examine any corresponding references of each program for more details on usage and authorship.

Cheers.<br>
— Alex

