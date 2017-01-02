Proposal: Remove Non-Const Type Tags
====================================

.. Note::
	Feature awaiting further design consideration.

This is a proposal to simplify the language type system by removing all non-``const`` type tags.

Removing Type Tags
------------------

Lval
~~~~

``lval`` can be removed by moving the lval functionality directly into the object, by:

* Removing existing lval types ``ptr_lval_t`` and ``value_lval_t``.
* Adding an ``address()`` method to ``ref_t``.
* Modifying the :doc:`lifetime methods <../LifetimeMethods>`.

ref_t::address
++++++++++++++

``address`` would be a new method of ``ref_t``:

.. code-block:: c++

	template <bool IsConst>
	const<IsConst>(T)* address() const(IsConst) noexcept;

Hence it would become valid to perform ``&r`` where ``r`` has type ``T&``. This is desirable because it is more familiar for C++ developers.

__assign
++++++++

``__assign`` would be a new lifetime method:

.. code-block:: c++

	void __assign(T value) noexcept;

For example, there is existing code such as:

.. code-block:: c++

	template <bool IsConst>
	lval ptr_lval_t<const<IsConst>(T)> index(size_t index) const(IsConst) noexcept;

This would be replaced with:

.. code-block:: c++

	template <bool IsConst>
	const<IsConst>(T)& index(size_t index) const(IsConst) noexcept;

Hence when code performs ``array[index] = value;`` this will become ``array[index].__assign(value);``.

__move
++++++

``__moveto`` would be replaced by ``__move``:

.. code-block:: c++

	T __move() noexcept;

Hence the expression ``move value`` would become ``value.__move()``.

The purpose of this change is to simplify expansion of ``move value`` as well as adding type safety to the move method.

ref
~~~

``ref`` can be removed by simply providing special treatment to ``ref_t``, which makes sense as it is a primitive type. The transformation of ``T&`` would now be ``ref_t<T>`` rather than ``ref<T> ref_t<T>``, and ``ref_t`` would always have its indirect method call support (i.e. there wouldn't be a non-``ref`` version of ``ref_t``).

Containers
++++++++++

A useful property of ``T&`` is that it can currently be stored in containers:

.. code-block:: c++

	std::varray<T&> getReferenceArray();

This behaviour is preserved by removing ``ref`` because the indirect method call support of ``ref_t`` doesn't actually affect its API. So ``T&`` is a copyable, assignable and movable type, but when the compiler can see that the type is ``T&`` it will perform method calls to ``T``.

References-to-references
++++++++++++++++++++++++

A consequence of having references in containers is that it would be valid to have references to references:

.. code-block:: c++

	template <typename T>
	void f(T&& arg);
	
	template <typename T>
	void g(T& arg);
	
	template <typename T>
	void h(T arg);
	
	template <typename T>
	void example() {
		std::varray<T&> array = getReferenceArray();
		f<T>(array[0]);
		g<T&>(array[1]);
		h<T&&>(array[2]);
	}

The reference-to-reference type is created by the substitution of ``T&`` into ``std::varray::index``:

.. code-block:: c++

	template <bool IsConst>
	const<IsConst>(T)& index(size_t index) const(IsConst) noexcept;

After substitution:

.. code-block:: c++

	T&& index(size_t index) noexcept;

This is correct because we're getting a reference to the reference stored inside the array. Clearly, within the templated code ``T`` looks like a normal value, but outside of the template it is actually seen as ``T&``.

.. Note::
	C++ doesn't allow reference-to-reference types, which means it isn't possible to store references in a container.

Manipulating a reference
++++++++++++++++++++++++

There are cases where it would be desired to call methods of ``T&`` rather than ``T``:

.. code-block:: c++

	class RefClass(T& reference) {
		RefClass __move() noexcept {
			// This will attempt to call ``__move()`` on ``T``, but we just want
			// to move the reference.
			return @(@reference.__move());
		}
	}

In this case it is proposed to add new syntax ``.&`` for calling methods of the reference:

.. code-block:: c++

	class RefClass(T& reference) {
		RefClass __move() noexcept {
			// This will now call ``__move()`` on ``T&``.
			return @(@reference.&__move());
		}
	}

Furthermore ``.&&`` could be used for calling methods of ``T&&``, ``.&&&`` for ``T&&&``, etc.

Variable reference
++++++++++++++++++

A variable reference expression ``v`` would always be ``decltype(v)&``:

* ``T v`` means expression ``v`` has type ``T&``.
* ``T& v`` means expression ``v`` has type ``T&&``.
* ``T&& v`` means expression ``v`` has type ``T&&&``.

Address-of
++++++++++

The new ``address()`` method of ``ref_t`` would effectively always be called via ``r.&address()``:

* ``T v`` means expression ``&v`` will call ``address()`` on ``T&``.
* ``T& v`` means expression ``&v`` will call ``address()`` on ``T&``.
* ``T&& v`` means expression ``&v`` will call ``address()`` on ``T&``.

Users can use ``r.address()``, ``r.&&address()``, etc. if they intended a different meaning, however in almost all cases the desire will be to turn the reference into a pointer (as is common in C++).

.. Note::
	All other operations will call methods of ``T``; ``move value`` -> ``value.__move()``, ``value = ...`` -> ``value.__assign(...)``, etc.

staticref
~~~~~~~~~

``staticref`` is used to give the compiler information about a ``typename``:

.. code-block:: c++

	interface HasStaticMethod {
		static int static_method();
	}
	
	template <typename T: HasStaticMethod>
	int f() {
		staticref<T> typename v = T;
		return v.static_method();
	}

This code shows how ``T.static_method()`` is actually expanded, and indeed ``staticref`` primarily exists to allow ``T.static_method()`` to work.

typename_t
++++++++++

``staticref`` can be removed by adding a template argument to ``typename_t``. The template argument clearly cannot itself be ``typename``, hence a new ``abstract typename`` would be added:

.. code-block:: c++

	template<abstract typename API>
	__primitive typename_t {
		// ...
	}

As with ``ref_t``, the compiler would now recognise ``typename_t`` as a special case for calling static methods, so the above code would become:

.. code-block:: c++

	interface HasStaticMethod {
		static int static_method();
	}
	
	template <typename T: HasStaticMethod>
	int f() {
		typename_t<T> v = T;
		return v.static_method();
	}

``typename`` would now resolve to ``typename_t<none_t>``, where ``none_t`` is an empty interface; the compiler can use the ``require()`` predicates to determine the real interface when ``typename`` appears in a template argument.

abstract_typename_t
+++++++++++++++++++

``abstract typename`` would resolve to a new primitive type ``abstract_typename_t``, taking no template arguments:

.. code-block:: c++

	__primitive abstract_typename_t {
		// ...
	}

Passing interfaces to typename
++++++++++++++++++++++++++++++

As part of this change, it would become illegal to pass interface types to ``typename``:

.. code-block:: c++

	void f() {
		g<InterfaceType>();
	}
	
	template <typename T>
	void g();

Hence ``typename`` means a concrete object type must be specified, such as a ``class``. This fits with the fact that static methods can only be called on ``typename`` and **not** ``abstract typename``. For example, this clearly wouldn't be valid:

.. code-block:: c++

	void f() {
		InterfaceType.static_method();
	}

Cast from typename_t to abstract_typename_t
+++++++++++++++++++++++++++++++++++++++++++

There would now be an implicit compile-time cast from ``typename_t`` to ``abstract_typename_t``:

.. code-block:: c++

	template <typename T>
	void f() {
		g<T>();
	}
	
	template <abstract typename T>
	void g();

This is valid because a concrete type can be treated as if it is an abstract type. The opposite way around is **not** valid:

.. code-block:: c++

	template <abstract typename T>
	void f() {
		g<T>(); // ERROR: cannot convert 'abstract typename' to 'typename'
	}
	
	template <typename T>
	void g();

Calling methods of typename_t
+++++++++++++++++++++++++++++

Unlike ``ref_t`` there is no problem with calling methods of ``typename_t``:

.. code-block:: c++

	class TypeClass(typename type) {
		TypeClass __move() noexcept {
			return @(@type.__move());
		}
	}

Static methods can also be called without issue:

.. code-block:: c++

	size_t f() {
		return typename.__sizeof();
	}

.. Note::
	The expression ``typename`` has type ``typename_t<typename_t<none_t>>``, because the expression satisfies the API defined by ``typename_t<none_t>``, and hence can be called by any of the static methods of ``typename_t<none_t>``.

Removing Const
--------------

It is theoretically possible to remove ``const`` as well:

::

	const<P>(T) -> const_t<T, P>

.. code-block:: c++

	template <abstract typename T, bool P>
	__primitive const_t { }

``const_t`` would have indirect method calls for both static and non-static methods, hence being similar to a combination of ``typename_t`` and ``ref_t``.

However this change doesn't simplify the type system considerably and would require significant changes to the compiler (which are likely to make the compiler slower).
