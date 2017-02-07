Proposal: Self Const
====================

.. Note::
	Feature awaiting further design consideration.

This is a proposal to add ``selfconst`` as a means to avoid having to use ``const`` pass-through.

Rationale
---------

Currently ``const`` pass-through is required for some methods:

.. code-block:: c++

	class ArrayType {
		template <bool IsConst>
		const<IsConst>(int)& index(size_t size) const(IsConst) noexcept;
	}

This is verbose and error-prone. Furthermore it communicates that the ``index()`` method may or may not modify ``self``, which is incorrect; ``index()`` will never modify ``self``.

Also, interface methods cannot be templated, which means either the ``const`` or non-``const`` variant must be chosen:

.. code-block:: c++

	interface ArrayAPI {
		// Can either have:
		int& index(size_t size) noexcept;
		
		// ..or:
		const int& index(size_t size) const noexcept;
	}

selfconst
---------

``selfconst`` would be a new identifier that indicates the ``const``-ness of the object.

.. code-block:: c++

	class ArrayType {
		selfconst(int)& index(size_t size) const noexcept;
	}

It could be used as either ``selfconst(Type)`` in a type expression or ``selfconst`` in a predicate.

Member variables
~~~~~~~~~~~~~~~~

In a ``const`` method all references to member variables have type ``selfconst(T)&``. This reflects that ``const`` methods can be called both for ``const`` and non-``const`` objects.

``selfconst(T)`` isn't equivalent to ``const(T)`` but it also isn't equivalent to ``T``. ``selfconst(T)&`` is implicitly castable to ``const(T)&`` (but clearly **not** ``T&``).

Meaning of const method
~~~~~~~~~~~~~~~~~~~~~~~

The addition of ``selfconst`` means that methods are always marked ``const`` when they don't modify the object, regardless of whether they return non-``const`` references into the object.
