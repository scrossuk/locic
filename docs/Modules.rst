Modules
=======

In C and C++
------------

In C and C++, no source file can access the structs or functions implemented by another file unless it has its own declarations. Here's an example in C++:

.. code-block:: c++

	// Source file 'A.cxx'.
	struct SomeStruct{ int i; };
	
	SomeStruct function(){
		SomeStruct s;
		s.i = 42;
		return s;
	}

.. code-block:: c++

	// Source file 'B.cxx'.
	
	struct SomeStruct{ int i; };
	SomeStruct function();
	
	int main(){
		SomeStruct s = function();
		printf("Number is %d.\n", s.i);
		return 0;
	}

In this case, file 'B.cxx' must provide declarations for the implementations in file 'A.cxx', since the compiler compiles each source file separately and therefore without reference to other source files.

Header Files
~~~~~~~~~~~~

This is of course problematic if the structures and functions defined in one source file need to be used in many source files, particularly if there are many functions, in which case re-writing the declarations for each source file wastes development time. The solution is to use header files:

.. code-block:: c++

	// Header 'Header.hpp'.
	
	struct SomeStruct{ int i; };
	SomeStruct function();

.. code-block:: c++

	// File 'A'.
	#include "Header.hpp"
	
	SomeStruct function(){
		SomeStruct s;
		s.i = 42;
		return s;
	}

.. code-block:: c++

	// File 'B'.
	#include "Header.hpp"
	
	int main(){
		SomeStruct s = function();
		printf("Number is %d.\n", s.i);
		return 0;
	}

Header files take advantage of the preprocessor, via an \#include, to copy the text from a header file directly into a number of source files.

Header Guards
~~~~~~~~~~~~~

Headers may also need to include each other, but unfortunately this can create problems:

.. code-block:: c++

	// Header 'Header1.hpp'.
	struct SomeStruct{ int i; };

.. code-block:: c++

	// Header 'Header2.hpp'.
	#include "Header1.hpp"
	
	SomeStruct function();

.. code-block:: c++

	// File 'A'.
	#include "Header1.hpp"
	#include "Header2.hpp"
	
	SomeStruct function(){
		SomeStruct s;
		s.i = 42;
		return s;
	}

.. code-block:: c++

	// File 'B'.
	#include "Header2.hpp"
	
	int main(){
		SomeStruct s = function();
		printf("Number is %d.\n", s.i);
		return 0;
	}

This will cause an error in 'A.cxx' since the header 'Header1.hpp' is included twice, and therefore the struct definition appears twice in its code, which is not allowed in C or C++. The solution to this problem is header guards:

.. code-block:: c++

	// Header 'Header1.hpp'.
	#ifndef HEADER1_HPP
	#define HEADER1_HPP
	
	struct SomeStruct{ int i; };
	
	#endif

.. code-block:: c++

	// Header 'Header2.hpp'.
	#ifndef HEADER2_HPP
	#define HEADER2_HPP
	
	#include "Header1.hpp"
	
	SomeStruct function();
	
	#endif

.. code-block:: c++

	// File 'A'.
	#include "Header1.hpp"
	#include "Header2.hpp"
	
	SomeStruct function(){
		SomeStruct s;
		s.i = 42;
		return s;
	}

.. code-block:: c++

	// File 'B'.
	#include "Header2.hpp"
	
	int main() {
		SomeStruct s = function();
		printf("Number is %d.\n", s.i);
		return 0;
	}

Loci Modules
------------

The C++ code above does manage to finally solve this problem, but at the expense of the creation of extra files and writing macros to protect headers. Most importantly, the above code duplicates information; all the information is available in the source files, so if the compiler were to analyse multiple source files together, the programmer need never write header files.

Put simply, this is exactly what happens in Loci. When you compile source files, you pass a group of files to the compiler which will then analyse them together and 'extract' the appropriate declarations from definitions in the code. These groups will typically be all the files that produce a single static or shared library, or even an executable.

Note that this solution isn't complete, since libraries and executables do need to handle shared data structures and functions. For example, a library may define a specific API for use by the executable, but the library code should be compiled separately to the executable code.

Some sort of header files are needed for this case, which in Loci are simply normal '.loci' files that contain declarations as opposed to definitions. It is planned in future that, in contrast to C or C++, there would be tools to automatically generate these 'header files' for the developer, after which they may apply their own customisations.

Loci projects can be broken down into one or more modules, with each module consisting of one or more source files. The sources files for a module are all passed to the Loci compiler, which then generates the relevant assembly code (LOCIC generates LLVM IR) for the module. These outputs can then be linked together to produce the final project binaries.

To enable this process, Loci supports *importing* and *exporting* symbols and constructs from a module. Any symbols or constructs not marked for import and export are considered to be internal, and therefore unavailable for linking.

Imports
~~~~~~~

Here's an example of an imported function:

.. code-block:: c++

	import double sqrt(double value) noexcept;

This code imports an external function named 'sqrt', presumably referring to the C standard library function. This *import* statement is 'unnamed', meaning that no module name is provided and therefore no symbol name mangling occurs (assuming the function is in the global namespace).

Here's another example using a named *import* statement:

.. code-block:: c++

	import custom.module 1.0.0 {
		class CustomClass {
			static CustomClass Create();
			
			void doSomething();
		}
	}

In this case the class 'CustomClass' is being imported from another module with name 'custom.module' and API version '1.0.0'. Note that the API version must exactly match that used by the corresponding module's export statement.

Exports
~~~~~~~

This is extremely similar to the *import* statements. Here are examples matching the above:

.. code-block:: c++

	export double sqrt(double value) noexcept {
		return internalSqrtFunction(value);
	}

.. code-block:: c++

	export custom.module 1.0.0 {
		class CustomClass(int member0, int member1) {
			static Create() {
				return @(1, 2);
			}
			
			void doSomething() {
				printf(C"CustomClass(%d, %d)\n", @member0, @member1);
			}
		}
	}

API Versions
~~~~~~~~~~~~

API versions are an extremely useful feature that ensures developers can retain module compatibility while continuing to enhance their APIs. In the example above 'custom.module' is exporting 'CustomClass' with API version 1.0.0.

It may turn out that the module developer decides to add a new class, change a method name, perhaps even modify documented behaviour etc. Any of these changes should be provided by a new API version; **DO NOT MODIFY AN EXISTING API!** Here's an example:

.. code-block:: c++

	import custom.module 1.0.0 {
		class CustomClass {
			static CustomClass Create();
			
			void doSomething();
		}
	}
	
	import custom.module 1.1.0 {
		class CustomClass {
			static CustomClass Create();
			
			// New method!
			int getFirstValue() const;
			
			// Renamed from 'doSomething'.
			void printValues() const;
		}
	}

This is a Loci 'header' that a module developer might provide to clients (as mentioned above, tools will soon be available to automate most of this process). A few changes have been made to the module in the new API version, however the old API version is also provided. This means that existing clients can continue to use the old API version without any issues, while new clients can use the new API version; existing clients can of course upgrade in due course.

It's important to note that the version here is *not* the release version; you could release multiple versions of software that implement the same set of APIs. The version here therefore refers to the API, and should be incremented according to changes in the API.

Here's how a client might use this API:

.. code-block:: c++

	using custom.module 1.0.0;
	
	void function() {
		auto object = CustomClass();
		object.doSomething();
	}

Here's another iteration of the API:

.. code-block:: c++

	import custom.module 1.0.0 {
		class CustomClass {
			static CustomClass Create();
			
			void doSomething();
		}
	}
	
	import custom.module 1.1.0 {
		class CustomClass {
			static CustomClass Create();
			
			int getFirstValue() const;
			
			void printValues() const;
		}
	}
	
	import custom.module 1.2.0 {
		class CustomClass {
			static CustomClass Create();
			
			int getFirstValue() const;
			
			// New method!
			int getSecondValue() const;
			
			void printValues() const;
		}
	}

There are now 3 API versions here, and it's possible the module developer is now overloaded maintaining backwards compatibility! Hence they might decide to deprecate the oldest API:

.. code-block:: c++

	import(deprecated) custom.module 1.0.0 {
		class CustomClass {
			static CustomClass Create();
			
			void doSomething();
		}
	}
	
	import custom.module 1.1.0 {
		class CustomClass {
			static CustomClass Create();
			
			int getFirstValue() const;
			
			// Renamed from 'doSomething'.
			void printValues() const;
		}
	}
	
	import custom.module 1.2.0 {
		class CustomClass {
			static CustomClass Create();
			
			int getFirstValue() const;
			
			int getSecondValue() const;
			
			void printValues() const;
		}
	}

In this case, the compiler will generate warnings when clients re-build their code to encourage them to upgrade to a more recent API.

Implementation
--------------

See:

* :doc:`Multi-pass Compilation <MultiPassCompilation>`
* :doc:`Name Mangling <NameMangling>`


