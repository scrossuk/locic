Releases
========

Available Releases
------------------

This section lists completed releases of the Loci Compiler Tools, in reverse version order.

v1.1
~~~~

Released on 23rd August 2014, version 1.1 provides:

**Language features**

* Code Generation fix for :doc:`templates <Templates>` (to replace C++-like template expansion with a single representation for each class, to allow templated APIs across module boundaries)
* :doc:`Module imports and exports <Modules>`
* :doc:`scope(success), scope(failure) and scope(exit) <Exceptions>`
* :doc:`noexcept <Exceptions>`
* :doc:`Type-templated functions/methods <Templates>`
* Type aliases
* :doc:`assert and unreachable statements <AssertStatement>`
* Implicit and explicit casts between types using templated methods

**Standard library**

* :doc:`Standard library memory allocators and smart pointers <HeapMemoryManagement>`
* Standard library containers
* :doc:`Standard library strings <Strings>`

**Other**

* Vastly improved performance, particularly for :doc:`Code Generation <CompilerDesign>`.
* A larger set of examples and updates to examples to demonstrate newly implemented features.
* Significantly improved documentation in reStructuredText using Sphinx, which can generate multiple output formats including HTML and PDF.
* A much larger set of integrated tests to check both accept and reject cases, as well as testing the standard library.

v1.0
~~~~

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

v1.2
~~~~

Planned for the end of 2014, version 1.2 aims to provide:

* Clarifying/defining implicit casting rules
* C enums and unions
* :doc:`Vectorised types <VectorTypes>`
* Lambdas
* Value-templates, including lists of types
* Variadic templates
* User-defined :doc:`reference types <References>`
* User-defined function types
* Standard library fibers and threads
* Standard library events and networking
* Null coalescing operator
* Const-templated functions/methods (to allow a function/method to support both const and non-const usage)
* Union datatype function 'overloading' (i.e. splitting a function into multiple functions similar to a type-switch)

In Consideration
~~~~~~~~~~~~~~~~

The following features have not yet been assigned a release:

* Automatic generation of :doc:`imports from exports <Modules>`.
* Automatic generation of imports from C header files.
* Automatic parallelisation through 'spawn' and 'sync'
* Class invariants
* Unit test functions
* Pre-conditions and post-conditions
* Compile-time introspection
* Run-time 'reflection'
* Compile-time checking of assertions, invariants, pre-conditions and post-conditions

