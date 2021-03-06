Compiler Design
===============

This document provides a high level overview of the design structure of the Loci compiler.

Representations
---------------

The compiler uses the following internal representations:

* **Source code** - Text format code as most developers will recognise.
* **AST** - An abstract syntax tree representation heavily based on the structure of text source code. This representation is not suitable for internal analysis since it is complex to cross-reference construct declarations and uses. There is also no invariant requiring the representation to be valid (e.g. there may be uses that refer to non-existent declarations).
* **LLVM IR** - A target-independent assembly representation specified by the LLVM compiler infrastructure.

Stages
------

The compiler consists of four stages:

* **Lexing** - Identifying tokens within a source file.
* **Parsing** - Matching sequences of tokens with language constructs, generating AST representation.
* **Semantic Analysis** - Verifying and manipulating AST representation, issuing any relevant error messages.
* **Code Generation** - Generating LLVM IR that matches AST operations.

Since it's a front-end, these additional stages are provided by LLVM:

* **Target Independent Optimisation**
* **Module Linking**
* **Target Code Generation**

Typically *Target Independent Optimisation* and *Module Linking* are combined as part of *Link Time Optimisation*.

Lexer
~~~~~

A lexer is responsible for reading source files and generating sequences of tokens. For example 'a + b' turns into [NAME(a), PLUS, NAME(b)]. This stage is implemented in libs/Lex.

Originally the lexer was generated by Flex but it has since been rewritten directly in C++ code in order to:

* Improve error messages.
* Support complex parser/lexer interactions.
* Be consistent with having a handwritten parser.

Parser
~~~~~~

The parser converts the sequence of tokens from the lexer into a AST (Abstract Syntax Tree). This stage has to resolve various complexities of the Loci grammar, including:

* Using ``<`` and ``>`` for template syntax conflicts with less-than and greater-than operators.
* Reference type syntax (``TYPE &``) conflicts with bitwise-and (``VALUE & VALUE``) in assignments (e.g. ``TYPE& VALUE = ...``).
* Pointer type syntax (``TYPE *``) conflicts with multiply (``VALUE * VALUE``) in assignments (e.g. ``TYPE* VALUE = ...``).
* Static array syntax (``TYPE [ VALUE ]``) conflicts with indexing syntax (``VALUE [ VALUE ]``).

In some cases the parser is unable to resolve ambiguities so it generates these in the AST and they are then later resolved by Semantic Analysis.

This is implemented using a handwritten recursive descent parser (Clang and GCC also have handwritten recursive descent parsers), for the following reasons:

* Easier to produce good error messages.
* Simpler to understand/modify.
* More amenable to targeted optimisation.
* Implemented without additional dependencies.
* Doesn't require auto-generated header files.

The parser was previously a GLR (Generalised LR) parser generated by 'Bison'. However a rewrite was undertaken to switch from the Bison-generated parser to the recursive descent parser in order to:

* Improve error messages.
* Handle difficult cases, such as ``>>`` ending template arguments.
* Split the parser into multiple files (Bison required having a single large ``Parser.y`` file).
* Remove the constraint of using a C union to hold parse tree information.
* Track comments linked to types/functions.
* Put token information in pre-written header files, rather than having them generated by Bison.

Semantic Analysis
~~~~~~~~~~~~~~~~~

This stage essentially recurses through the AST, while looking up names in the context that they appear, and then creating direct references from a use to the corresponding declaration. Developers can of course make mistakes, so this stage will emit errors if uses and declarations don't match up.

The casting rules are also checked here, which involves:

* 'Dissolving' lvals.
* De-referencing refs.
* Calling *implicitCopy* to copy objects where required.
* Performing implicit casts by calling the appropriate methods.
* Performing polymorphic casts.

In addition to this, Semantic Analysis performs strict checks on the AST as required by Loci, such as:

* Ensuring that re-throw statements do not appear in a nested try block.
* Verifying that noexcept functions (including destructors) cannot throw exceptions.
* Ensuring that non-void functions return a value for all code paths.
* Checking that there are no dead code paths (as much as can be checked by a static analysis).
* Looking at scope(exit|failure|success) blocks to ensure they follow they aren't escaped incorrectly.

Semantic Analysis also implements convenience features of the language, such as generating default methods and producing debugging information for use by Code Generation.

Code Generation
~~~~~~~~~~~~~~~

Once the source code has been manipulated and verified by Semantic Analysis, it's in a suitable form to generate LLVM IR instructions. This is achieved with a lot of help from LLVM's libraries, which assist in generating sequences of IR ('IRBuilder'), debug information ('DIBuilder') etc. There's also a library for generating human readable and binary IR representations, and utilities to verify that functions generated by the Loci compiler are correct.

That said, there are some significant Loci-specific complexities that this stage has to overcome. These include:

* Generating vtables for classes as required.
* Producing 'template generator' functions.
* Creating stubs to provide compatibility between different function signatures (which are typically different due to template instantiations involving primitive types).
* Mangling function/type names.
* Generating load/store operations for class declarations (i.e. types with unknown size).
* Producing method implementations for primitive types.
* Encoding/decoding function parameters and function types according to the target ABI.
* Performing virtual method calls and generating method stubs.
* Generating unwind sequences that are correct for constructs such as scope(exit|failure|success) and control flow operations.
* Calling the exception runtime functions as required (including generating landing pads).
* Setting IR debug information that's given by Semantic Analysis.

Much work has focused on (successfully) optimising this stage, so that many of these operations are only performed as necessary.

