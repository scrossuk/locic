Implicit Operations
===================

Loci supports implicit copying and implicit cast operations. The primary purpose of these features is to reduce code verbosity and speed up development.

Implicit Copying
----------------

(Also see :doc:`Move vs Copy <MoveVersusCopy>`, where this topic is also discussed.)

Consider this code:

.. code-block:: c++

	class ExampleClass(int value) {
		static create = default;
	}
	
	ExampleClass copyFunction(const ExampleClass& object) {
		return object;
	}

Since *passThroughFunction* doesn't use the *move* operator (which would require receiving the object by value or an :doc:`lvalue <LvaluesAndRvalues>`, rather than passing by reference), the presumed intended purpose is to copy the object. The problem is that while *ExampleClass* looks like a perfectly copyable object, for very good reasons the language won't allow copying to happen.

The developer could add a copy method:

.. code-block:: c++

	class ExampleClass(int value) {
		static create = default;
		
		ExampleClass copy() const {
			return @(value);
		}
	}
	
	ExampleClass copyFunction(const ExampleClass& object) {
		return object.copy();
	}

This code will now compile and run correctly. The *copy* method is an excellent solution when copying is likely to be an expensive procedure, such as copying a heap-based data structure. However in this case it's likely to be too heavyweight (imagine having to call *copy()* for every *int* object); we really just want the original code to build and run correctly.

So here's the solution:

.. code-block:: c++

	class ExampleClass(int value) {
		static create = default;
		
		ExampleClass implicitCopy() const {
			return @(value);
		}
	}
	
	ExampleClass copyFunction(const ExampleClass& object) {
		return object;
	}

*implicitCopy* is an example of a 'magic method', similar to operator overloading methods, in that it enables special behaviour within the language for using/manipulating the object. As is mentioned on :doc:`Move vs Copy <MoveVersusCopy>`, you can use :doc:`Algebraic Datatypes <AlgebraicDatatypes>` which have automatically generated *implicitCopy* methods.

Implicit Casts
--------------

**Note that the rules for implicit casting are still under consideration.**

Loci has a simple infrastructure for implicit casts between objects, using a method named *implicitCast*. This integrates into the larger process of general implicit casts, which include operations such as:

* Automatically dissolving :doc:`lvalues <LvaluesAndRvalues>`.
* Copying object references to produce object values.
* Copying const objects to produce non-const objects.
* De-referencing multi-layered reference values.
* Binding values to references (where the binded values exist until the current scope ends).

Since Loci doesn't support :doc:`Method Overloading <FunctionOverloading>`, the *implicitCast* method cannot be duplicated for each potential destination type. Instead, it should be :doc:`templated <Templates>` with any potential destination type, which must then have a static method to accept the source type.

Here's an example straight for the language primitives:

.. code-block:: c++

	template <typename T>
	interface __implicit_cast_from_int_t {
		static T implicit_cast_int_t(int_t value) noexcept;
	}
	
	template <typename T>
	interface __cast_from_int_t {
		static T cast_int_t(int_t value) noexcept;
	}
	
	__primitive int_t {
		static int_t implicit_cast_byte_t(byte_t value) noexcept;
		static int_t implicit_cast_short_t(short_t value) noexcept;
		
		static int_t cast_long_t(long_t value) noexcept;
		
		template <typename T: __implicit_cast_from_int_t<T>>
		T implicitCast() const noexcept;
		
		template <typename T: __cast_from_int_t<T>>
		T cast() const noexcept;
		
		// etc.
	}

Here, *int_t* is the actual type name of *int* (which is actually just a keyword that is translated by the compiler). You can see that *int_t* has both an *implicitCast* and a *cast* method above, where the former is a 'magic method' that allows implicit casts and the latter requires an explicit call by the developer.

The design of primitive casts is based around the principle that extension casts are **OK** (no information is lost), so they can be implicit. However, truncation casts are **DANGEROUS**, so must be explicit. You can see this in the specification of *int*, which can be implicitly casted from the smaller types *byte_t* and *short_t*, but requires an explicit cast from the larger type *long_t*.

