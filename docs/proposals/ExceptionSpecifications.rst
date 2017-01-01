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
