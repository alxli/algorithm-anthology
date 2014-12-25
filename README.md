Alex's Anthology of Algorithms: Common Code for Contests in Concise C++ (A³C⁵)
==================

*Note*: You may find an **outdated** PDF version here: http://alexli.ca/A3C5.pdf.
This version will contain mistakes that is fixed in the current GitHub branch.
Unfortunately, I created it in a hurry with Microsoft Word when I was too busy to
tinker with actual LaTeX. Please contact me (alextrovert@gmail.com) if you are
good at LaTeX and would like to help me typeset the respository into a book like
the PDF above. I would be more than ecstatic!

## Introduction

This project does not intend to better your understanding of algorithms. You will only find examples and brief descriptions to help you better understand the given implementations. Thus, you are expected to already have knowledge of algorithmic programming paradigms, as well as the actual algorithms being discussed. In many cases, it is possible to learn an algorithm by examining its implementation. However, memorizing another person’s code is generally a less efficient way to learn than researching the idea and trying to implement it by yourself. Treat this as a last resort, if you will, when you simply require a working implementation.

If you seek to become a better contest programmer, I suggest you follow through with the USACO training pages (http://train.usaco.org), read up on informatics books and online algorithm tutorials (e.g. “Introduction to Algorithms” by CLRS, “The Algorithm Design Manual” by Skiena, “The Art of Computer Programming” by Knuth, and Topcoder tutorials: http://www.topcoder.com/tc?d1=tutorials&d2=alg_index&module=Static). Practice on sites such as the Sphere Online Judge (http://www.spoj.com), Codeforces (http://www.codeforces.com), the UVa Online Judge (uva.onlinejudge.org), and the PEG judge (http://www.wcipeg.com). Perhaps along the way, you will infrequently refer here for insight on ways to implement any newly-learned concepts.

C++ is my language of choice because of its predominance in competitions. The International Olympiad in Informatics (and practically every other programming contest) accepts solutions to tasks in C, C++ and Pascal. C++ is a fast, flexible language with a sizable Standard Template Library and support for useful features like built-in memory management and object-oriented programming. In an attempt to focus less on reinventing the wheel and more on the algorithms themselves, the implementations here will often try to take advantage of useful features of the C++ language. The programs are tested with version 4.7.3 of the GNU Compiler Collection (GCC) on a 32-bit system (that means, for instance, bool and char are assumed to be 8-bits, int and float are assumed to be 32-bits, double and long long are assumed to be 64-bits, and long double is assumed to be 96-bits).

All the information in descriptions come from Wikipedia and other online sources. Some programs you will find are direct implementations of pseudocode found online, while others are re-adaptated from informatics books and journals. If references for a program are not listed in its comment section, you may assume that I have written them from scratch. You are free to use, modify, and distribute these programs in accordance to GPL v2, but please examine any associated references to the programs. I strongly recommend making an effort to understand any code you did not write on your own. Once you are well acquainted with the underlying algorithms, you will become better at adapting them for the problem at hand.

This anthology started as a personal project to implement classic algorithms in the most concise and “vanilla” way possible while minimizing code obfuscation. I tried to determine the most appropriate trade-off between clarity, flexibility, efficiency, and code length. Short descriptions of what the programs do along with their time (and space, where necessary) complexities are typically included. For further clarification, diagrams may be given for the examples. If these are insufficient to help you understand, then you should be able to find extra information by referring to the header comments and looking online. Lastly, I do not guarantee that the programs are bug-free - it is always best to read the comments and examples, especially when you are modifying a program.

Cheers and best of luck!

— Alex Li
