References
==========

References are aliases for an object in memory. They can be used to access the methods of the object indirectly. Unlike pointers, they cannot be ``null``.

.. code-block:: c++
	
	void example() {
		int value = 10;
		int& reference = value;
		reference = 20;
		assert value == 20;
	}

.. Note::
	Loci's references are almost identical to C++; the notable exception being that references can be stored in a container.

Indirect Method Call
--------------------

References allow method calls to the referenced object via the normal 'dot' syntax:

.. code-block:: c++
	
	void callMethod(Object& value) {
		value.method();
	}

Assignment
----------

References provide support for assignment:

.. code-block:: c++
	
	void f(int& value) {
		value = 100;
	}

As a template argument
----------------------

References can be passed as template arguments, notably allowing storing them in containers.

.. code-block:: c++

	template <typename T>
	T copyValue(const T& value) require(copyable<T>) {
		return value.copy();
	}

	int& copyReference(int& value) {
		return copyValue<int&>(value);
	}

This works because the 'special effects' of method indirection only occur when the compiler can see that the type is a reference. Hence templated code sees the reference type as if it is a normal value type.

A consequence of this property is that a trivial template code substition would not work correctly:

.. code-block:: c++

	int& copyReference(int& value) {
		// This will try to copy the int, but the original code copied the reference.
		return value.copy();
	}

References-to-references
------------------------

It is possible to create a reference that refers to another reference, up to any number of levels, with additional ampersands in the type. Any indirect method calls to ``T&``, ``T&&``, etc. will always call methods of ``T``:

.. code-block:: c++

	void a(Object arg) {
		arg.method(); // Calls Object::method().
	}
	
	void b(Object& arg) {
		arg.method(); // Calls Object::method().
	}
	
	void c(Object&& arg) {
		arg.method(); // Calls Object::method().
	}
	
	void d(Object&&& arg) {
		arg.method(); // Calls Object::method().
	}

Manipulating a reference
------------------------

.. Note::
	Not currently implemented.

There are cases where it would be desired to call methods of ``T&`` rather than ``T``:

.. code-block:: c++

	class RefClass(T& reference) {
		RefClass __move() noexcept {
			// This will attempt to call ``__move()`` on ``T``, but we just want
			// to move the reference.
			return @(@reference.__move());
		}
	}

The syntax ``.&`` can be used to call methods of the reference:

.. code-block:: c++

	class RefClass(T& reference) {
		RefClass __move() noexcept {
			// This will now call ``__move()`` on ``T&``.
			return @(@reference.&__move());
		}
	}

Furthermore ``.&&`` can be used for calling methods of ``T&&``, ``.&&&`` for ``T&&&``, etc.

Conversion to pointers
----------------------

References can be converted into pointers using the address-of (``&``) operator. For ``T&``, ``T&&``, ``T&&&``, etc. the address-of operator will always get the address of the ``T`` object. It is therefore effectively called via ``r.&address()``:

* ``T v`` means expression ``&v`` will call ``address()`` on ``T&``.
* ``T& v`` means expression ``&v`` will call ``address()`` on ``T&``.
* ``T&& v`` means expression ``&v`` will call ``address()`` on ``T&``.

Users can use ``r.address()``, ``r.&&address()``, etc. if they intended a different meaning, however in almost all cases the desire will be to turn the reference into a pointer (as is common in C++).
