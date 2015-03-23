Releases
========

Available Releases
------------------

This section lists completed releases of the Loci Compiler Tools, in reverse version order.

v1.2
~~~~

Available here: [TODO]

Released on [TODO] March 2015, version 1.2 provides:

**Language features**

* :doc:`const, noexcept and require predicates <Predicates>`
* Boolean template values (for use in predicates)
* :doc:`__moveto methods <MoveVersusCopy>`
* Fixing :doc:`Const <Const>` rules (in particular adding transitive const)
* :doc:`C enums and unions <CompatibilityWithC>`
* :doc:`Unused variables and values <UnusedValues>` (replacing void cast syntax)
* :doc:`Assert noexcept <Exceptions>`

**Standard library**

* UTF-8 support for :doc:`Standard library strings <Strings>`
* Threads
* Events
* Networking (just IPv4 and TCP for this release)

**Compiler**

* Much easier for end users to build (now builds against LLVM Debian packages)
* Added 'Array', 'StableSet' and other containers for improved performance
* Added 'String' class for uniquifying strings
* Refactored support code into 'support' library
* Substantially improved Semantic Analysis performance
* Eliminated unnecessary generation of vtables and template generators for primitive cast/implicit_cast methods
* Fixed various CodeGen issues by enforcing stricter rules in SEM for binding references
* Fixed ownership semantics for various SEM types
* Added pre-compiled header for LLVM

**Examples**

* Added 'Simulations' example
* Removed 'General' example

**Documentation**

* Re-focused various pages to provide most important/relevant information at the start
* Added :doc:`Predicates <Predicates>` page
* Re-wrote much of :doc:`Const <Const>` page
* Re-wrote much of :doc:`Modules <Modules>` page, with a new diagram of the compilation model

**Test**

* Larger set of tests (e.g. for C enums and unions)
* New 'Check Success' tests ensure code compiles without having to run JIT
* Added some initial unit tests

**Other**

* Continuous Integration - verifying support for LLVM 3.3/3.4/3.5/3.6

**Dependencies**

* Tested to build and work with LLVM 3.3/3.4/3.5/3.6

v1.1
~~~~

Available here: `locic-1.1.src_.tar.gz <http://loci-lang.org/releases/locic-1.1.src_.tar.gz>`_

Released on 27th August 2014, version 1.1 provides:

**Language features**

* Switching from C++-like :doc:`template <Templates>` expansion to use :doc:`Template Generators <TemplateGenerators>` (to allow templated APIs across module boundaries)
* :doc:`Module imports and exports <Modules>`
* :doc:`scope(success), scope(failure) and scope(exit) <Exceptions>`
* :doc:`noexcept <Exceptions>`
* :doc:`Type-templated functions/methods <Templates>`
* Type aliases
* :doc:`assert and unreachable statements <AssertStatement>`
* :doc:`Implicit and explicit casts <ImplicitOperations>` between types using templated methods

**Standard library**

* :doc:`Standard library memory allocators and smart pointers <HeapMemoryManagement>`
* Standard library containers
* :doc:`Standard library strings <Strings>`

**Other**

* Vastly improved performance, particularly for :doc:`Code Generation <CompilerDesign>`.
* A larger set of examples and updates to examples to demonstrate newly implemented features.
* Significantly improved documentation in reStructuredText using Sphinx, which can generate multiple output formats including HTML and PDF.
* A much larger set of integrated tests to check both accept and reject cases, as well as testing the standard library.

**Dependencies**

* Tested to build and work with LLVM 3.3/3.4/3.5

v1.0
~~~~

Available here: `locic-1.0.src_.tar.gz <http://loci-lang.org/releases/locic-1.0.src_.tar.gz>`_

Released on 6th April 2014, version 1.0 provides:

* :doc:`Standard integer/floating point primitives <PrimitiveObjects>`
* C structs
* Functions (C-compatible)
* :doc:`Multi-pass compilation <MultiPassCompilation>`, allowing symbols declarations and uses to appear in any order
* :doc:`Class declarations and definitions <Classes>`
* Static and dynamic methods
* Default constructors (using ‘= default’ syntax)
* Type deduction for local variables (using ‘auto’ keyword)
* :doc:`Exceptions <Exceptions>`, including exception hierarchies and try-catch
* Destructors (exception-safe)
* :doc:`Interfaces <StructuralTyping>`, including polymorphic casts and virtual calls
* :doc:`Algebraic datatypes <AlgebraicDatatypes>`, including union datatypes
* Type switch on datatypes
* Pattern matching datatypes
* :doc:`Class and interface templates <Templates>`
* :doc:`lval <LvaluesAndRvalues>` and :doc:`ref <References>` support, including implicit lval dissolve
* Implicit lval generation (value_lval for local variables, member_lval for member variables)
* :doc:`lval operations <LvaluesAndRvalues>`, including address, assign, dissolve and move
* :doc:`const methods and cast const-correctness <Const>`
* null, including null constructors for user-defined types
* :doc:`Integer, floating point and C string literals <Literals>`
* Method name canonicalization

Future Releases
---------------

This section lists planned releases of the Loci Compiler Tools, in reverse version order.

v1.3
~~~~

Planned for Summer 2015, version 1.3 aims to provide:

**Language features**

* Named predicates
* Value-templates
* Variadic templates
* Template argument deduction
* User-defined :doc:`reference types <References>`
* User-defined function types
* Lambdas

**Standard library**

* Standardise APIs for version 1.0.0
* Fix std::map implementation (std.container)
* Hash table set and map (std.container)
* Files and directories (std.filesystem)
* DNS resolution (std.network)
* IPv6 (std.network)
* UDP (std.network)
* Points, Vectors, Matrices (std.geometry)
* Fibers (std.concurrency)
* Binary search, sorting (std.algorithm)
* URL creation/parsing (std.url)
* Endianness (std.buffer?)

**Compiler**

* Clarifying/defining :doc:`implicit casting rules <ImplicitOperations>` - mostly related to improving Semantic Analysis
* Move default method generation into CodeGen
* Add CMake module files to make it easier to create Loci projects

**Primitives**

* Function/method/interface method types
* Statically sized array
* :doc:`Vectorised types <VectorTypes>`
* Min, max and range

**Tools**

* Generation of :doc:`imports from exports <Modules>`
* Generation of Loci imports from C (and potentially C++) headers

In Consideration
~~~~~~~~~~~~~~~~

The following features have not yet been assigned a release:

**Language Features**

* Union datatype function 'overloading' (i.e. splitting a function into multiple functions similar to a type-switch)
* Statically checked :doc:`exception specifications <Exceptions>`
* Unit test functions
* Enum raw type specification (e.g. an enum based on a float type)
* Automatic parallelisation through 'spawn' and 'sync'
* Class invariants
* Pre-conditions and post-conditions
* Compile-time introspection
* Run-time 'reflection'
* Compile-time checking of assertions, invariants, pre-conditions and post-conditions
* Null coalescing operator

**Standard Library**

(Many of these will be APIs over existing 3rd-party libraries.)

* Complex Numbers (std.numeric)
* Infinite precision arithmetic (std.numeric)
* Random number generation (std.numeric)
* Precise time measurement (std.chrono)
* Flyweights (std.flyweight?)
* Radix tree (std.container)
* Regular expressions (std.regex)
* Dates (std.date)
* Character encoding translations (std.string)
* Localisation (std.string)
* Function objects (std.function?)
* Garbage collection (std.memory)
* Cryptography (std.crypto?)
* HTTP client (std.http?)
* GUI (std.gui?)
* Interprocess communication (std.process?)

**Compiler**

* ARM ABI support
* Separate llvm-abi into separate project
* Windows and Mac support
* Multiple error message reporting
* Improved/standardised error messages
* Emit TBAA (Type Based Alias Analysis) information
* Javascript-based build (e.g. with Emscripten) for demonstration purposes

**Primitives**

* Atomic operations

**Tools**

* Verify imports and exports against each other
* Benchmarks of language features
* Generate C and C++ headers from Loci imports

**Examples**

* Remove 'events' and 'network' examples (probably turn them into tests)
* Ogre3D based example
* Add Qt5-based instant messaging example

Development
-----------

The Loci Compiler Tools are under active development in `this GitHub repository <https://github.com/scross99/locic>`_. You can checkout the latest version by:

.. code-block:: bash

	git clone https://github.com/scross99/locic.git

You can then follow the :doc:`Getting Started Guide <GettingStarted>` to build the compiler.

There is also a continuous integration build job at `Travis CI <https://travis-ci.org/scross99/locic>`_.

Queries/Suggestions
-------------------

This project is being developed by `Stephen Cross <http://scross.co.uk>`_.

Contributions, queries, suggestions and feedback are all very welcome; currently the best thing to do is `raise an issue on GitHub <https://github.com/scross99/locic/issues>`_.
