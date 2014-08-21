Primitive Objects
=================

The C standard library provides some basic mathematical routines to perform simple operations:

.. code-block:: c++

	void function(){
		float a = -1.5f;
		float b = fabs(a);
		float c = floor(b);
		float d = sqrt(c);
	}

Through the compatibility with C, these routines clearly remain available in Loci. However, Loci revamps the primitive types to be :doc:`Object Types <Classes>`:

.. code-block:: c++

	void function(){
		float a = -1.5f;
		float b = a.abs();
		float c = b.floor();
		float d = c.sqrt();
	}

These modifications turn the primitive types into object types from the developer's perspective, even though the implementation is identical to C and there is therefore no performance penalty.

Semantics
---------

Syntactically, the change is quite significant, but there is also a considerable semantic difference since primitive references can be interface types, and primitives can satisfy template requirements that are based on their methods. Unlike C++, and surprisingly even Java, Loci provides a neat way to integrate primitives with other object types.

Use with templates
~~~~~~~~~~~~~~~~~~

Here's an example of using :doc:`Operator Overloading <OperatorOverloading>` and :doc:`Templates <Templates>` with primitive types as well as user-defined :doc:`Classes <Classes>`:

.. code-block:: c++

	template <typename T>
	interface Comparable {
		compare_result_t compare(const T& other) const;
	}
	
	template <typename T: Comparable<T>>
	class PairSorter (T first, T second) {
		static create = default;
		
		const T& first() const {
			return @first;
		}
		
		const T& second() const {
			return @second;
		}
		
		void sort() {
			if (@first > @second) {
				// Swaps the two values; usually developers
				// would use std::swap.
				T tmp = move @first;
				@first = move @second;
				@second = move tmp;
			}
		}
	}
	
	class UserType(int value) {
		static create = default;
		
		int value() const {
			return @value;
		}
		
		compare_result_t compare(const UserDefinedType& other) const {
			return @value.compare(other.value());
		}
	}
	
	void exampleFunction() {
		auto intSorter = PairSorter<int>(3, 2);
		intSorter.sort();
		printf(C"intSorter: %d, %d\n", intSorter.first(), intSorter.second());
		
		auto userSorter = PairSorter<UserType>(UserType(40), UserType(50));
		userSorter.sort();
		printf(C"userSorter: %d, %d\n", intSorter.first().value(), intSorter.second().value());
	}

So this should print:

::

	intSorter: 2, 3
	userSorter: 40, 50

Polymorphism
~~~~~~~~~~~~

Here's an example using polymorphism via :doc:`Structural Typing <StructuralTyping>` with primitive objects:

.. code-block:: c++

	interface IntAbs {
		int abs() const;
	}
	
	void polymorphicFunction(const IntAbs& value) {
		printf(C"abs() value: %d\n", value.abs());
	}
	
	void exampleFunction() {
		int i = -1;
		int j = 0;
		int k = 1;
		
		polymorphicFunction(i);
		polymorphicFunction(j);
		polymorphicFunction(k);
	}

Which will print:

::

	abs() value: 1
	abs() value: 0
	abs() value: 1

Implementation
--------------

See:

* :doc:`Dynamic Dispatch <DynamicDispatch>`
* :doc:`Vtable Generation <VtableGeneration>`

