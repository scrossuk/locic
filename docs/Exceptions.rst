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

Exception Specifiers
--------------------

*noexcept* is actually just a special case of Loci's *exception specifiers*, which follow a similar syntax to C++ but that are **statically checked** by the compiler. For example:

.. code-block:: c++

	import custom.library 1.0.0 {
		exception FileOpenFailedException();
		
		class File {
			static File open(const std::string& fileName) throw(FileOpenFailedException);
		}
	}

This code is very clear that *openFile* may only throw exceptions of type *FileOpenFailedException* (or derived exception types). As previously mentioned, this property will be statically checked by the compiler.

Here is the equivalent of the above *noexcept* using an *exception specifier* (though the former is recommended):

.. code-block:: c++

	int addInts(int a, int b) throw() {
		return a + b;
	}

The main reason to use specifiers is to produce APIs with clear failure modes, such as the file opening example expressed above. Omitting the exception specifier means that the function may throw any exception:

.. code-block:: c++

	import custom.library 1.0.0 {
		class File {
			static File open(const std::string& fileName);
		}
	}

This means that developers can choose to use exception specifiers where appropriate and avoid them otherwise. Typically, specifiers are appropriate for use in heavily used core APIs (such as the standard library), but inappropriate as part of application logic or a custom rarely used API.

In regard to :doc:`Module API versions <Modules>`, any changes to exception specifiers should be made in a new API version; for this reason it may be appropriate to use a generic exception type in exception specifiers (from which the client can obtain information about the error) and then throw derived exception types internally. For example:

.. code-block:: c++

	import custom.library 1.0.0 {
		exception FileException(std::string what);
		
		class File {
			static File open(const std::string& fileName) throw(FileException);
		}
	}

The *open* constructor method could now be implemented as:

.. code-block:: c++

	export custom.library 1.0.0 {
		exception FileException(std::string what);
		exception FileNotFoundException() : FileException("File not found.");
		exception FileAccessDeniedException() : FileException("File access denied.");
		
		class File(/* ... */) {
			static File open(const std::string& fileName) throw(FileException) {
				if (!fileExists(fileName)) {
					throw FileNotFoundException();
				}
				
				if (!fileIsAccessible(fileName)) {
					throw FileAccessDeniedException();
				}
				
				// etc...
			}
		}
	}

Overriding Static Analysis with Assert
--------------------------------------

The :doc:`Assert Statement <AssertStatement>` can be used to inform the compiler that a block of code will not throw, even though static analysis suggests it could. For example:

.. code-block:: c++

	import bool fileExists(const std::string& fileName) noexcept;
	
	import bool fileIsAccessible(const std::string& fileName) noexcept;
	
	import std::string readFile(const std::string& fileName);
	
	std::string readFileOrReturnNothing(const std::string& fileName) noexcept {
		if (fileExists(fileName) && fileIsAccessible(fileName)) {
			return readFile(fileName);
		} else {
			return "";
		}
	}

The compiler will reject this code as invalid, since *readFile* may throw but *readFileOrReturnNothing* is declared as *noexcept*. However, let's assume that *readFile* is known to not throw in the situation shown here. The programmer can assert this by doing:

.. code-block:: c++

	import bool fileExists(const std::string& fileName) noexcept;
	
	import bool fileIsAccessible(const std::string& fileName) noexcept;
	
	import std::string readFile(const std::string& fileName);
	
	std::string readFileOrReturnNothing(const std::string& fileName) noexcept {
		if (fileExists(fileName) && fileIsAccessible(fileName)) {
			assert noexcept {
				return readFile(fileName);
			}
		} else {
			return "";
		}
	}

This means the compiler will generate code to check this property at run-time when configured to do so (e.g. for a debug build), and otherwise trust the programmer and assume the property is true. Hence no error will be produced by the error in this case. Given that this overrides the assistance of static analysis, this should be done **with great care!**

A similar construct can be used for exception specifiers:

.. code-block:: c++

	import bool fileExists(const std::string& fileName) noexcept;
	
	import bool fileIsAccessible(const std::string& fileName) noexcept;
	
	import custom.library 1.0.0 {
		exception FileException(std::string what);
		exception FileNotFoundException() : FileException("File not found.");
		exception FileAccessDeniedException() : FileException("File access denied.");
		
		class File {
			static File open(const std::string& fileName) throw(FileException);
		}
	}
	
	bool tryToReadFile(const std::string& fileName) throw(FileAccessDeniedException) {
		if (fileExists(fileName)) {
			assert throw(FileAccessDeniedException) {
				auto file = File.open(fileName);
				return true;
			}
		} else {
			return false;
		}
	}

Again, this overrides the static analysis and so should be avoided in most cases; it usually makes sense to build exception-throwing code on top of *noexcept* code rather than the other way around.

Note that the structures described above are essentially equivalent to:

.. code-block:: c++

	void throwAnyFunction();
	
	void noThrowFunction() noexcept {
		try {
			throwAnyFunction();
		} catch (...) {
			unreachable;
		}
	}
	
	exception TypeOne();
	exception TypeTwo();
	
	void throwBothFunction() throw(TypeOne, TypeTwo);
	
	void throwOneFunction() throw(TypeOne) {
		try {
			throwBothFunction();
		} catch (TypeTwo e) {
			(void) e;
			unreachable;
		}
	}
	
	exception Parent();
	exception ChildOne() : Parent();
	exception ChildTwo() : Parent();
	
	void throwParentFunction() throw(Parent);
	
	void throwChildFunction() throw(ChildOne) {
		try {
			throwBothFunction();
		} catch (ChildOne e) {
			(void) e;
			throw;
		} catch (...) {
			unreachable;
		}
	}

In these cases the compiler's static analysis will recognise that the catch blocks prevent exceptions of the given types unwinding further.

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

