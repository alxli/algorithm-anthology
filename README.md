Alex's Anthology of Algorithms: Common Code for Contests in Concise C++ (A<sup>3</sup>C<sup>5</sup>)
==================

*Announcement*: An updated PDF version of this repo is coming soon. Until then, please avoid any older PDF versions in circulation, as they are highly outdated and buggy.

## Introduction

Welcome to a comprehensive collection of common algorithm and data structure implementations. The code here is suitable for direct use in programming contests, or just for your own studying. I wrote them with the following principles in mind:

* Clarity: A reader already familiar with an algorithm should have no problem understanding how its implementation works. Consistency in naming conventions should be emphasized, and any tricks or language-specific hacks should be documented.

* Genericness: It should be easy to adapt an implementation to achieve slightly different goals. This could be through tweaking the parameters or data types at play. During a live contest, more lines of code to be edited corresponds to more room for bugs. C++ templates are often used to increase tweakability at the slight cost of simplicity.

* Portability: Different contest environments use different versions of C++ (though almost all of them use GCC). In order to maximize compatibility, non-standard features are avoided. The decision to adhere to C++98 standards is due to many contest systems being stuck on an older version of the language. Moreover, minimizing newer C++ specific features will make the implementations more language-agnostic. Refer to the "Portability" section below.

* Efficiency: The code here is designed to be performant on real contests, and should maintain a low constant overhead. This is often challenging in the face of clarity and tweakability, but we can hope for contest setters to be liberal with time limits. You can reasonably assume that when a program times out, it is the fault of choosing an algorithm with the wrong complexity class entirely, rather than insufficient constant optimization.

* Concision: Code chunks are often shifted around the file in the frenzy of timed contests. To minimize the amount of scrolling and searching, code compactness is valued. It is no surprise that each implementation is confined to a single source file, since nearly all contest systems require individual file submissions with no outside dependencies.

A good trade-off between these factors is the holy grail.

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
* Usage of GCC's built-in functions like `__builtin_popcount()` and `__builtin_clz()`. These can be extremely convenient, and are easily implemented if they are not available. See here for a reference: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
* Usage of compound-literals, e.g. `myvector.push_back((mystruct){a, b, c})`. This adds a little more concision by not requiring a constructor definition.
* Certain hacks that may depend on aspects of the platform (e.g. endianness), such as getting the signbit with type-puned pointers. Be weary of this for all bitwise/micro-optimization related code.

## Usage Remarks

This project does __not__ aim to introduce and explain algorithms from the ground up. To take advantage of this anthology, you must have prior understanding of the algorithms in question. In each source file, you will at most find brief descriptions to identify the problem being solved, and small examples to demo how the code should be used. Researching an algorithm and implementing it independently is the only way to properly learn it. You are free to use, modify, and distribute these programs in accordance to the license, but please first examine any references on each section for details on usage and authorship.

Cheers.<br>
— Alex

