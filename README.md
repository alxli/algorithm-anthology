Alex's Anthology of Algorithms: Common Code for Contests in Concise C++ (A<sup>3</sup>C<sup>5</sup>)
==================

*Note*: You may find the currently available algorithms in a single PDF here: http://alexli.ca/A3C5-new.pdf.
You may also find an **outdated** PDF version here: http://alexli.ca/A3C5.pdf.
The PDF version contains only a fraction of algorithms found in this GitHub repository, and a whole multitude of mistakes that are fixed here. Use at your own risk!

## Introduction

This anthology started as a personal project to implement classic algorithms in the most concise and "vanilla" way possible while minimizing code redundancy and obfuscation. I tried to make the most appropriate trade-off between clarity, flexibility, efficiency, and code length. Short descriptions of what the programs do along with their time (and space, where necessary) complexities are included. If these are insufficient to help you understand, then you should be able to find extra information by referring to the code's comments and looking online. I do not guarantee that the programs are bug-free - it is always best to carefully study the usage notes and examples first, especially before you make your own modifications.

C++ is my language of choice because of its predominance in competitions. The International Olympiad in Informatics (and practically every other programming contest) accepts solutions to tasks in C, C++ and Pascal. C++ is a fast, flexible language with a sizable standard library and support for useful features like built-in memory management and object-oriented programming. In an attempt to focus less on reinventing the wheel and more on the essence of the algorithms themselves, these implementations will attempt to take advantage of useful features of the C++ language and library (iterators, templates, functions in \<algorithm\>, STL data structures, etc.) whenever possible.

The primary purpose of this project is not to better your understanding of algorithms. To take advantage of this anthology, you must have prior understanding of the algorithms in question. In each source code file, you will find brief descriptions and simple examples to clarify how the functions and classes should be used (not so much how they work). This is why if you actually want to learn algorithms, you are better off researching the idea and trying to implement it by yourself. Treat this as a last resort, if you will, when you just need a concise and working implementation in the heat of a programming contest.

All the information in the comments (descriptions, complexities, etc.) come from Wikipedia and other online sources. Some programs here are direct implementations of pseudocode found online, while others are adaptated and translated from informatics books and journals. If references for a program are not listed in its comments, you may assume that I have written them from scratch. You are free to use, modify, and distribute these programs in accordance to the license, but please examine any corresponding references to each program for more details on usage and authorship before you use it.

Cheers and hope you enjoy!

— Alex Li

## Portability

All programs are tested with version 4.7.3 of the GNU Compiler Collection (GCC) compiled for a 32-bit target system.

That means the following assumptions are made:
* bool and char are 8 bits
* int and float are 32 bits
* double and long long are 64 bits
* long double is 96 bits

Programs are highly portable (ISO C++ 1998 compliant), __except__ in the following regards:
* Usage of long long and related features \[-Wlong-long\] (such as LLONG_MIN in \<climits\>), which are compliant in C99/C++0x or later. 64-bit integers are a must for many programming contest problems, so it is necessary to include these.
* Usage of variable sized arrays \[-Wvla\] (an easy fix using vectors, but I chose to keep it because it is simpler and because dynamic memory is generally good to avoid in contests)
* Usage of GCC's built-in functions like __builtin_popcount() and __builtin_clz(). These can be extremely convenient, and are easily implemented if they're not available. See here for a reference: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
* Usage of compound-literals, e.g. vec.push_back((mystruct){a, b, c}). This is used in the anthology because it makes code much more concise by not having to define a constructor. It is also trivial to fix, so what the heck.
* Ad-hoc cases where bitwise hacks are intentionally used, such as functions for getting the signbit with type-puned pointers. If you are looking for these features, chances are you don't care about portability anyway.
