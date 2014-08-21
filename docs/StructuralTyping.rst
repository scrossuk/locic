Structural Typing
=================

From Wikipedia:

	A structural type system (or property-based type system) is a major class of type system, in which type compatibility and equivalence are determined by the type's actual structure or definition, and not by other characteristics such as its name or place of declaration. Structural systems are used to determine if types are equivalent and whether a type is a subtype of another. It contrasts with nominative systems, where comparisons are based on the names of the types or explicit declarations, and duck typing, in which only the part of the structure accessed at runtime is checked for compatibility.

Interfaces
----------

Java provides the concept of interfaces as equivalent to a pure virtual class in C++. This means that interfaces declare the methods that an implementing class must support, without providing any implementation of those methods. As compared to inheritance of classes, this is an ideal way to model the API of a class.

Loci uses, and extends, this concept by using interfaces as the basis for structural typing. This means that classes don't need to explicitly specify that they implement an interface, but if they have all the methods specified in the interface with the correct types and names then references to the class can be implicitly cast to references to the interface.

For example:

.. code-block:: c++

	class Bucket {
		static Create(int v);
		
		bool containsValue() const;
		
		void placeValue(int value);
		
		int getValue() const;
	}
	
	interface ValueHolder {
		bool containsValue() const;
		
		int getValue() const;
	}
	
	int f(const ValueHolder& valueHolder) {
		return valueHolder.getValue();
	}
	
	void function() {
		Bucket bucket = Bucket(2);
		int value = f(bucket);
	}

This clearly diverges from the approach taken by many languages, which is to requires classes to specify the interfaces they implement. However, the increased flexibility of this method allows interfaces to be defined **after** classes have been defined, which would allow code like the following:

.. code-block:: c++

	interface LengthValue {
		size_t length() const;
	}
	
	size_t f(const LengthValue& lv) {
		return lv.length();
	}
	
	void function() {
		std::string string = "Hello world!";
		size_t length = f(string);
		// etc...
	}


'std::string' is a standard type, so is designed and implemented prior to the definition of the custom interface 'LengthValue', and therefore it would be unlikely that standard library developers would foresee the need to implement that (or a similar) interface. However, structural typing allows the cast from 'std::string&' to 'LengthValue&' to be made because the types are in fact compatible.

Primitive Objects
-----------------

As mentioned in :doc:`Primitive Objects <PrimitiveObjects>`, this functionality works equivalently well with primitive types:

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

Implementation
--------------

See:

* :doc:`Dynamic Dispatch <DynamicDispatch>`
* :doc:`Vtable Generation <VtableGeneration>`

