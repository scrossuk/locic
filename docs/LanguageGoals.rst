Language Goals
==============

Background
----------

The C programming language is a widely used programming language and compilers are available for almost all architectures. Furthermore, many programming languages provide the means to call into/out-of C, facilitating inter-operation of different programming languages with C as a 'bridge'. While technically useful, the C programming language is often perceived as lacking sufficiently powerful abstractions to construct large and complex systems. For this reason, languages such as C++, Objective C and D were invented to provide abstractions on top of the language.

Unfortunately, these languages have significant problems. For example, C++ and D place undue attention to compile-time functionality that serves to complicate the source code. Interestingly, Objective-C shifts the vast majority of the burden to run-time, and hence uses weak typing. Preference of programming languages is certainly based on the opinion of individual programmers, however languages also face general criticism. In particular, C++ attracts criticism due to its large set of features and rules that ultimately combine to produce complex semantics. Equally, however, programmers support the combination of powerful features as found in a multi-paradigm language such as C++.

The design of languages that are both simple and powerful is difficult, and popular existing programming languages represent a variety of reasonable solutions. A number of newer languages have emerged that operate at a 'higher level' than languages such as C and C++, of which examples are Java and C#. Typically, these languages use primitives and pointers as fundamental types, deploy some form of garbage collection, use classes as their fundamental data structures and provide the means to construct inheritance hierarchies. There is a disputed performance cost to using these languages over lower level languages, which is generally found to be small and hence arguably insignificant.

However, these languages also suffer significant problems: there are no deterministic destruction routines (which must be provided by the programmer through constructs such as try...finally), the languages have extensive dependencies on the large and complex environment that supports them and they lack direct access to lower level routines typically available in C. Furthermore, despite aims to remain simple, many of these languages have become increasingly complex as it is discovered the initial design was insufficiently powerful. Special cases (e.g. Java's + operator for strings) further serve to complicate the languages.

Aims
----

This document therefore describes and rationalises the design of the Loci programming language, which aims to provide strong compatibility with C, while adding useful efficient high level abstractions, and without the complexities that arise in C++. Note that the Loci Compiler Tools (of which this document is currently a part) may not have a complete implementation of all features described here.

Loci should satisfy the following (quite vague) goals:

* *Modular* - The language must help to produce programs in which the basic problem is split into distinct components, which can be easily re-combined to solve other problems.
* *Portable* - The language should be independent of any specific machine architecture or operating system, such that programs can be ported easily.
* *Compatible* - The language must be compatible with existing languages/standards so that it can make use of existing services.
* *Simple* - The language must remove all unnecessary complexity and maximize developer efficiency so that time can be best spent solving the problem at hand. It is important to reduce the learning curve to remove any barriers preventing new developers from embracing the language.
* *Clean* - The language must help to produce clean and consistent applications which can be easily read and understood. It is important that when one developer sees code written by another developer it is easy for them to understand it, in order to aid collaboration.
* *Small* - The language must produce resulting code that is small and free of dependencies except those specified in the program and understood to the developer, such that distribution is simple and fast.
* *Fast* - The language must provide good performance, to maximise its utility.

Name
----

While it has been suggested that Loci could stand for 'Loci: objects, classes and interfaces', this is not the intended understanding because the language is much more than just being object oriented (like C++, Loci is a multi-paradigm language). Instead, the name reflects the following definitions of its singular form:

	locus - the set of all points or lines that satisfy or are determined by specific conditions

	locus - A place or locality, especially a centre of activity

Specific Goals
--------------

Establishing this basis for Loci, it is possible to elaborate more specific goals for the language:

* **Provide stable interfaces between modules.** A change in one module should not require a module that depends on it to be re-compiled, unless there are changes that break the API. In particular, classes in libraries can change their internal structure (i.e. their member variables) without requiring re-compilation of clients of the library.
* **Follow a standard binary interface for each platform.** Modules compiled by different compilers must work together. Loci creates standard C functions with fixed naming rules and calling conventions.
* **Fast compile times.** This is vital to maximise productivity. Loci files are effectively isolated from each other, such that they can be tokenized and parsed in parallel, and then the resulting structures can be used to resolve and verify dependencies. Effectively parallelising builds for other languages is often significantly more complex.
* **Minimal syntactic complexity.** For example, header guards are required in C to prevent a header from being included twice. In Loci all files are equal and are only analyzed once, so there is no need to add any sort of guards. Furthermore the Loci compiler uses multiple passes so type declarations/definitions do not have to appear before their use.
* **Minimal semantic complexity.** Loci avoids language features (or combinations of language features) that affect the readability and predictability of code. For example, Loci does not support function or method overloading, since these can easily increase the complexity of the language with little to no added benefit.
* **Provide a powerful polymorphism mechanism.** Loci provides interfaces, that are effectively equivalent to C++ pure virtual classes with a virtual destructor, or Java interfaces. Loci uses structural typing, so casts are allowed from classes to interfaces if the class provides all the methods required by the interface.

Design
------

Loci is very closely related to C in both syntax and semantics, with the ability to call C functions (and be called from C) easily and to handle C types. Numerous ideas from languages such as C++, Java, Objective C, ML and Python are also used (although in varying amounts; the language certainly pulls more ideas from C++ than any other language) to develop the features and abstractions that Loci provides on top of the basic functionality of C.

A primary concern for the language design is closely resembling the 'look and feel' of languages such as C and C++, with the intention that developers familiar with those languages should be able to easily get started in Loci. Syntactic or semantic divergences from existing languages are justified in this document.

Loci emphasises providing choice for the programmer, in the form of features that are each suited for particular purposes, where such features interact well with the rest of the language. A good example is algebraic data types, which are well suited to expressing data such as ASTs (Abstract Syntax Trees, which are constructed in a compiler front-end and approximately express the structure of the input source code).

However, as well as using ideas from many languages, Loci also specifically rejects some features, such as method/function overloading, class inheritance and weak typing (specifically, a large set of legal implicit casts; for example, in C, it is valid to implicitly cast from 'void *' to 'int *'). Each of the features left out are considered at least unnecessary and potentially harmful, and interact poorly with the surrounding environment.

In addition, Loci modifies well known features to make them easier to use, to help the programmer avoid common pitfalls and as an attempt to fix the flaws in some programming languages that are now evident in hindsight. The language also attempts to help programmers verify code correctness and to make code reasonably self-documenting.

This section explains and justifies some of the design decisions made within the language; the following section then shows how these features are implemented. Each design decision was made with a good understanding of how it would be implemented, in particular to minimise the compiler complexity (and correspondingly, the time taken for compilation), code dependencies and code size, and to maximise the performance.

