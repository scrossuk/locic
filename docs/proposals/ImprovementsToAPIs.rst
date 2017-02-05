Proposal: Improvements to APIs
==============================

.. Note::
	Feature awaiting further design consideration.

This proposal suggests some simplifications to how APIs are described.

Rationale
---------

* Currently we have too much indentation for export/import.
* Namespaces add further indentation and verbosity.
* It would be simpler not to have multiple API descriptions/implementations per source file.

Design
------

The key points of the design are:

* New keywords: ``api``, ``implement`` and ``depends``.
* Every file must be describing or implementing an API. This is expressed on the first line (either ``api ...`` or ``implement api ...``).
* ``namespace`` is removed; modules can reference the api name if needed to resolve clashes.

API description
~~~~~~~~~~~~~~~

As an example, here is how the API description for TCP streams might look:

.. code-block:: c++

	api std::tcp 0.1.0 depends {
		primitives 0.1.0,
		std::event 0.1.0,
		std::string 0.1.0
	}

	exception socket_error(string error);

	interface in_buffer {
		const uint8_t* data() const;
		
		size_t size() const;
	}

	interface out_buffer {
		uint8_t* data();
		
		size_t size() const;
	}

	class tcp_stream {
		event_source all_events() const noexcept;
		
		event_source read_events() const noexcept;
		
		event_source write_events() const noexcept;
		
		bool valid() const noexcept;
		
		endpoint peer() const noexcept;
		
		size_t read_some(out_buffer& destBuffer);
		
		size_t write_some(const in_buffer& sourceBuffer);
	}

The key aspects are:

* The file must start with ``api <name> <version>``.
* Each dependency must be specified with ``import <name> <version>;``. The dependencies themselves are specified in other source files.
* Each exported function/type must use ``export``.

API implementation
~~~~~~~~~~~~~~~~~~

Here's how the API implementation for ``varray`` might look:

.. code-block:: c++

	implement api std::container 0.1.0;

	import std::memory 0.1.0;

	/**
	 * \brief Resizable array.
	 */
	template <sized_type T>
	export class varray(size_t size, size_t capacity, T* data) {
		static create() noexcept {
			return @(0u, 0u, null);
		}
		
		...
	}

The key aspects are:

* The file must start with ``implement api <name> <version>``.
* Each dependency must be specified with ``import <name> <version>;``. The dependencies themselves are specified in other source files.
* Each exported function/type must use ``export``.

We use ``implement api ...`` rather than just ``implement ...`` to give a clear attachment between this code and the associated ``api ...`` (in another file).

Main function
~~~~~~~~~~~~~

The main function would be in a special unnamed API:

.. code-block:: c++

	import std::{io, string} 0.1.0;

	export int main(unused int argc, unused ubyte ** argv) {
		println("test!");
		return 0;
	}

Assigning imports to namespaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With the removal of ``namespace``, there could be clashes between imported names. A module can fix this by explicitly referencing the API it wants:

.. code-block:: c++

	implement api test 0.1.0;

	import first_api 0.1.0;
	import second_api 0.1.0;

	export void f() {
		first_api::f();
		second_api::f();
		
		// For a specific version.
		first_api@0.1.0::f();
	}

Importing API that depends on other API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Importing an API means that its dependencies would **not** also be imported:

.. code-block:: c++

	implement api test 0.1.0;

	import drive_car_api 0.1.0;

	export void f() {
		// ERROR: 'car_api' not imported.
		drive_car(car());
	}

This helps to avoid unnecessary imports (and hence name clashes). It can be fixed by adding the relevant ``import``:

.. code-block:: c++

	implement api test 0.1.0;

	import car_api 0.1.0;
	import drive_car_api 0.1.0;

	export void f() {
		// OK
		drive_car(car());
	}

Dependencies vs implementation imports
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``depends`` indicates dependencies of the API, whereas ``import ...`` in an implementation is a dependency of the **implementation**. A dependency of an API implies a dependency of the implementation, but not vice versa.

The compiler should issue warnings for dependencies that aren't required by the API:

.. code-block:: c++

	implement api test 0.1.0 depends {
		// ERROR: no constructs within 'car_api' are exposed by 'test'
		car_api 0.1.0
	}

	import drive_car_api 0.1.0;

	export void f() {
		// OK
		drive_car(car());
	}

The compiler should also issue warnings for imports that aren't by the implementation:

.. code-block:: c++

	implement api test 0.1.0;

	import car_api 0.1.0;
	import drive_car_api 0.1.0;
	
	// ERROR: no constructs within 'blah' are used by 'test'
	import blah 0.1.0;

	export void f() {
		// OK
		drive_car(car());
	}

Interacting with other languages
--------------------------------

With C
~~~~~~

Loci would able to inport C headers directly:

.. code-block:: c++

	implement api test 0.1.0;

	import c::memory 1.0.0: lang(c) <memory.h>;

	export void f() {
		(void) malloc(10);
	}

Internally the compiler would produce something like:

.. code-block:: c++

	api c::memory 1.0.0;

	void* malloc(size_t size);

	// etc..

With C++
~~~~~~~~

Similarly for C++:

.. code-block:: c++

	implement api test 0.1.0;

	import c++::vector 1.0.0: lang(c++) <vector>;

	export void f() {
		auto array = std::vector<int>();
	}

Internally the compiler would produce something like:

.. code-block:: c++

	api c++::vector 1.0.0;

	template <typename T>
	class std::vector {
		// etc.
	}

This proposal changes the syntax of APIs to now use ``::`` rather than ``.`` to make this more seamless.
