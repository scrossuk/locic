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

Final
-----

Consider the following code:

.. code-block:: c++

	void function() {
		int i = 0;
		const(int*) p = &i;
		*p = 1;
	}

In this case we may have intended to use ``const`` to prevent accidental assignments to p, but in this case due to the transitivity of ``const`` we've also disabled assignments to the value it points-to.

Fortunately the ``final`` keyword provides a way to prevent assignments to an lvalue without having to mark it ``const``. So the above code would become:

.. code-block:: c++

	void function() {
		int i = 0;
		final int* p = &i;
		*p = 1;
	}

Now the code will compile, but any assignments to 'p' itself fails. The implementation of this keyword is to use a ``final_lval`` as the underlying lvalue type, which doesn't support assignment in any case, rather than ``value_lval`` (which does support assignment for non-``const`` types).

Note that ``final`` is an lvalue qualifier (or 'variable qualifier') rather than a type qualifier, so doesn't affect Loci's type system in any way.

Logical Const
-------------

Loci provides 'const' to mark data as logically constant, which means that the fundamental memory contents of 'const' objects may vary, as long as there is no change to the external behaviour of the object. ''No change to the external behaviour" means the following two functions 'function' should be equivalent, and any transformation between them is valid:

.. code-block:: c++

	void f(const Type& value);
	void g(const Type& value);
	
	void function(){
		const Type var = SOME_EXPR;
		f(var);
		g(var);
	}
	
	void function(){
		const Type var = SOME_EXPR;
		const Type tmpVar = _copy_of_var_;
		f(var);
		g(tmpVar);
	}

Here '_copy_of_var_' means a simple byte-for-byte copy of variable 'var' that does not involve invoking a 'copy' method, and furthermore that the destructor for 'tmpVar' is not run. Therefore, this effectively means that the second function does not have to reload the value of the variable from memory, since it can assume that it has not changed.

Compilers are allowed to optimise (note also that optimisations can only be performed where the compiler can prove that it has the only (const) reference to an object, otherwise other parts of the program may have non-const references to the object and thereby modify it in parallel, or as part of, the execution of the function with the const reference) based on the validity of this transformation. This optimisation requires that 'f' and 'g' operate within type rules and don't use 'const_cast', and this is a requirement that the developer must follow. Consider, for example:

.. code-block:: c++

	// A type alias.
	using CString = const ubyte *;
	
	void unknownStringOperation(const CString string);
	void printSize(size_t size);
	void printStringLength(const CString string) {
		size_t length = 0u;
		CString ptr = string;
		while (*ptr != 0u) {
			length++;
			ptr++;
		}
		printSize(length);
	}
	
	void function() {
		// Prefix 'C' means 'C string'.
		const CString string = C"This is a string";
		unknownStringOperation(string);
		printStringLength(string);
	}

By the above equivalence, the compiler can assume this is equivalent to:

.. code-block:: c++

	// ... as above ...
	
	void function() {
		unknownStringOperation(C"This is a string");
		printStringLength(C"This is a string");
	}

Which, in combination with other transformations (such as inlining), leads to the optimised code:

.. code-block:: c++

	using CString = const ubyte *;
	
	void unknownStringOperation(const CString string);
	void printSize(size_t size);
	
	void function() {
		unknownStringOperation(C"This is a string");
		printSize(16u);
	}

Overriding Const
----------------

As part of 'logical const', Loci provides the '__override_const' keyword, which allows developers to explicitly ignore const markers if needed:

.. code-block:: c++

	struct Struct {
		int normalField;
		__override_const int mutableField;
	};
	
	void function(const Struct& ref) {
		// Invalid.
		ref.normalField = 1;
		
		// Valid.
		ref.mutableField = 1;
	}

Following the rules of logical const, '__override_const' should only ever be used when it has no effect on the external behaviour of an object. Again, this means the above transformation should apply. And since optimisations are allowed to occur based on const, it is important that developers only use '__override_const' when absolutely necessary and ensure correctness when it is used.

A good example of its correct use would be in a reference counting smart pointer class, in which the reference count field can (and should) be marked as '__override_const'. Considering the transformation above once again, it doesn't matter whether 'f' modifies the reference count (it could, for example, create a copy of the smart pointer and store it somewhere, increasing the reference count), because 'g' only depends on a count greater than 0 (and the reference counting invariant is intended to ensure that is always true until the last smart pointer object is destroyed).

Marking class member variable mutexes as '__override_const' is another example of a good use of the keyword, since 'lock' and 'unlock' methods modify the external behaviour of the mutex (consider calling 'lock' twice in a row, without calling 'unlock') and therefore require it to be non-const, but any object that contains a mutex to handle races uses it in a way that does not affect its external behaviour (i.e. the above transformation is valid):

.. code-block:: c++

	class Mutex {
		void lock();
		void unlock();
	}
	
	class Lock(Mutex& mutex) {
		static create(Mutex& mutex) {
			mutex.lock();
			return @(mutex);
		}
		
		~ {
			@mutex.unlock();
		}
	}
	
	class CustomType(__override_const Mutex mutex, Type value){
		// ...
		
		void setValue(Type value) {
			auto lock = Lock(@mutex);
			@value = value;
		}
		
		int getValue() const {
			// 'Lock' object will call 'lock'
			// and 'unlock' on the mutex.
			unused auto lock = Lock(@mutex);
			return @value;
		}
	}

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
