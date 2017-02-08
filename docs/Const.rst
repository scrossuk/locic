Const
=====

The ``const`` keyword indicates that an object should not be modified via the reference marked ``const``. It is very similar to the ``const`` keyword in C and C++, albeit there are some minor differences (detailed below).

Usage
-----

Const Values
++++++++++++

``const`` prevents modifications of values. For example, the following code is invalid:

.. code-block:: c++

	void function() {
		const int i = 0;
		i = 1;
	}

In this case the compiler will complain that the underlying :doc:`lvalue <LvaluesAndRvalues>` has a :ref:`require predicate <require-predicates>` for its ``assign`` method that specifies the ``const int`` type must be ``movable``; ``const`` types are **not** ``movable`` and therefore the assignment cannot occur.

Const Pointers
++++++++++++++

Here's another example of invalid code:

.. code-block:: c++

	void function() {
		int i = 0;
		const int* p = &i;
		*p = 1;
	}

While ``i`` can be modified directly, ``p`` refers to ``const int`` and therefore assignment is again not possible.

Const Predicates
++++++++++++++++

It's possible to specify that a value is ``const`` based on the result of a :ref:`predicate <const-predicates>`:

.. code-block:: c++

	template <bool IsConst>
	const<IsConst>(int)& function(const<IsConst>(int)* ptr) {
		return *ptr;
	}

This function will work with both ``const`` and non-``const`` values while remaining type safe. This is **not** possible in C and can only be achieved in C++ via function overloading.

Methods
+++++++

Methods can be marked as ``const`` to indicate they do not modify their owning object.

.. code-block:: c++

	class ClassObject {
		void normalMethod();
		void constMethod() const;
	}
	
	void f(const ClassObject& object){
		// Invalid - non-const method cannot
		// be called on const object.
		object.normalMethod();
		
		// Valid.
		object.constMethod();
	}

It's possible to specify a :ref:`predicate <const-predicates>` within the ``const`` declaration. For example:

.. code-block:: c++

	interface RunnableType {
		void run();
	}
	
	interface ConstRunnableType {
		void run() const;
	}
	
	template <typename T>
	class RunWrapper {
		void run() require(T : RunnableType) const(T : ConstRunnableType);
	}

The ``run()`` method of ``RunWrapper`` requires that ``T`` has a ``run()`` method, using a :ref:`require() predicate <require-predicates>`. Furthermore, if the ``run()`` method of ``T`` is ``const`` then the ``run()`` method of ``RunWrapper`` will **also** be ``const``.

selfconst
+++++++++

``selfconst`` is a convenient mechanism to indicate that a method signature (typically the return type) depends on whether ``self`` is ``const``:

.. code-block:: c++

	class Array {
		selfconst(int)& index(size_t size) const noexcept;
	}

This tells the compiler that the ``index()`` method won't modify the ``Array`` object, however it can return a non-``const`` reference if ``self`` is non-``const``.

``selfconst`` can also be used as a predicate value; ``selfconst(Type)`` is equivalent to ``const<selfconst>(Type)``.

Properties
----------

.. _cumulative-const:

Cumulative
++++++++++

``const`` is cumulative, meaning that an application of ``const()`` to a type **cannot** reverse the effects of any previous application of ``const()`` to the type.

Each application of ``const()`` is a logical **OR** with the type's existing ``const`` predicate. So ``const<A>(const<B>(Type))`` is equivalent to ``const<A or B>(Type)``.

.. Note::
	An important consequence of cumulative ``const`` is that ``const<false>()`` has no effect, since ``const<false>(const<A>(Type))`` is equivalent to ``const<false or A>(Type)``, which is equivalent to ``const<A>(Type)``.

This property of ``const`` is particularly relevant to :doc:`templates <Templates>`:

.. code-block:: c++

	template <typename T, bool Predicate>
	const<Predicate> T& f(T& value) {
		return value;
	}
	
	void g() {
		const int i = 10;
		int& r = f<const int, false>(i);
		// Did we get a non-const reference to a const object?
	}

For the templated function ``f()``, ``T`` may or may not ultimately be ``const``. If ``const`` was **not** cumulative then the cast from ``T&`` to ``const<Predicate> T&`` in ``f()`` would actually be invalid for the invocation ``f<const int, false>``, because it would be performing a cast from ``const int&`` to ``int&``.

However ``const`` is cumulative so ``f()`` is actually performing a cast from ``const int&`` to ``const int&``, which is valid. The error is therefore in ``g()``, which is attempting to cast the ``const int&`` returned by ``f()`` to ``int&``.

.. Note::
	Cumulative ``const`` is also useful in terms of the :doc:`vtable generation <VtableGeneration>` because it prevents templates accessing the original non-``const`` object, which may have a larger set of methods; the vtable given to the template is only guaranteed to contain methods available in the ``const`` object.

.. _relative-const:

Relative
++++++++

``const`` is relative, since templated code may interact with a ``const`` object as if it were non-``const``.

.. code-block:: c++

	interface CalculatorAPI {
		int calculate();
	}
	
	template <typename T: CalculatorAPI>
	int f(T& value) {
		value.calculate();
	}
	
	class Calculator {
		int calculate() const;
	}
	
	int g(const Calculator& calculator) {
		return f<const Calculator>(calculator);
	}

In this code ``f()`` sees ``T`` as a non-``const`` variable and calls a non-``const`` method on ``T``. However ``g()`` sees ``const Calculator`` with a ``const`` method. Despite this apparent contradiction, the code is valid.

This works because ``f()`` knows that ``T`` must satisfy ``CalculatorAPI`` otherwise the instantiation would fail. The ``calculate()`` method of ``CalculatorAPI`` is not marked ``const``, but this doesn't mean the real method being called can't be ``const`` (the opposite way around would be an error).

Transitive
++++++++++

``const`` is transitive due to restrictions specified by the language's :doc:`primitive types <PrimitiveObjects>` (e.g. ``ptr_t``, which is the underlying type of a pointer). These restrictions necessarily mean that ``const`` is also transitive for custom types built on these primitives.

As a result of this, syntax such as ``int* const`` is replaced by ``const(int*)``, to be clearer that both the pointer and the ``int`` to which it refers **cannot** be modified via this reference:

.. code-block:: c++

	void function() {
		int i = 0;
		const(int*) p = &i;
		*p = 1; // ERROR!
	}

This is unlike C and C++, for which ``const`` is **not** transitive for pointer types:

.. code-block:: c++

	void function() {
		int i = 0;
		int* const p = &i;
		*p = 1; // This is valid.
	}

.. Note::
	``const(int*)`` is actually equivalent to ``const ptr_t<int>``, which actually means the ``int`` isn't ``const``. However the effect of the restrictions imposed by ``ptr_t`` (and all primitives) mean that it might as well be; ``const ptr_t<int>`` and ``ptr_t<const int>`` behave identically.

Not only is the syntax much simpler here but the semantics of transitive ``const`` are much more obvious. For example, consider:

.. code-block:: c++

	struct Example {
		int* value;
	};
	
	void function(const Example exampleInstance) {
		*(exampleInstance.value) = 42;
	}

This code would be valid in C++ but is *invalid* in Loci. The intention behind this approach is to provide behaviour that is clearer and more closely matches the intuition of developers.

Logical Const
-------------

Loci provides ``const`` to mark data as logically constant, which means that the fundamental memory contents of ``const`` objects may vary, as long as there is no change to the external behaviour of the object.

Specifically, this means callers are unable to observe any computational side effects of ``const`` methods on the object; the calls may be slower (in the case of mutex locking) or faster (in the case of caching) to execute, but the inputs/outputs should be unchanged.

Overriding Const
++++++++++++++++

As part of logical ``const``, Loci provides the '__override_const' keyword (similar to ``mutable`` in C++), which allows ``const`` methods to modify a member variable marked ``__override_const``.

.. code-block:: c++

	class ComplexResultCalculator(__override_const int result) {
		static create() {
			// Assume that the result can never be zero.
			return @(0);
		}
		
		int calculateResult() const {
			if (@result == 0) {
				@result = performSlowCalculation();
			}
			return @result;
		}
	}

``ComplexResultCalculator`` uses ``__override_const`` to provide caching behaviour in a ``const`` method, which is a valid use case. This relies on ``ComplexResultCalculator`` correctly invalidating its cache when necessary (e.g. if a non-``const`` method could affect the calculation result).

.. Note::
	The caching used by ``ComplexResultCalculator`` is **not** thread safe. Logical ``const`` does **not** mandate thread safety.

Other suitable use cases for ``__override_const``:

* Reference counting (e.g. ``shared_ptr``) - Changes to the count don't affect the current reference, hence the reference count is **not** observable.
* Locking - Some classes may wish to lock a mutex to be thread-safe in ``const`` methods; the locking/unlocking is **not** observable.

.. Warning::
	Developers should use ``__override_const`` with care, since inappropriate use would violate ``const``-correctness.

Casting Const Away
------------------

.. Note::
	Feature not currently implemented; awaiting further design consideration.

``const`` can be cast away if needed with ``const_cast``, but doing so could be very dangerous since it violates ``const``-correctness. In general the only valid use for ``const_cast`` is to modify the type of a pointer to support an API that fails to use ``const``, but it is guaranteed that the API does not modify the object:

.. code-block:: c++

	void doSomething(int i);
	
	void oldAPI(int* i) {
		doSomething(*i);
	}
	
	void f(const(int*) i) {
		oldAPI(const_cast<int *>(i));
	}
