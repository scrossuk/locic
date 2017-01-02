Primitive Objects
=================

Loci is fundamentally a very simple language, in which all types are objects that have a set of static methods, dynamic methods, some member variables, template variables and a few other properties (it should be emphasised that there is **NO performance penalty** for this design choice; see `Implementation`_).

New object types can be constructed using existing object types, but at the lowest level there are some built-in object types called *primitives*. These include:

* **C integers:** byte (equivalent to C's *char*), short, int, long, long long and unsigned equivalents.
* **C floats:** float, double and long double.
* **Fixed size integers:** int8_t, int16_t, int32_t, int64_t and unsigned equivalents.
* **System-sized integers:** size_t, ptrdiff_t etc.
* **Indirect types:** pointers and references.
* **Lvals:** ptr_lval_t and value_lval_t.
* **Misc:** void, bool, compare_result_t, null_t, typename, 

You can view the methods of these in /runtime in the source tree, where they're declared as primitives.

Methods
-------

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

	template <comparable T>
	class PairSorter(T first, T second) {
		static create = default;
		
		const T& first() const noexcept {
			return @first;
		}
		
		const T& second() const noexcept {
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
		
		int value() const noexcept {
			return @value;
		}
		
		compare_result_t compare(const UserDefinedType& other) const noexcept {
			return @value.compare(other.value());
		}
		
		// Or just use:
		// 
		// compare = default;
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

Extending Primitives
--------------------

Extension Methods
~~~~~~~~~~~~~~~~~

Primitives can be extended in a trivial manner by defining extension methods. These are methods declared/defined outside of an object type which can't access its member variables (hence don't break encapsulation) but are able to augment the object type.

For example:

.. code-block:: c++

	int int_t::add_to_self() const noexcept {
		return self + self;
	}
	
	int function(int i) {
		return i.add_to_self();
	}

This provides a clean way to extend primitive types (or other object types) to support new methods (which may be used to support an existing interface, which is particularly useful with :doc:`Templates <Templates>`) by using the existing set of methods.

Adding new primitives
~~~~~~~~~~~~~~~~~~~~~

One interesting advantage of the structure of Loci is that the set of primitives could be adjusted as needed for a particular use case.

For example, a project could add new primitive types (or new methods to existing primitives) to CodeGen to support additional functionality available in target hardware (though if you do this please do try to push any changes back to the mainline compiler). Similarly, it would be possible to reduce the language by eliminating certain primitives in order to support a particular target or for safety/verification purposes.

New functionality in the language often just involves augmenting primitive types. For example, the upcoming addition of atomics will involve adding new methods to primitive types.

Implementation
--------------

When a primitive method is called, Semantic Analysis sees the call as equivalent to any other call. On the other hand, CodeGen specifically looks for calls to primitive methods and emits them as individual instructions (e.g. add).

For example:

.. code-block:: c++

	int function(int i) {
		return i + i;
	}

There's a call to int_t::add here, with some syntactic sugar provided by :doc:`Operator Overloading <OperatorOverloading>`, meaning Semantic Analysis sees something like:

::

	Function(
		name: function,
		returnType: int,
		parameterTypes: [ int ],
		parameterVars: [ i : int ],
		code: Scope(
			ReturnStatement(
				value: FunctionCall(
					function: method(
						name: int_t::add,
						context: Var(i)
					),
					arguments: [ Var(i) ]
				)
			)
		)
	)

However CodeGen recognises calls to primitive methods and knows to emit something like the following:

.. code-block:: llvm

	define i32 @function(i32) {
		%1 = add i32 %0, i32 %0
		ret i32 %1
	}

(It's not entirely that simple, because CodeGen has to be prepared for modifications to variable 'i', and hence must emit an alloca which will then be optimised out later, but the point is that no function or function call is emitted for the primitive method.)

As shown above it's possible to use primitive types in a polymorphic manner (i.e. cast references to primitives to references to interfaces), and in that case vtable generation will produce a vtable for the primitive types, which will then involve actually emitting methods for the primitive. These methods are marked 'alwaysinline' such that if optimisations later turn virtual calls to primitive methods into direct calls, they will also then inline the method and hence produce code exactly like the above.

However you choose to use primitives you can expect the underlying implementation to always emit the best possible code given the circumstances. For example, if you use :doc:`Templates <Templates>` across a :doc:`Module <Modules>` boundary and pass a primitive type as a template parameter, you can expect the resulting code to use stack-based autoboxing of the primitive values.

If you want to improve on this it's strongly encouraged to use Link Time Optimisation (LTO i.e. link modules together and then optimise the result), which will most likely inline away the templates and hence give you code like the above that operates directly on the primitive type, at the expense of losing the API boundary (which may be required e.g. if you plan to deploy one module as a shared library).

See also:

* :doc:`Compiler Design <CompilerDesign>`
* :doc:`Dynamic Dispatch <DynamicDispatch>`
* :doc:`Template Generators <TemplateGenerators>`
* :doc:`Vtable Generation <VtableGeneration>`

