Alex's Anthology of Algorithms: Common Code for Contests in Concise C++ (A<sup>3</sup>C<sup>5</sup>)
==================

[PDF Download](https://github.com/alxli/algorithm-anthology/raw/master/Book/a3c5.pdf). Please beware of any older versions in circulation, as they are very buggy.

## Introduction

Welcome to a comprehensive collection of common algorithms and data structures. The ultimate goal of this book is not to explain concepts from the ground up, but instead to explore the finer details behind their *implementations*. There are many potential ways you can use this, for instance:

* as a reference to help you better understand topics that you have only studied on a high level,
* as a printed codebook, which is a permitted resource for contests such as the ACM ICPC, or
* to cross-check existing code you have written for contest or coding interview questions.

Before diving into any section, it is strongly recommended that you have already studied the algorithms involved. Reading the code first is never an ideal approach to properly understand an algorithm. You should instead try proving (its correctness and time/space complexity) and implementing it from scratch.

Every topic to be explored is easily researchable online. Thus instead of including theoretical discussions, I document just enough to establish the problem being solved, notation being used, and any special trickery involved. I have also included small, non-rigorous examples to demonstrate usage of the code.

We mentioned that the implementation itself is the focus, but what makes an implementation good? The code is written with the following principles in mind:

* *Clarity:* A reader already familiar with an algorithm should have no problem understanding how its implementation works. Consistency in naming conventions should be emphasized, and any tricks or language-specific hacks should be documented.
* *Concision:* To minimize the amount of scrolling and searching during the frenzy of time contests, it is helpful for code to be compact. Shorter code is also generally easier to understand, as long as it is not overly cryptic. Finally, each implementation should fit in a single source file as required by nearly all online judging systems.
* *Efficiency:* The code here is designed to be performant on real contests, and should maintain a low constant overhead. This is often challenging in the face of clarity and tweakability, but we can hope for contest setters to be liberal with time and memory limits. If the code here times out, you can reasonably rule out insufficient constant optimization and assume that you are choosing an algorithm from a suboptimal complexity class.
* *Genericness:* Implementations should be easy to adapt to achieve slightly different goals. One may want to tweak some core logic, parameters, data types, etc. In timed contests, we would certainly prefer this process to be as painless as possible. C++ templates are often used to increase tweakability at a slight cost to simplicity.
* *Portability:* Different contest environments use different compiler builds. In order to maximize compatibility, non-standard and newer features are avoided. The decision to follow C++98 standards is due to many contest systems being stuck on an older version of the language. Moreover, minimizing newer C++ features will make the code more language-agnostic.

As these points and the title both suggest, there is a slight bias towards contests. Compiling a codebook for my personal reference during contests was indeed how this project got started. This work has become much more multipurpose now. Whatever your use case is, I hope you discover something enlightening.

Cheers.<br>
— Alex

## Portability Note

All programs were tested with GCC and compiled for a 32-bit target using the switches below::

```
g++ -std=gnu++98 -pedantic -Wall -Wno-long-long -O2
```

This means the following are assumed about data types:
* `bool` and `char` are 8-bit.
* `int` and `float` are 32-bit.
* `double` and `long long` are 64-bit.
* `long double` is 96-bit.

Programs are highly portable (ISO C++ 1998 compliant), except in the following regards:
* Usage of `long long` and dependent features, which are compliant in C99/C++0x or later. 64-bit integers are a must for many contest problems.
* Usage of variable sized arrays. While easily replaced by vectors, they are generally simpler and avoid dynamic memory (which some argue is a bad idea for contests).
* Usage of GCC's built-in functions like `__builtin_popcount()` and `__builtin_clz()`. These can be extremely convenient, but are straightforward to implement if unavailable. See here for a reference: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
* Usage of compound-literals, e.g. `myvector.push_back((mystruct){a, b, c})`. This adds a little more concision by not requiring a constructor definition.
* Hacks that may depend on the platform (e.g. endianness), such as getting the signbit with type-punned pointers. Be weary of portability for all bitwise/lower level code.
