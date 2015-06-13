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
* :ref:`predicate-aliases`

*and* and *or* expressions both have left associativity; for clarity these expressions can only be combined using parentheses.

Using Predicates
----------------

Predicates can be used in the following situations:

* :ref:`require-predicates` - Specify that some condition must hold for a function/class to be used.
* :ref:`move-predicates` - Specify that some condition must hold for a type to be movable.
* :ref:`const-predicates` - Specify that some condition must hold for a type to be `const`.
* :ref:`noexcept-predicates` - Specify that some condition must hold for a function to be `noexcept`.

.. _require-predicates:

Require Predicates
~~~~~~~~~~~~~~~~~~

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

.. _move-predicates:

Move predicates
~~~~~~~~~~~~~~~

Move predicates are just require() predicates for the type's implicitly generated :ref:`__moveto method <customising-move-operations>`. For example:

.. code-block:: c++

	template <typename T>
	move(T : movable)
	class TestClass { }

This just says that TestClass is only movable if the type parameter T is movable.

.. _const-predicates:

Const Predicates
~~~~~~~~~~~~~~~~

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

.. _noexcept-predicates:

Noexcept Predicates
~~~~~~~~~~~~~~~~~~~

Much like the above cases it's possible to use predicates inside a *noexcept* qualifier. For example:

.. code-block:: c++

	template <bool IsNoExcept>
	void f() noexcept(IsNoExcept) { }
	
	template <bool IsNoExcept>
	void g() noexcept(IsNoExcept) {
		f<IsNoExcept>();
	}

The compiler will try to prove that the caller function's *noexcept* predicate implies the called function's *noexcept* predicate (a ``noexcept(false)`` can call a ``noexcept(true)`` function but not vice versa), hence in this case ``IsNoExcept implies IsNoExcept``.

.. _predicate-aliases:

Predicate Aliases
-----------------

Loci has a generalised `using` statement that makes it possible to create aliases for predicates. For example:

.. code-block:: c++

	interface Socket {
		size_t read(uint8_t* data, size_t size);
		size_t write(const uint8_t* data, size_t size);
	}
	
	template <typename T>
	using IsSocket = T : Socket;

This can then be used in predicates, such as:

.. code-block:: c++

	template <typename T>
	require(IsSocket<T>)
	void writeZeroes(T& socket) {
		while (true) {
			const uint8_t zeroByte = 0u;
			const size_t writeSize = socket.write(&zeroByte, 1u);
			if (writeSize == 0u) {
				return;
			}
		}
	}

Built-in aliases
~~~~~~~~~~~~~~~~

Here are a few built-in aliases that can be used to query type properties:

* `is_movable<T>` - check if type T is movable
* `is_copyable<T>` - check if type T can be copied (has `copy` method)
* `is_noexcept_copyable<T>` - check if type T can be copied without throwing exceptions
* `is_implicit_copyable<T>` - check if type T can be implicitly copied (has `implicit_copy` method)
* `is_noexcept_implicit_copyable<T>` - check if type T can be implicitly copied without throwing exceptions
* `is_comparable<T>` - check if type T can be compared to itself
* `is_noexcept_comparable<T>` - check if type T can be compared to itself without throwing exceptions
* `is_default_constructible<T>` - check if type T has be constructed with no arguments
* `is_dissolvable<T>` - check if type T can be dissolved (see :doc:`LvaluesAndRvalues`)
* `is_const_dissolvable<T>` - check if type T can be dissolved to produce `const` reference

Indirect Requirements
~~~~~~~~~~~~~~~~~~~~~

Sometimes predicate aliases will have particular requirements themselves; for example:

.. code-block:: c++

	template <typename T>
	require(is_movable<T>)
	interface CreateValue {
		T createValue();
	}
	
	template <typename T, typename CreateType>
	require(is_movable<CreateType>)
	using CanCreateValue = T : CreateValue<CreateType>;

Here the type being created must be movable because it's being returned from a function. When this predicate is used this requirement must be re-stated:

.. code-block:: c++

	template <typename T, typename CreateType>
	require(is_movable<CreateType> and CanCreateValue<T, CreateType>)
	CreateType createValue(T& value) {
		return value.createValue();
	}

If this isn't the intention, you can add the requirement to the alias predicate, such as:

.. code-block:: c++

	template <typename T, typename CreateType>
	require(is_movable<CreateType>)
	using CanCreateValue = is_movable<CreateType> and T : CreateValue<CreateType>;

Hence the user only needs to specify the alias:

.. code-block:: c++

	template <typename T, typename CreateType>
	require(CanCreateValue<T, CreateType>)
	CreateType createValue(T& value) {
		return value.createValue();
	}

This works because the alias is 'inlined' so that the `createValue` function directly requires that `CreateType` is movable. Given this is a requirement it must be true and hence can be used to satsify the requirement posed by the alias. This behaviour can be quite confusing and it's likely that in future the semantics of this will be simplified.
