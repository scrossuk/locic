Proposal: Variants
==================

.. Note::
	Feature awaiting further design consideration.

This is a proposal for replacing 'union datatypes' with 'variants':

.. code-block:: c++

	class Dog { }
	datatype Cat();
	struct Hamster { };
	
	variant Animal = Dog | Cat | Hamster;

A variant is a generalisation of union datatypes to work with any object type, so it allows mixing classes, datatypes, etc.

Syntax
------

Inline definitions
~~~~~~~~~~~~~~~~~~

Unlike union datatypes, variants wouldn't define the object types inline, since this would be very inconvenient:

.. code-block:: c++

	// Not proposed! Very inconvenient!
	variant Animal = class Dog { ... } | datatype Cat() | struct Hamster { };

Furthermore, inline definitions restrict the object type to only being used in a single variant.

Changes to datatypes
~~~~~~~~~~~~~~~~~~~~

``datatype`` might be able to have declarations/definitions similar to ``class``:

.. code-block:: c++

	datatype Cat() {
		void meow();
	}

This isn't a necessary part of this change, but it is easier to do this when inline definitions are removed.

Semantics
---------

Method Calling
~~~~~~~~~~~~~~

If there are common methods across all of the variant sub-types then these can be called via the variant:

.. code-block:: c++

	variant IntOrFloat = int | float;
	
	IntOrFloat f(IntOrFloat value) {
		return value.copy();
	}

Conditional expansion
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

	variant IntOrFloat = int | float;
	
	void f(IntOrFloat value) {
		if (int intVal = value) {
			// ...
		}
	}

This should also work for references to the variant:

.. code-block:: c++

	variant IntOrFloat = int | float;
	
	void f(IntOrFloat& value) {
		if (int& intVal = value) {
			// ...
		}
	}

Switch expansion
~~~~~~~~~~~~~~~~

(This already exists for union datatypes.)

.. code-block:: c++

	variant IntOrFloat = int | float;
	
	void f(IntOrFloat value) {
		switch (value) {
			case(int intVal) {
				// ...
			}
			case(float floatVal) {
				// ...
			}
		}
	}

Templates
~~~~~~~~~

Variants should allow a templated type to be part of a variant:

.. code-block:: c++

	template <typename T>
	variant optional = T | None;
	
	void f(optional<int> value) {
		if (int intVal = value) {
			
		}
	}
