Proposal: Late Move Parameters
==============================

.. Note::
	Feature awaiting further design consideration.

This is a proposal for a new ``latemove`` attribute for function parameters that indicates the argument should be moved **after** the function call, unlike a normal parameter which would be moved before the function call.

Rationale
---------

Currently move operations are invoked regularly in cases where they could be avoided. For example:

.. code-block:: c++

	template <movable T>
	unique_ptr<T> new_unique(T value);
	
	void f() {
		auto p = new_unique<Class>(Class());
		// ...
	}

There will be at least one move operation to move the value ``Class()`` onto the heap memory allocated by ``new_unique()``. However this is potentially avoidable by having ``Class()`` directly construct its result on the heap memory allocated by ``new_unique()``.

Usage
-----

The ``latemove`` keyword indicates that a parameter should be moved by the caller **after** the function call.

.. code-block:: c++

	template <typename T>
	unique_ptr<T> new_unique(latemove T value);
	
	void f() {
		auto p = new_unique<Class>(Class());
		// ...
	}

The effect is that ``new_unique()`` can only pass its parameter ``value`` to other function's arguments marked ``latemove``. Notably, it is unable to call any of the methods of ``T`` on ``value``.

If it assigns ``value`` to some storage this is only observed once the function call has returned:

.. code-block:: c++

	template <typename T>
	void assign(T* ptr, latemove T value) {
		*ptr = move value;
		// *ptr will have been destroyed, but will not yet contain the contains of 'value'.
	}

.. Note::
	Functions taking ``latemove`` arguments don't require that the variable type is movable, since these functions don't actually perform a move; the caller may or may not perform a move and hence the requirement is only ever needed in the caller.

Implementation
--------------

``latemove`` can only be specified on function argument variables, **not** types, however it affects the type of the argument:

.. code-block:: c++

	latemove T -> latemove_t<T>

latemove_t
~~~~~~~~~~

``latemove_t`` is a simple primitive type that holds a ``T**``. It is roughly equivalent to the following:

.. code-block:: c++

	template <typename T>
	class latemove_t(T** move_destination) {
		static create = default;
		
		void set_move_destination(T* move_destination) {
			*@move_destination = move_destination;
		}
	}

The purpose of ``latemove_t`` is therefore to capture the move destination once it has been determined.

Transformation
~~~~~~~~~~~~~~

Code that called the ``assign()`` function (shown previously) might look something like:

.. code-block:: c++

	void f(Class* ptr) {
		assign(ptr, Class());
	}

This code is then transformed into:

.. code-block:: c++

	void f(Class* ptr) {
		Class* _move_dest;
		assign<Class>(ptr, latemove_t<Class>(&_move_dest));
		new(_move_dest) Class();
	}

The compiler puts a ``T*`` on the stack and then passes in the address of it contained in a ``latemove_t``.

In this case ``Class()`` is being constructed directly in the memory referred to be ``ptr``. The ``assign()`` function only calls the destructor on ``*ptr``, but the move is delayed until ``assign()`` has returned.

.. Note::
	The ``T*`` on the stack is only generated for the initial ``T`` -> ``latemove T`` implicit cast. If a function argument is marked ``latemove T`` and given a ``latemove_t<T>`` then no special transformation is performed.

Throwing expressions
~~~~~~~~~~~~~~~~~~~~

The ``latemove`` transformation moves two function calls across each other. This has observable side effects, but that's understood since the source code also specifies those side effects via the ``latemove`` keyword.

A key issue, however, is that an expression constructing an object may fail and throw an exception. For example:

.. code-block:: c++

	class Class() {
		static create() {
			throw Exception();
		}
	}
	
	void f(Class* ptr) {
		assign(ptr, Class());
	}

Since ``Class()`` may throw, the transformation cannot be performed because mutations performed by ``assign()`` (in this case, calling the destructor on ``*ptr``) cannot be reversed if ``Class()`` fails. In this case we get a move:

.. code-block:: c++

	void f(Class* ptr) {
		T* _tmp = alloca(sizeof(Class));
		new(&_tmp) Class();
		Class* _move_dest;
		assign<Class>(ptr, latemove_t<Class>(_move_dest));
		new(_move_dest) move *_tmp;
	}

This code is effectively equivalent to the code that would've been generated without ``latemove``. The problem is that the constructor of ``Class`` is **not** declared as ``noexcept``.

.. Note::
	The other way around (if ``assign()`` could throw but ``Class()`` was ``noexcept``) would **not** affect the transformation because the throwing function (``assign()``) is run before the non-throwing function. If ``assign()`` failed then ``Class()`` wouldn't be run, but that's an understood side effect of ``latemove``.

Chaining latemove
~~~~~~~~~~~~~~~~~

``latemove`` can chain efficiently:

.. code-block:: c++

	Class f(latemove Class value) noexcept {
		return move value;
	}
	
	Class g(latemove Class value) noexcept {
		return move value;
	}
	
	Class h() noexcept {
		return Class();
	}
	
	void assign(Class* ptr) {
		*ptr = f(g(h()));
	}

This is ultimately transformed into something similar to:

.. code-block:: c++

	void f(Class* retptr, latemove_t<Class> value) noexcept {
		value.set_move_destination(retptr);
	}
	
	void g(Class* retptr, latemove_t<Class> value) noexcept {
		value.set_move_destination(retptr);
	}
	
	void h(Class* retptr) noexcept {
		new(retptr) Class();
	}
	
	void assign(Class* ptr) {
		ptr->~();
		
		Class* _g_ret;
		f(ptr, latemove_t<Class>(&g_ret));
		
		Class* h_ret;
		g(g_ret, latemove_t<Class>(&h_ret));
		
		h(h_ret);
	}

The optimised code would be:

.. code-block:: c++

	void assign(Class* ptr) {
		ptr->~();
		new(ptr) Class();
	}

Without ``latemove`` the code would be:

.. code-block:: c++

	void f(Class* retptr, Class* value) noexcept {
		new(retptr) move *value;
	}
	
	void g(Class* retptr, Class* value) noexcept {
		new(retptr) move *value;
	}
	
	void h(Class* retptr) noexcept {
		new(retptr) Class();
	}
	
	void assign(Class* ptr) {
		ptr->~();
		T* _tmp0 = alloca(sizeof(Class));
		h(_tmp0);
		T* _tmp1 = alloca(sizeof(Class));
		g(_tmp1, _tmp0);
		f(ptr, _tmp1);
	}

The optimised code would have two unnecessary moves corresponding to the bodies of ``f()`` and ``g()``.

Interaction with other features
-------------------------------

Internal Constructor
~~~~~~~~~~~~~~~~~~~~

As part of this proposal the internal constructor should use ``latemove`` for its arguments:

.. code-block:: c++

	class Parent(Child arg) {
		static create(latemove Child arg) noexcept {
			return @(move arg);
		}
	}

This means expressions such as ``Parent(Child())`` would have the ``Child`` be directly constructed into its location in the ``Parent`` object.

Named Parameters
----------------

Named parameters could refer to each other if they are both ``latemove``:

.. code-block:: c++

	class Parent(First first, Second second) {
		static create() noexcept {
			return @(first=First(), second=Second(first));
		}
	}

This works because the internal constructor will provide the pointers of both arguments, and ``Second(first)`` is computed after the internal constructor has completed (hence the pointer to ``first`` is available).

Again, however, this relies on ``Second(first)`` being ``noexcept``, otherwise it must be performed prior to the internal constructor.

Reversibility
-------------

The requirement for ``latemove`` expressions to be ``noexcept`` could be avoided by allowing a function or method to specify how it can be reversed. For example:

.. code-block:: c++

	class PushBackReverser(size_t& sizeRef) {
		static create = default;
		
		void call() {
			@sizeRef--;
		}
	}
	
	template <typename T>
	class Array(T* data, size_t size) {
		void push_back(latemove T value) reverse(PushBackReverser) {
			reserve(size() + 1);
			new(&data[size()]) move value;
			@size++;
			return void, reverse PushBackReverser(@size);
		}
	}

The ``PushBackReverser`` is an additional return value from the ``push_back()`` method that allows the effects of the method to be reversed.

A similar mechanism for ``new_unique`` might be:

.. code-block:: c++
	
	class NewReverser(void* ptr) {
		static create = default;
		
		void call() {
			heap_free(@ptr);
		}
	}
	
	template <typename T>
	unique_ptr<T> new_unique(latemove T value) reverse(NewReverser) {
		T* ptr = heap_alloc(sizeof(T));
		new(ptr) move value;
		return unique_ptr<T>(ptr), reverse NewReverser(ptr);
	}

This requires significant additions in terms of syntax, semantics and code generation, and would require the caller to handle the reverser object even if it didn't intend to use it. Compared to the generally low cost of a move operation, full reversibility seems to **not** be worthwhile.

However, there are at least two special cases where no reverse operation is required:

* ``new(ptr) expr``
* ``@(expr, ...)``

In these cases ``expr`` can be a throwing expression because these cases don't perform any mutations beyond the move operation.
