References
==========

Loci references are in many ways similar to C++ references. The key differences between C++ and Loci references from the point of view of developers are that:

* References cannot be used to assign to the referred-to value; lvals should be used for this.
* References can be used inside containers, since they are assignable.

Ref Type Attribute
------------------

References are implemented in Loci based on the *ref* type attribute (*lval* is a similar type attribute; see :doc:`Lvalues and Rvalues <LvaluesAndRvalues>`), which specifies that the value should be treated with special rules. For example, consider the following code:

.. code-block:: c++

	IntRefType getNonRef();
	
	ref<int> IntRefType getRef();
	
	void function() {
		// Invalid: "Type 'IntRefType' doesn't have a method named 'abs'."
		int a = getNonRef().abs();
		
		// Valid; implicitly dereferences 'IntRefType' to call method 'abs'.
		int b = getRef().abs();
	}

Here you can see that implicit conversion operations between types are able to use the information provided by the *ref* attribute to generate implicit dereference operations. Note that the first point above isn't entirely true; *ref* and *lval* can be combined to achieve this:

.. code-block:: c++

	ref<lval<int> IntLvalType> IntLvalRefType getLvalRef();
	
	void function() {
		// Implicitly dereferences 'IntLvalRefType' to call method 'assign'.
		getLvalRef() = 44;
	}

Of course, the *ref* syntax is quite verbose, and 'normal' references (i.e. as opposed to user-defined reference types) can be used with this C++-like syntax:

.. code-block:: c++

	int f(int& value) {
		return value.abs();
	}
	
	int f(ref<int> ref_t<int> value) {
		return value.abs();
	}

Both functions are equivalent; the compiler automatically turns the ampersand into the *ref* attribute applied to the primitive type 'ref_t'. This is very similar to the compiler's automatic translation of C pointer syntax into 'ptr_t' with a template argument.

Here are some more examples of references:

.. code-block:: c++

	void function(int data, int * pointer){
		{
			// Invalid - no implicit cast from
			// pointer type to reference type.
			int& reference = pointer;
		}
		
		{
			// Valid - using dereference operator
			// to cast from pointer to reference.
			int& reference = *pointer;
		}
		
		{
			// Invalid - again, no implicit cast
			// from pointer type to reference type.
			int& reference = &data;
		}
		
		{
			// Valid - making reference of 'data'.
			int& reference = data;
			
			// Invalid - cannot assign since referred-to
			// type is not an lval.
			reference = 1;
			
			// Invalid - type 'ref_t' has no overload for
			// the dereference operator.
			*reference = 2;
			
			// Valid - this will dereference the value and
			// then call '.add(5)'.
			int newData = reference + 5;
		}
		
		{
			// Valid.
			// Binds value to non-const reference.
			// Note the difference to C++, which only allows binding to const references.
			int& reference = 1;
			
			// ...which is basically equivalent to this.
			int __unnamed_value = 1;
			int& reference = __unamed_value;
		}
		
		{
			// Valid (same as above).
			const int& reference = 5;
			
			// ...which is basically equivalent to this.
			const int __unnamed_value = 5;
			const int& reference = __unamed_value;
		}
	}


