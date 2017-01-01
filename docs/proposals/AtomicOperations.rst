Proposal: Atomic Operations
===========================

.. Note::
	Feature awaiting further design consideration.

Atomic operations provide well-defined behaviour when multiple threads interact.

Usage
-----

.. code-block:: c++

	void f(atomic int& value) {
		value++;
	}

This code will generate code that atomically increments the ``int`` referred to by ``value``.

Implementation
--------------

Atomic keyword
~~~~~~~~~~~~~~

The ``atomic`` keyword makes a type atomic by wrapping it with ``atomic_t``:

::

	atomic int -> atomic_t<int>

``atomic`` has no effect other than this and does **not** introduce a new type tag (i.e. unlike ``const``, ``lval`` and ``ref``).

Atomic type
~~~~~~~~~~~

``atomic_t`` is a type that supports methods such as ``increment``, ``decrement``, ``addassign``, etc. and calls to the wrapped type's ``atomic_increment``, ``atomic_decrement``, ``atomic_addassign``, etc.

The code above is therefore approximately equivalent to:

.. code-block:: c++

	void f(int& value) {
		value.atomic_increment();
	}

Loading values
~~~~~~~~~~~~~~

Values can be loaded atomically simply by an implicit cast, which calls the ``atomic_load`` method of the wrapped type:

.. code-block:: c++

	int f(atomic int& value) {
		return value;
	}

Storing values
~~~~~~~~~~~~~~

.. code-block:: c++

	void f(atomic int* value) {
		*value = 42;
	}
