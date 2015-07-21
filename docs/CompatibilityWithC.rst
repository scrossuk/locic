Compatibility with C
====================

Loci is highly related and compatible with C. It therefore supports:

* :doc:`Primitive types <PrimitiveObjects>` (e.g. 'int').
* Pointer types.
* :ref:`Struct types <structs>`.
* :ref:`Enum types <enums>`.
* :ref:`Union types <unions>`.
* Function pointer types.
* :ref:`C's calling convention, so it can call and be called from C functions <calling_to_from_c>`.
* :ref:`C Strings <c_strings>`.

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
	using NewName = (*)(ReturnType)(ParamType); // Loci

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

.. _structs:

Structs
-------

As shown above, Loci supports C's struct types. For example:

.. code-block:: c

	struct Point {
		int x;
		int y;
	}

Like everything else, structs are actually just objects, the practical impact being that structs have:

* A default :ref:`constructor <class_constructors>`: ``auto point = Point(1, 2);``
* A default :ref:`implicit copy method <implicit_copy_methods>`: ``auto point = other_point;``
* A default :ref:`compare method <compare_methods>`: ``if (point == other_point) { ... }``
* Other obvious requirements, such as having alignment/size.

Opaque Structs
~~~~~~~~~~~~~~

Opaque structs can be defined similar to C:

.. code-block:: c

	struct OpaqueStruct;

Unlike normal structs opaque structs don't have any methods and can only be passed around by pointer/reference. This is typically useful as a well-typed alternative to ``void*`` when handling an externally created and managed struct.

.. _enums:

Enums
-----

Loci supports C's enums. For example:

.. code-block:: c

	enum Color {
		RED,
		GREEN,
		BLUE
	}

This actually effectively builds an object type (in Loci all values are essentially objects, with some internal state and a set of methods). You can construct values using the automatically generated constructors:

.. code-block:: c++

	Color function() {
		return Color.RED();
		// Or: Color::Red() if you prefer that.
	}

.. _unions:

Unions
------

Loci supports C's unions, though as in C care must be taken with this particular feature. Here's an example:

.. code-block:: c

	union IntOrFloat {
		int intValue;
		float floatValue;
	}

In this case there is a default ('create') constructor that zero-initialises the union.

.. code-block:: c++

	IntOrFloat function() {
		auto value = IntOrFloat();
		value.intValue = 100;
		return value;
	}

This feature exists for compatibility with C and it is **strongly** advised in the vast majority of cases to use :doc:`Algebraic Datatypes <AlgebraicDatatypes>` as a safer alternative.

.. _calling_to_from_c:

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

Manually calling into Loci
~~~~~~~~~~~~~~~~~~~~~~~~~~

Unlike C++, you can absolutely call Loci functions/methods directly from C by applying the appropriate :doc:`Name Mangling <NameMangling>`. For example, consider calling into this function:

.. code-block:: c++

	namespace Namespace {
		void function();
	}

This would be achieved by:

.. code-block:: c

	void cfunction() {
		F2N9NamespaceN8function();
	}

There's nothing wrong with this and indeed it facilitates effective compatibility with C (and all the other languages that are also compatible with C). This gets a little more complex if the function returns a class type:

.. code-block:: c++

	namespace Namespace {
		class TestClass {
			void method();
			
			// etc.
		}
		
		TestClass function();
	}

The C code then must allocate the necessary space for the class and pass a pointer to this as the first parameter. This is required because the size of classes are not in general known until run-time, which facilitates omitting . You can query the size/alignment of the class by calling the relevant functions:

.. code-block:: c

	void cfunction() {
		const size_t size = MT2N9NamespaceN9TestClassF1N8__sizeof();
		
		// Not needed in the case of heap allocation, but queried for completeness.
		const size_t alignMask = MT2N9NamespaceN9TestClassF1N11__alignmask();
		
		// A heap allocation isn't necessary - Loci-generated code uses
		// stack allocations - but it's simpler to demonstrate here.
		void* objectPointer = malloc(size);
		
		// The function will write to the given pointer.
		F2N9NamespaceN8function(objectPointer);
		
		// Call a method on the class.
		MT2N9NamespaceN9TestClassF1N6method(objectPointer);
		
		// Must call class destructor!
		MT2N9NamespaceN9TestClassF1N9__destroy(objectPointer);
		
		free(objectPointer);
	}

An alignment mask is just the alignment (which is always a power of 2) minus one, which is useful because calculating the maximum alignment of a set of fields (e.g. when computing the alignment of a class) just involves a bitwise OR of the alignment masks and then adding one.

It's worth noting at this point that the mangling and method names are not yet fully standardised but that it is expected this will occur soon (i.e. version 1.3).

.. _c_strings:

C Strings
---------

Loci supports C strings, which essentially just involves manipulating pointers to *ubyte* (the type *char* is renamed to *byte* and Loci treats ASCII character bytes as unsigned). For example:

.. code-block:: c++

	size_t get_cstring_length(const ubyte* ptr) {
		size_t length = 0u;
		while (*ptr != 0u) {
			length++;
			ptr++;
		}
		return length;
	}

Needless to say, it's recommended to use standard library :doc:`Strings <Strings>` rather than trying to manipulate C strings (an extremely error-prone process).

Note that C string :doc:`literals <Literals>` must use the 'C' prefix or suffix:

.. code-block:: c++

	// Prefix:
	const size_t length = get_cstring_length(C"Hello world!");
	
	// Suffix:
	const size_t length = get_cstring_length("Hello world!"C);

Without the prefix or suffix Loci will try to find a function called 'string_literal', which conveniently gives std.string a hook to provide a standard library string when no prefix/suffix is specified.

