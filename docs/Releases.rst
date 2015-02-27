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

* :doc:`Require and Const predicates <Predicates>`
* :doc:`move_to methods <MoveVersusCopy>`
* Fixing :doc:`Const <Const>` rules
* Clarifying/defining :doc:`implicit casting rules <ImplicitOperations>`
* :doc:`C enums and unions <CompatibilityWithC>`
* Statically checked :doc:`exception specifications <Exceptions>`
* :doc:`Vectorised types <VectorTypes>`
* :doc:`Unused variables and values <UnusedValues>` (replacing void cast syntax)

**Standard library**

* Improvements to :doc:`Standard library strings <Strings>`
* Standard library fibers and threads
* Standard library events and networking

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

Planned for June 2015, version 1.3 aims to provide:

* ARM ABI support
* Lambdas
* Value-templates
* Variadic templates
* Template argument deduction
* User-defined :doc:`reference types <References>`
* User-defined function types
* Automatic generation of :doc:`imports from exports <Modules>`.
* Automatic generation of imports from C header files.

In Consideration
~~~~~~~~~~~~~~~~

The following features have not yet been assigned a release:

* Windows and Mac support
* Null coalescing operator
* Union datatype function 'overloading' (i.e. splitting a function into multiple functions similar to a type-switch)
* Automatic parallelisation through 'spawn' and 'sync'
* Class invariants
* Unit test functions
* Pre-conditions and post-conditions
* Compile-time introspection
* Run-time 'reflection'
* Compile-time checking of assertions, invariants, pre-conditions and post-conditions

Development
-----------

The Loci Compiler Tools are under active development in `this repository <https://github.com/scross99/locic>`_. You can checkout the latest version by:

.. code-block:: bash

	git clone https://github.com/scross99/locic.git

You can then follow the :doc:`Getting Started Guide <GettingStarted>` to build the compiler.
