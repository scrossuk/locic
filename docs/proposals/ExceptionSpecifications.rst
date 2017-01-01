Proposal: Exception Specifications
==================================

.. Note::
	Feature awaiting further design consideration.

Exception specifications are a proposed feature to constrain the set of exceptions thrown by a function or method, by **static checking** at compile-time.

Usage
-----

.. code-block:: c++

	import custom.library 1.0.0 {
		exception FileOpenFailedException();
		
		class File {
			static File open(const std::string& fileName) throw(FileOpenFailedException);
		}
	}

This code is very clear that ``File.open`` may only throw exceptions of type ``FileOpenFailedException`` (or derived exception types). As previously mentioned, this property will be statically checked by the compiler.

Rationale
---------

The main reason to use specifications is to produce APIs with clear failure modes, such as the file opening example expressed above. Omitting the exception specification means that the function may throw any exception:

.. code-block:: c++

	import custom.library 1.0.0 {
		class File {
			static File open(const std::string& fileName);
		}
	}

This means that developers can choose to use exception specifications where appropriate and avoid them otherwise. Typically, specifications are appropriate for use in heavily used core APIs (such as the standard library), but inappropriate as part of application logic or a custom rarely used API.

In regard to :doc:`Module API versions <../Modules>`, any changes to exception specifications should be made in a new API version; for this reason it may be appropriate to use a generic exception type in exception specifications (from which the client can obtain information about the error) and then throw derived exception types internally. For example:

.. code-block:: c++

	import custom.library 1.0.0 {
		exception FileException(std::string what);
		
		class File {
			static File open(const std::string& fileName) throw(FileException);
		}
	}

The ``File.open`` constructor method could now be implemented as:

.. code-block:: c++

	exception FileNotFoundException() : FileException("File not found.");
	exception FileAccessDeniedException() : FileException("File access denied.");
	
	export custom.library 1.0.0 {
		exception FileException(std::string what);
		
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

Note that any change to the derived exception types (i.e. with an unchanged exception specification) thrown by a function is generally **not** considered a breaking change to the API.

Interaction with other features
-------------------------------

noexcept
~~~~~~~~

Exception specifications can never be empty. If the specification is not given, it is implicitly defined to be infinite (i.e. contain all possible exceptions).

This means the following code is invalid:

.. code-block:: c++

	int addInts(int a, int b) throw() {
		return a + b;
	}

As a result, exception specifications and ``noexcept`` are orthogonal: ``noexcept`` indicates *whether* an exception can be thrown and specifications indicate *what* exceptions can be thrown.

Templates
~~~~~~~~~

As with ``noexcept``, templated code may wish to add the specification of a template type's method to its own signature. The following syntax is proposed:

.. code-block:: c++

	template <throwset ThrowSet>
	interface RunnableType {
		void run() throw(ThrowSet);
	}
	
	template <typename T, throwset ThrowSet>
	require(T : RunnableType<ThrowSet>)
	void run(T& object) throw(ThrowSet) {
		object.run();
	}

This requires creating a new ``throwset_t`` type that represents an exception specification.

The language may also define a ``union`` operation that manipulates ``throwset``:

.. code-block:: c++

	template <typename T, throwset A, throwset B>
	require(T : RunnableType<A> and T : JumpableType<B>)
	void run(T& object) throw(A union B) {
		object.run();
		object.jump();
	}

``run()`` calls two methods so its specification is the union of their specifications. Other exception types may also be added:

.. code-block:: c++

	exception ExceptionA();
	exception ExceptionB();
	
	template <typename T, throwset ThrowSet>
	require(T : RunnableType<ThrowSet>)
	void run(T& object) throw(ThrowSet union { ExceptionA, Exception B });

.. Note::
	There is no need for an ``intersect`` operation on ``throwset``, as there is no mechanism to only throw common exception types of two or more functions. Furthermore, such an operation could result in an empty specification, which is illegal.

Assert
~~~~~~

Just as the :doc:`assert statement <../AssertStatement>` can be used with ``noexcept``, it also makes sense to define it for ``throw()``.

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
