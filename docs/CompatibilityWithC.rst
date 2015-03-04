Compatibility with C
====================

Loci is highly related and compatible with C. It therefore supports:

* Primitive types (e.g. 'int').
* Pointer types.
* Struct types.
* Enum types.
* Union types.
* Function pointer types.
* C's calling convention, so it can call and be called from C functions.

Syntax
------

There are however, some minor syntactic changes. For example, struct type names should not be prefixed with the keyword 'struct' (except in their definition):

.. code-block:: c++

	struct ExampleType {
		int v;
	};
	
	// Invalid.
	struct ExampleType function();
	
	// Valid.
	ExampleType function();

This syntactic change applies similarly for enum and union types. The intention, clearly, is to reduce the verbosity of the code and therefore the burden on the programmer to write it.

Casts also use a different syntax:

.. code-block:: c++

	void function(){
		float pi = 3.141592654;
		
		// Invalid.
		int intPi = (int) pi;
		
		// Valid.
		int intPi = cast<int>(pi);
	}

The new syntax is much clearer, helping readers to understand exactly what value is being cast, since the precedence of the type cast may not be obvious (few programmers remember all, or close to all, of their language's operator precedences). Consider this example in C++:

.. code-block:: c++

	void function(Type * ptr, Type::*function()){
		// As one expects...
		int i = (int) ptr->member;
		
		// ...is equivalent to this.
		int i = ((int) ptr->member);
		
		// But this...
		int i = (int) ptr->member + 1;
		
		// ...is equivalent to this.
		int i = ((int) ptr->member) + 1;
		
		// And this...
		int i = (int) ptr->*function();
		
		// ...would be equivalent to this.
		int i = ((int) ptr)->*function();
		
		// (Which is of course broken.)
	}

Semantics
---------

There are also some minor semantic changes, in particular to the available implicit casts:

.. code-block:: c++

	void function(){
		float pi = 3.141592654f;
		
		// Invalid - no implicit cast.
		int intPi = pi;
		
		// Valid - explicit cast is allowed.
		int intPi = cast<int>(pi);
		
		// Invalid - no implicit cast from 'void *' to 'int *'.
		int * intPtr = malloc(sizeof(int));
		
		// Valid.
		int * intPtr = reinterpret_cast<int*>(malloc(sizeof(int)));
	}

In this case, the intention is to make it clear to a reader when values are being cast to fundamentally different types (such as an integer and a floating point value). The 'cast' operator supports legal implicit and explicit casts, whereas the 'reinterpret_cast' operator allows casts between pointers whose target types (i.e. the type being pointed-to) are different.

Type Syntax
-----------

Loci follows the following structure for all typed variable declarations:

.. code-block:: c

	varDecl ::= TYPE NAME

This differs significantly from C, in which types and names can overlap in confusing ways, such as in function pointer types:

.. code-block:: c

	void (*f)(); // C
	(*)(void)() f; // Loci
	
	int (*f)(); // C
	(*)(int)() f; // Loci
	
	int (*f)(int, int); // C
	(*)(int)(int, int) f; // Loci

Similarly, typedefs are much clearer in Loci:

.. code-block:: c

	typedef ReturnType (*NewName)(ParamType param); // C
	typedef (*)(ReturnType)(ParamType) NewName; // Loci

Some types (such as structs and datatypes) also support pattern matching, with the following (rough) structure:

.. code-block:: c

	pattern ::= '_' /* wildcard */
	pattern ::= varDecl
	pattern ::= NAME '(' patternList ')'
	
	nonEmptyPatternList ::= pattern
	nonEmptyPatternList ::= nonEmptyPatternList ',' pattern
	
	patternList ::= /* empty */
	patternList ::= nonEmptyPatternList

This facilities code like the following:

.. code-block:: c++

	struct Example {
		int x;
		int y;
	};
	
	void function(Example value) {
		Example(int x, _) = value;
	}

Calling to/from C
-----------------

All Loci functions/methods are generated to use the standard C calling convention on the target platform. This means it's trivial to call to/from C code. For example:

.. code-block:: c

	struct CStruct {
		int value;
	};
	
	void doSomeProcessingInC(struct CStruct* data) {
		data->value = 42;
	}

If this is some C code, then you can call into it from Loci with essentially identical code:

.. code-block:: c++

	struct CStruct {
		int value;
	};
	
	void doSomeProcessingInC(CStruct* data) noexcept;

Note that the :doc:`noexcept qualifier <Exceptions>` has been added to the function in Loci; this is not required but clearly represents the fact that the C function won't throw an exception and this aids static analysis of exception safety in Loci.
