Proposal: Object Properties
===========================

.. Note::
	Feature awaiting further design consideration.

This is a proposal to add syntax for representing object properties:

.. code-block:: c++

	class Example(int value) {
		int value;
	}

	int test(Example object) {
		return object.value;
	}

This is effectively a generalisation of methods; methods currently take a tuple of 0 or more values, whereas properties don't take a tuple at all.

Language
--------

Const-ness of properties
~~~~~~~~~~~~~~~~~~~~~~~~

Properties are always implicitly ``const``, since developers wouldn't expect syntax such as ``object.method`` (i.e. without parentheses) as performing a mutation on the object.

Hence note that this proposal depends on adding ``selfconst``, since this allows us to return non-``const`` references from properties that are ``const``:

.. code-block:: c++

	class Test {
		selfconst(int)& member;
	}

Without ``selfconst`` (and using ``const`` pass-through) the compiler would think that the method sometimes modifies ``self``, and hence is not valid as a property.

Noexcept-ness of properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Similarly to ``const``, properties are implicitly ``noexcept``, since we're not expecting them to perform any operations that would require throwing (e.g. allocating memory).

Datatype Properties
~~~~~~~~~~~~~~~~~~~

The members of a ``datatype`` are equivalent to explicitly defined properties in a ``class``:

.. code-block:: c++

	datatype Test(int member);

	// ...effectively becomes:
	class Test(int value) {
		...

		selfconst(int)& member;

		...
	}

Method and property clashes
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Consider:

.. code-block:: c++

	class Example {
		Example copy() const;
		int copy;
	}

This isn't allowed because it makes ``object.copy`` ambiguous; the property must be renamed.

Note that this also disallows datatypes such as:

.. code-block:: c++

	// ERROR: property 'copy' clashes with auto-generated method 'copy'
	datatype Test(int copy);

Standard library
----------------

``std::varray<>`` should use properties for ``size``, ``capacity`` and ``empty``:

.. code-block:: c++

	size_t concatSize(int[] a, int[] b) {
		return a.size + b.size;
	}

Similarly, ranges should use properties for ``front``, ``back``, etc.:

.. code-block:: c++

	template <typename T>
	interface bidirectional_range {
		bool empty;
		
		void skip_front();
		
		T front;
		
		void skip_back();
		
		T back;
	}
	
	int addFrontAndBack(const forward_range<int>& elements) {
		assert !elements.empty;
		return element.front + elements.back;
	}
