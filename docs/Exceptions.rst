Exceptions
==========

There has been (and is continuing to be) much debate on the relative merits of *checked exceptions*, *unchecked exceptions* and C-style error checking (along with other variants).

Based on an assessment of the implementations of exceptions in languages such as Java (checked), and C++ and C# (unchecked), Loci uses unchecked exceptions as a standard method for functions to report some form of failure, since these trigger fewer architectural maintenance problems.

Here's an example:

.. code-block:: c++

	exception GenericException(std::string what);
	exception RealException(std::string what, int i) : GenericException(what);
	
	void f(int i) {
		if (i < 0) {
			throw RealException("Function 'f' passed negative value.", i);
		}
	}
	
	void caller() {
		try {
			f(-1);
		} catch (GenericException exception) {
			const std::string& reason = exception.what;
			//...
		}
	}

Exceptions are allocated via a separate mechanism to normal objects, and unlike standard object types their hierarchies must be explicit (in contrast to :doc:`Structural Typing <StructuralTyping>`), primarily due to the performance implications this would have for a run-time downcast.

Recommended Usage
-----------------

Exceptions are intended to be used for runtime environment issues, such as:

* A socket being closed unexpectedly.
* A user providing incorrect input.
* An expected file (e.g. for configuration) does not exist.

Exceptions **should not** be used for programmer errors or normal control flow, such as:

* A *null* pointer was passed to a function that expected a non-*null* pointer.
* To indicate out-of-bounds access to an array.

Where exceptions are being thrown in the latter cases, they can usually be replaced by an :doc:`Assert Statement <AssertStatement>` (which traps rather than throwing an exception).

Rethrow
-------

Just like C++, you can rethrow exceptions in a catch block:

.. code-block:: c++

	void example() {
		try {
			throwingFunction();
		} catch (GenericException exception) {
			printf(C"Exception occurred!\n");
			throw;
		}
	}

Note that, unlike C++, constructs such as the following are disallowed:

.. code-block:: c++

	void example() {
		try {
			throwingFunction();
		} catch (GenericException exception) {
			printf(C"Exception occurred!\n");
			try {
				throw;
			} catch (OtherException otherException) {
				printf(C"Other exception occurred!\n");
			}
		}
	}

This is because Loci manages exceptions via unique ownership rules, rather than shared ownership rules, which could be violated in this case if there were two references to the same exception (requiring a more complex mechanism to determine when to destroy the object).

Noexcept
--------

If you know that a function won't throw, and won't ever have to do so in future (i.e. it cannot fail), then you can specify it as *noexcept*:

.. code-block:: c++

	int addInts(int a, int b) noexcept {
		return a + b;
	}

You should be able to use the *noexcept* specifier relatively often as long as you use the :doc:`Assert Statement <AssertStatement>` for issues such as verifying parameters are correct, as mentioned above. So in general you should do something like this:

.. code-block:: c++

	int addPositiveInts(int a, int b) noexcept {
		assert a > 0 && b > 0;
		return a + b;
	}

Consider a violation of this specifier:

.. code-block:: c++

	int addPositiveInts(int a, int b) noexcept {
		if (a < 1 || b < 1) {
			throw InvalidValues();
		}
		return a + b;
	}

This code is not valid and will be **rejected** by the compiler, since the *noexcept* property is statically checked.

Destructors
-----------

Consider the following code:

.. code-block:: c++

	class ExampleClass() {
		~ {
			throw SomeException();
		}
	}

Since Loci doesn't support throwing exceptions out of destructors, this code is broken. Fortunately destructors are automatically specified as *noexcept*, so a compiler error will be produced since this property is statically checked.

Cleanup
-------

**NEVER** use a catch block to perform cleanup actions. Instead, you can use one of:

* Destructor
* Scope exit block

Destructor Cleanup
~~~~~~~~~~~~~~~~~~

(Also known as *RAII*.)

In this case you create an object on the stack which manages the relevant resource, and in the destructor you perform the cleanup actions. Loci is very similar to C++ in this respect and so the same rules apply. Here's an example:

.. code-block:: c++

	class Resource(void* ptr) {
		static create() {
			return @(malloc(10u));
		}
		
		~ {
			free(@ptr);
		}
	}
	
	void function() {
		auto resourceObject = Resource();
	}

Scope Exit Block
~~~~~~~~~~~~~~~~

This is a construct inspired by the D programming language. Here's an example:

.. code-block:: c++

	int function() {
		scope (exit) {
			printf(C"Scope exit 1!\n");
		}
		scope (exit) {
			printf(C"Scope exit 2!\n");
		}
		printf(C"Returning 10...\n");
		return 10;
	}

This will output:

::

	Returning 10...
	Scope exit 2!
	Scope exit 1!

Note that scope exit blocks may only be exited 'normally'. That is, it cannot be exited in any of these ways:

* A *break* statement
* A *continue* statement
* A *return* statement
* By an exception

A *scope(exit)* block is run in all cases; there are also variants for 'success' (when a scope is exited normally or via control flow) and 'failure' (when a scope is exited due to an exception):

.. code-block:: c++

	int function() {
		scope (exit) {
			printf(C"Scope exit!\n");
		}
		scope (success) {
			printf(C"Scope success!\n");
		}
		scope (failure) {
			printf(C"Scope failure!\n");
		}
		throw SomeException();
	}

This will output:

::

	Scope failure!
	Scope exit!

To facilitate deterministically calling potentially-throwing functions, it's allowed to throw from a *scope(success)* block:

.. code-block:: c++

	int function() {
		scope (exit) {
			printf(C"Scope exit!\n");
		}
		scope (failure) {
			printf(C"Scope failure!\n");
		}
		scope (success) {
			printf(C"Scope success!\n");
			throw SomeException();
		}
		return 10;
	}

This will output:

::

	Scope success!
	Scope failure!
	Scope exit!

