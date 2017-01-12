Proposal: Remove Lifetime Methods
=================================

.. Note::
	Feature awaiting further design consideration.

This is a proposal to remove lifetime methods and eliminate the concept of an object having a 'dead' state.

Rationale
---------

Lifetime methods add significant complexity to the language, and the dead state is particularly something that has to be defined carefully.

Due to the inner/outer separation, method calls to ``__move`` and ``__destroy`` don't have any effect in the dead state.

Changes
-------

This proposal aims to remove lifetime methods by ensuring that all accessible objects are always in a live state.

Move
~~~~

Currently it's possible to create a dead state by moving out of a variable:

.. code-block:: c++

	void f(Object value) {
		g(move value);
		// 'value' now in 'dead' state.
	}

This usage of ``move`` is reasonable and useful; this proposal therefore suggests that ``value`` cannot be accessed after a ``move`` operation:

.. code-block:: c++

	void f(Object value) {
		g(move value);
		
		// ERROR: cannot access variable 'value' after move.
		value.method();
	}

``move`` can also be used on arbitrary reference-typed values:

.. code-block:: c++

	void method() {
		f(move @memberVariable);
	}

This proposal suggests to make such usage illegal; ``move`` can only be used on local variables (instead mechanisms a ``swap()`` function could be used).

Ultimately it would be ideal to be able to use ``move`` on all references, followed by some kind of re-assignment. For example, the following is very likely safe:

.. code-block:: c++

	void method() {
		f(move @memberVariable);
		new(&@memberVariable) Object();
	}

So future work would allow the compiler to prove this is safe.

Assignment
~~~~~~~~~~

The status of assignment is currently unclear; this proposal clarifies it by defining it as a normal method of a class:

.. code-block:: c++

	class Object {
		void assign(Object value) noexcept;
	}

Importantly this means variables cannot be assigned after they have been moved. A new operator might be used for this case:

.. code-block:: c++

	void f(Object value) {
		g(move value);
		value := Object();
	}

An alternative is to keep using placement ``new``:

.. code-block:: c++

	void f(Object value) {
		g(move value);
		new(&value) Object();
	}

Destructor
~~~~~~~~~~

Destructors would no longer implicitly check for the dead state, since they would be guaranteed to only be run once for each object.

islive/setdead/isvalid/setinvalid
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These are no longer needed and can be removed.

Containers
~~~~~~~~~~

Due to the changes containers must now provide a way to extract elements; their erasing methods would return the element being removed. For example, ``std::varray<T>::pop_back()`` would now return a ``T``.

Ranges
~~~~~~

The existing ranges still work, but there may need to be new range types (which could be called 'generators') that support extracting elements:

.. code-block:: c++

	template <typename T>
	interface input_generator {
		bool empty() const;
		T pop_front();
	}
	
	template <typename T>
	interface bidirectional_generator {
		bool empty() const;
		T pop_front();
		T pop_back();
	}

These interfaces match the containers themselves. As part of these changes, ``output_range`` could become ``output_generator``:

.. code-block:: c++

	template <typename T>
	interface output_generator {
		void push_back(T value);
	}
