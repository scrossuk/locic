Predicates
==========

Predicates are one of the key foundations of Loci's type checking system, which allows specifying constraints on template types, provide optional methods if an underlying type includes the necessary support for those methods and to create implications of type capabilities.

Here's a relevant example:

.. code-block:: c++

	template <typename T>
	require(T : movable)
	class TestClass(T value) {
		// ...
		
		TestClass<T> copy() const require(T : copyable<T>) {
			return @(@value.copy());
		}
		
		// ...
	}

Here we have a value inside the class that may or may not support copying. In the case that the value is copyable then it will be possible to call the copy() method of the class, which will then copy the internal value. If the value is not copyable then the predicate will not be satisfied and a compiler error will be produced.

Note that the most similar feature in C++ to predicates is concepts.

Predicate Expression
--------------------

A predicate consists of an expression that given particular inputs (i.e. template arguments) either evaluates to true or false.

The following expressions are allowed in a predicate:

* ``true`` or ``false`` - Constant literals for true and false cases.
* ``Var`` - True if the boolean template argument is true.
* ``Type : RequireType`` - True if 'Type' provides all the methods in 'RequireType'.
* ``Expression and Expression`` - True if both subexpressions are true.
* ``Expression or Expression`` - True if at least one subexpression is true.
* ``( Expression )`` - True if the enclosed expression is true (brackets used for precedence).

*and* and *or* expressions both have left associativity; *and* expressions have 'tighter' binding.

Require Predicates
------------------

The above example shows predicates being used in a *require* qualifier, which expresses the template's requirements of the parameters provided. These can be used on a per-class basis:

.. code-block:: c++

	template <typename T>
	require(T : Startable and T : Stoppable)
	class TestClass(/* ... */) {
		// ...
	}

Note that the requirement types here are just interfaces (and *must* be interfaces), so you could have:

.. code-block:: c++

	interface Startable {
		void start();
	}

It's then possible to use the form above to specify requirements for a template type argument as needing to have the methods in this interface.

Where only a single requirement is specified for a type argument a shorter form can be used:

.. code-block:: c++

	template <typename T: Startable>
	class TestClass(/* ... */) {
		// ...
	}

If a template is instantiated with arguments that *do not* satisfy the require predicate then the compiler will issue an appropriate error.

Sometimes a function or class will have requirements for a template argument, and that will then be used by another function or class which augments those requirements. For example:

.. code-block:: c++

	template <typename T: Startable>
	void startObject(T& object) {
		object.start();
	}
	
	template <typename T>
	require(T : Startable and T : Stoppable)
	void restartObject(T& object) {
		object.stop();
		startObject(object);
	}

In this case the compiler proves that the predicate for *restartObject* implies the predicate for *startObject*; i.e. it proves ``(T : Startable and T : Stoppable) implies T : Startable``.

Move predicates
~~~~~~~~~~~~~~~

Move predicates are just require() predicates for the type's implicitly generated :doc:`__moveto method <MoveVersusCopy>`. For example:

.. code-block:: c++

	template <typename T>
	move(T : movable)
	class TestClass { }

This just says that TestClass is only movable if the type parameter T is movable.

Const Predicates
----------------

C++ developers are likely familiar with the idea of *const* overloading; here's an example in C++:

.. code-block:: c++

	// This is C++ code!
	class TestClass {
	public:
		// ...
		
		int* data() {
			return &mData;
		}
		
		const int* data() const {
			return &mData;
		}
		
		// ...
	private:
		int mData;
		
	}

This code addresses the problem that if the class instance is *const* then the returned pointer from the method must also be const, but when the class is not *const* it is desired to return a non-const pointer.

In Loci, *const* predicates are used to solve this problem without having to write code more than once:

.. code-block:: c++

	class TestClass(int data) {
		// ...
		
		template <bool IsConst>
		const<IsConst>(int)* data() const(IsConst) {
			return &@data;
		}
		
		// ...
	}

This code has a predicate based on the boolean template argument that determines whether the method returns a const or non-const pointer.

When the compiler analyses the method using the const predicate it is looking to ensure that a non-const pointer is never returned when the class instance is const (a ``const(false)`` type can be cast to a ``const(true)`` type but not vice versa). Hence it will attempt to prove that when 'IsConst' is true, that the returned pointer's const predicate (also 'IsConst') is also true. In other words it proves that ``IsConst implies isConst``, which is clearly a trivial operation.

The useful aspect of this code is that the code is only written once and the compiler will prove its correctness for both const and non-const forms.

Noexcept Predicates
-------------------

Much like the above cases it's possible to use predicates inside a *noexcept* qualifier. For example:

.. code-block:: c++

	template <bool IsNoExcept>
	void f() noexcept(IsNoExcept) { }
	
	template <bool IsNoExcept>
	void g() noexcept(IsNoExcept) {
		f<IsNoExcept>();
	}

The compiler will try to prove that the caller function's *noexcept* predicate implies the called function's *noexcept* predicate (a ``noexcept(false)`` can call a ``noexcept(true)`` function but not vice versa), hence in this case ``IsNoExcept implies IsNoExcept``.
