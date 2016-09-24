Releases
========

Available Releases
------------------

This section lists completed releases of the Loci Compiler Tools, in reverse version order.

v1.5
~~~~

Available here: `locic-1.5.src.tar.xz <http://loci-lang.org/releases/locic-1.5.src.tar.xz>`_

Released on 24th September 2016, version 1.5 provides:

**Compiler**

* Added more links between AST and SEM for efficiency and future merge
* Added ``SEM::Statement::For()`` to (in future) avoid generating loops in Semantic Analysis
* Fixed various error messages to be more consistent (e.g. start with a lowercase letter)
* Added ``size_t`` and ``ssize_t`` to valid vararg types
* Added error for non-type arguments to capability test (``A : B``)
* Fixed duplicate type errors to handle clashes with non-type objects
* Print 'previously defined here' message for function name clash
* Moved more code generation to use ``IREmitter`` rather than using LLVM's ``IRBuilder`` directly

**Test**

* Added ``check`` target that builds and tests the compiler
* Various improvements to duplicate object tests
* Added tests for covariant/contravariant casts
* Moved 'CheckSuccess' tests into library-specific directories (e.g. ``test/SemanticAnalysis``)
* Added currently-disabled tests for class inheritance
* Added test for ``std::string`` ``length()`` and ``empty()``
* Added test for destroying temporary in ternary value
* Added tests for ``std::path``
* Added test for ``unichar`` literal

**Language features**

* Added ``override`` and ``inherit`` keywords to lexer and parser; class inheritance is **not** yet implemented
* Character literals now use new ``unichar`` type

**Primitives**

* Added ``unichar`` type (full type name is ``unichar_t``)
* Added ``hash()`` methods for all integer primitives and ``typename_t``
* Added more cast methods for various primitive types
* Implemented copy methods for ``null_t``

**Standard library**

* Added ``std::filesystem::file_stream`` and ``std::filesystem::path``
* Added ``empty()`` method for ``std::string``
* ``std::hash<>`` now works for ``int``; not yet extended to work with all types
* Fixed ``std::varray`` to use ``reverse_range<>`` rather than ``reverse_counter<>``

**Documentation**

* Added documentation for hashing support

**Dependencies**

* Tested to build and work with LLVM versions 3.3 to 3.9
* ``llvm-abi`` is now a Git submodule rather than being fetched from a URL by CMake

v1.4
~~~~

Available here: `locic-1.4.src.tar.xz <http://loci-lang.org/releases/locic-1.4.src.tar.xz>`_

Released on 25th April 2016, version 1.4 provides:

**Compiler**

* New and vastly better compiler diagnostics system.
* Rewritten lexer and parser to provide improved diagnostics (see http://scross.co.uk/2016/01/30/glr-vs-recursive-descent-parsing/ ).

**Test**

* All tests have now been moved to Lit (from CTest).
* Many more tests for diagnostics (tests added as diagnostics were converted to new system).
* New unit tests for lexer/parser.

**Language features**

* Renamed ``__empty`` :doc:`lifetime method <LifetimeMethods>` to ``__dead``.

**Primitives**

* Added ``min()`` and ``max()`` built-in functions.
* Added built-in :doc:`value generators <ValueGenerators>` ``range()``, ``range_incl()``, ``reverse_range()`` and ``reverse_range_incl()``.
* Removed ``is_`` prefix from built-in predicates (e.g. ``is_movable`` is now just ``movable``).
* Renamed ``ptr_lval`` to ``ptr_lval_t`` for consistency with other primitives.
* Added initial code for allowing hashable types.

**Standard library**

* Added initial code for hashing types (to be used in hash set/map implementations in next version).

v1.3
~~~~

Available here: `locic-1.3.src.tar.xz <http://loci-lang.org/releases/locic-1.3.src.tar.xz>`_

Released on 8th November 2015, version 1.3 provides:

**Language features**

* Added named predicates (or 'predicate aliases')
* Added ``__sizeof`` and ``__alignmask`` methods.
* Added support for opaque structs.
* Added :doc:`LifetimeMethods`
* Clarified :doc:`ObjectMemoryStates` (enables moving with all lvalues)
* Added ``static assert`` statement.
* Fixed various aspects of value templates support.
* Constrained operator combinations to avoid precedence confusion (see :doc:`OperatorOverloading`).
* Allowed ``require`` to be both before and after function declaration for templated functions.
* Fixed ``noexcept`` predicates value inference.
* Added syntax support for comparisons in function parameters without needing parentheses.
* Fixed method call and assignment evaluation order.

**Primitives**

* Added primitive function/method/interface-method pointer types.
* Added primitive static array type.

**Standard library**

* Fixed ``std.concurrency`` implementation to be more stable and portable.
* Modified ``std.container`` to use named predicates.
* Fixed ``std.memory`` in respect to Lifetime Methods.

**Compiler**

* Substantially improved llvm-abi API and backend (lots of code brought in from Clang) and integrated this into compiler.
* Moved default method generation (for 'implicitcopy', 'copy' etc.) into CodeGen.
* Modified CodeGen to emit opaque pointer types and typed pointer operations to faciliate upstream LLVM opaque pointer changes.
* Refactored CodeGen primitives into separate classes.

**Test**

* Added tests using LLVM's 'lit' to verify compiler's IR output.
* Added lit-based tests to check ABI correctness.
* Added new tests for all new/improved features mentioned above.
* Added many new syntax tests to check parser.
* Added tests for ``std.concurrency``.
* Added tests for primitives.
* Added tests for sizes of empty objects.
* Added tests for destroying temporary objects.
* Added tests for evaluation order.

**Other**

* Documentation now automatically uploaded to `website <http://loci-lang.org>`_.
* Build artifacts now automatically uploaded to `website (/travis) <http://loci-lang.org/travis/>`_.

**Dependencies**

* Tested to build and work with LLVM versions 3.3 to 3.7
* ``nest`` LLVM `ARM <http://reviews.llvm.org/D11126>`_ and `AArch64 <http://reviews.llvm.org/D10585>`_ patches accepted into LLVM 3.7 in preparation for fast virtual call implementation.

v1.2
~~~~

Available here: `locic-1.2.src.tar.xz <http://loci-lang.org/releases/locic-1.2.src.tar.xz>`_

Released on 28th March 2015, version 1.2 provides:

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

* `Continuous Integration <https://travis-ci.org/scross99/locic>`_ - verifying support for LLVM 3.3/3.4/3.5/3.6

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

v1.5
~~~~

Versions 1.5 aims to provide:

**Language features**

* Improve template argument deduction

**Primitives**

* Add static array comparison support.

**Standard library**

* Fix std::map implementation (std.container)
* Hash table set and map (std.container)
* Files and directories (std.filesystem)
* DNS resolution (std.network)
* IPv6 (std.network)
* UDP (std.network)
* Binary search, sorting (std.algorithm)

**Compiler**

* Add CMake module files to make it easier to create Loci projects
* Improve emitted debug information

**Tools**

* Generation of :doc:`imports from exports <Modules>`

**Test**

* Tests for primitives (e.g. integer overflow)
* More standard library tests

In Consideration
~~~~~~~~~~~~~~~~

The following features have not yet been assigned a release:

**Language Features**

* Fix function pointer ABI issues
* Variadic templates
* Lambdas
* User-defined :doc:`reference types <References>`
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
* Named parameters

**Primitives**

* :doc:`Vectorised types <VectorTypes>`
* Atomic operations

**Standard Library**

(Many of these will be APIs over existing 3rd-party libraries.)

* Standardise APIs for version 1.0.0
* Points, Vectors, Matrices (std.geometry)
* URL creation/parsing (std.url)
* Endianness (std.buffer?)
* Fibers (std.concurrency)
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

* Implement 'native' virtual calls on LLVM 3.6+.
* Clarifying/defining :doc:`implicit casting rules <ImplicitOperations>` - mostly related to improving Semantic Analysis
* ARM ABI support
* Windows and Mac support
* Multiple error message reporting
* Improved/standardised error messages
* Emit TBAA (Type Based Alias Analysis) information
* Javascript-based build (e.g. with Emscripten) for demonstration purposes

**Tools**

* Generation of Loci imports from C (and potentially C++) headers
* Verify imports and exports against each other
* Benchmarks of language features
* Generate C and C++ headers from Loci imports

**Examples**

* Remove 'events' and 'network' examples (probably turn them into tests)
* Ogre3D based example
* Add Qt5-based instant messaging example
