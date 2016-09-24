Loci - Programming Language
===========================

Version 1.5 is now available! See :doc:`Releases <Releases>` for more information.

.. toctree::
	:maxdepth: 1
	
	GettingStarted
	Releases

Introduction
------------

Loci is a multi-paradigm systems programming language. Or, to describe it in a slightly more intuitive way, it’s very similar to, and a close competitor of, C++.

It’s a language that aims to not only fix many of the problems that plague C++, but also to introduce whole new paradigms and programming styles that are extremely useful for developers. Alongside that, of course, the language aims to have no performance overhead versus C and C++.

Before going further, here’s some example code:

.. code-block:: c++

	class ExampleClass(int a, int b) {
		// Constructor with default implementation.
		static create = default;
		
		int getA() const noexcept {
			return @a;
		}
		
		int getB() const noexcept {
			return @b;
		}
	}
	
	interface ExampleInterface {
		int getA() const;
	}
	
	void printA(const ExampleInterface& examplePoly) {
		printf(C"A equals %d.", examplePoly.getA());
	}
	
	int main() {
		auto exampleInst = ExampleClass(1, 2);
		printA(exampleInst);
		return 0;
	}

The output is:

::

	A equals 1.

So hopefully the first thing that strikes you is that the language shares the look and feel of C++. On the other hand you’ll also notice differences that are designed to enhance the development experience, such as :doc:`Structural Typing <StructuralTyping>`, which allows a class instance to be casted to an interface instance as long as the class provides all the methods required by the interface.

This means the end of explicit declarations of polymorphic inheritance, which can be particularly valuable if you’re hoping to create an interface after the classes that implement it have already been written (or, e.g. are in the standard library). Note there’s :doc:`almost always no performance penalty <DynamicDispatch>` for this feature.

Features
~~~~~~~~

But this is really just a drop in the ocean and it would take many pages to describe the language in detail, as you can see if you look at the language documentation. So here’s a brief summary of the key features Loci offers (in addition to the above):

* :doc:`Multi-pass compilation <MultiPassCompilation>` - C++ developers regularly struggle with the need for symbol declarations to appear before their use (except for the few cases where this isn’t true); this just isn’t a problem in Loci, which can match up declarations and uses regardless of their relative ordering.
* :doc:`Modules <Modules>` - groups of files can be combined as 'modules', on the order of which it’s possible to express exported and imported symbols (via the ‘export’ and ‘import’ keywords). It’s also a great way to enable more inter-procedural optimisations across source files.
* :doc:`Templates <Templates>` - just like C++, you can automatically generate functions and types by creating templates. Unlike C++, you can express your type requirements via interfaces, and template type declarations can be used as part of module APIs.
* :doc:`Primitives are objects <PrimitiveObjects>` - other languages tend to encourage the notion that primitives are separate from user defined types, but Loci breaks down this barrier. This means you can call methods on primitive types (e.g. (-1).abs();), and cast primitives to interfaces in the same way as user defined types. Yet again there’s no performance penalty for this feature.
* :doc:`Move by default, copy by choice <MoveVersusCopy>` - C++ expects every type to have a copy constructor, generating one if none is specified. Loci is completely different, allowing moves on any type by default, but only allowing a type to be copied if it has a copy method.
* :doc:`Algebraic Datatypes <AlgebraicDatatypes>` - with Loci, it’s possible to easily build data values, and to then pattern-match those values. This therefore provides an efficient and type-safe alternative to typical combinations of enums, structs and unions.

The last point there sounds a little cryptic, but in fact :doc:`Algebraic Datatypes <AlgebraicDatatypes>` are a really simple and powerful concept, and the basis of functional programming. Here’s an example:

.. code-block:: c++

	datatype Optional = Something(int value) | Nothing;
	
	int getValue(Optional optional) {
		switch (optional) {
			case Something(int v) {
				return v;
			}
			case Nothing {
				// If value is 'nothing', return a meaningful number...
				return 42;
			}
		}
	}
	
	int main() {
		printf(C"Something value is %d.\n", getValue(Something(256)));
		printf(C"Nothing value is %d.\n", getValue(Nothing));
		return 0;
	}

As you probably expect, this prints:

::

	Something value is 256.
	Nothing value is 42.

This is a really basic example, but it shows the elegance of this solution in comparison to using combinations of enums, structs and unions. In fact, this solution has the further technical advantage that it can call type destructors (since the compiler is aware of the internal enum), and you can therefore put arbitrarily complex types in your datatypes.

Language
--------

.. toctree::
	:maxdepth: 1
	
	LanguageGoals
	CompatibilityWithC
	AlgebraicDatatypes
	AssertStatement
	Classes
	Const
	ControlFlow
	Exceptions
	Hashing
	ImplicitOperations
	LifetimeMethods
	Literals
	LvaluesAndRvalues
	Modules
	MoveVersusCopy
	ObjectMemoryStates
	OperatorOverloading
	Predicates
	PrimitiveObjects
	Ranges
	References
	StructuralTyping
	Templates
	UnusedValues
	ValueGenerators

Standard Library
----------------

.. toctree::
	:maxdepth: 1
	
	Concurrency
	Containers
	HeapMemoryManagement
	Strings
	Tuples

Compiler
--------

.. toctree::
	:maxdepth: 1
	
	CompilerDesign
	DynamicDispatch
	LLVMIntro
	MultiPassCompilation
	NameMangling
	TemplateGenerators
	VtableGeneration

Planned Features
----------------

These features are currently not implemented but are planned for upcoming releases.

.. toctree::
	:maxdepth: 1
	
	InteractingWithC++
	NamedParameters
	RTTI
	VectorTypes

Additional Topics
-----------------

.. toctree::
	:maxdepth: 1
	
	ClassInheritance
	FunctionOverloading
	GarbageCollection

