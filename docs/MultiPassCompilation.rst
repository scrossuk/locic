Multi-pass Compilation
======================

Semantics
---------

In C and C++
~~~~~~~~~~~~

Consider the following C or C++ code:

.. code-block:: c++

	void functionDefinition() {
		functionDeclaration();
	}
	
	void functionDeclaration();

Unfortunately, this code doesn't build. The problem is that the compiler sees *functionDefinition* before it sees *functionDeclaration*, and the former contains a latter to the reference.

Conceptually, at least, this means C and C++ use single-pass compilation. This means you have to make sure that your function declarations precede any references to them, and similarly for types.

In Loci
~~~~~~~

Consider the equivalent code in Loci:

.. code-block:: c++

	export void functionDefinition() {
		functionDeclaration();
	}
	
	import void functionDeclaration();

In contrast to C or C++, this will build successfully in Loci. In fact, ordering is completely irrelevant in Loci, so you never need to think about the ordering of constructs; your only concern is whether the uses and declarations match (e.g. a function is called with parameters of the correct type).

Implementation
--------------

See :doc:`Compiler Design <CompilerDesign>`.

This feature is actually relatively simple to implement, and in practice C++ compilers use multi-pass compilation (which is partially exposed for symbol uses inside classes). The implementation requirement is for the compiler's Semantic Analysis stage to iterate over the abstract syntax tree multiple times to resolve circular dependencies.

Member Variable Type Resolution
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here's an example:

.. code-block:: c++

	class FirstClass(SecondClass* object) { }
	
	class SecondClass(FirstClass* object) { }

There's nothing wrong with this code, but the compiler needs to be careful to handle the circular dependency between the classes member variable types:

::

	FirstClass -> SecondClass* -> SecondClass -> FirstClass* -> FirstClass -> SecondClass* -> etc.

The solution is:

First pass:

* Create a *TypeInstance* for *FirstClass*.
* Create a *TypeInstance* for *SecondClass*.

Second pass:

* Resolve types of member variables for *FirstClass*.
* Resolve types of member variables for *SecondClass*.

There are a variety of possible valid circular dependencies, and hence Semantic Analysis has a corresponding number of stages to resolve these.

Performance
~~~~~~~~~~~

For a single processor machine, multi-pass compilation could mean slightly longer compile times. However this effect is likely to be outweighed by issues such as quality of implementation (i.e. how much effort is spend on optimisation), and certainly by the use of :doc:`Modules <Modules>` to ensure that a source file is only processed once.

Fortunately, modern build machines are generally multi-core, and so there's the potential for significant gains in running each Semantic Analysis pass in parallel across source files, since each pass only depends on the previous pass. Given that the design of Loci means that constructs are essentially location-independent (and the language issues that force C++ compilers to use multi-pass compilation), in practice this design choice is expected to reduce compile times relative to C++ compilers.

