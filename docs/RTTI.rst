RTTI
====

(Not currently implemented! Likely to appear in version 1.3+.)

Loci provides far more comprehensive run-time type information than C++, giving extra functionality as well as efficiency to developers.

is_a
----

This supports checking whether the actual type (which is known only at run-time, unless A is a class type) of a reference or pointer conforms to a given target type (B in this example).

.. code-block:: c++

	void f(A& object) {
		if (is_a<B&>(object)) {
			//...
		}
	}

The performance for each case (depending on types of A and B) is:

* If A is a class type, and B is a class type, then the 'is_a' statement is resolved at compile-time (hence no run-time overhead).
* If A is a class type, and B is an interface type, then the compiler can again resolve it at compile-time.
* If A is an interface type, but B is a class type, the implementation for this is just a single comparison, so this is a very efficient way to check the type of an interface reference/pointer.
* If A is an interface type, and B is an interface type, then this call involves a number of comparisons proportional to the number of methods in the larger interface, unless it can be determined at compile-time that there is a valid static cast from A to B.

dynamic_cast
-----------

This operates just like C++:

.. code-block:: c++

	int f(A& object) {
		B& castObject = dynamic_cast<B&>(object);
		return castObject.method();
	}

This is effectively just the equivalent of calling is_a to *assert* the types are compatible, and then either treating the reference to A as a reference to B.

typeid
------

This allows access to information about a type from a reference or pointer to an instance of it:

.. code-block:: c++

	void f(T& a, T& b) {
		rtti::type_info typeInfo = typeid(a);
		
		// type_info structures can be compared,
		// but is_a is likely to be more efficient.
		if (typeInfo == typeid(b)) {
			//...
		}
		
		// Get the type's name.
		string typeName = typeInfo.name;
		
		// Get the type's virtual table.
		void * vtable = typeInfo.vtable;
		
		// Get the type's size.
		size_t typeSize = typeInfo.size;
		
		for (final auto i: std::counter<size_t>(0, typeInfo.methods.size())) {
			rtti::method_info methodInfo = typeInfo.methods[i];
			
			// Get method name.
			string methodName = methodInfo.name;
			
			// etc.
		}
	}

