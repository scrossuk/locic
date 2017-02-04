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

* New keywords: ``api`` and ``implement``.
* Every file must be describing or implementing an API. This is expressed on the first line (either ``api ...`` or ``implement api ...``).
* ``namespace`` is removed; modules can reference the api name if needed to resolve clashes.

API description
~~~~~~~~~~~~~~~

As an example, here is how the API description for TCP streams might look:

.. code-block:: c++

	api std::tcp 0.1.0;

	import primitives 0.1.0;
	import std::{event, string} 0.1.0;

	export exception socket_error(string error);

	export interface in_buffer {
		const uint8_t* data() const;
		
		size_t size() const;
	}

	export interface out_buffer {
		uint8_t* data();
		
		size_t size() const;
	}

	export class tcp_stream {
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

The main function could be in a special ``entry`` API:

.. code-block:: c++

	implement api entry;

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

Unnecessary imports
~~~~~~~~~~~~~~~~~~~

The compiler should issue warnings for imports that aren't required by the module.

Generating API dependencies from implementation imports
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``import`` in an API means a dependency of the API, whereas ``import`` in the implementation means a dependency of the module; if the module completely encapsulates an API imported by its implementation then that API shouldn't be exposed as a dependency of its own API.

Any tool generates API imports will need to strip any non-dependency imports. An alternative is to make the dependencies explicit:

.. code-block:: c++

	implement api test 0.1.0 depends {
		api_that_we_expose 0.1.0
	}

	import api_that_we_encapsulate 0.1.0;

	export void f(api_that_we_expose::Type value) {
		api_that_we_encapsulate::f(value);
	}

Interacting with C++
~~~~~~~~~~~~~~~~~~~~

Since this proposal suggests removing namespaces, it creates interaction issues with C++. To address this, C++ namespaces would become Loci APIs:

.. code-block:: c++

	api c++::outer_namespace;

	import c++::outer_namespace::inner_namespace;

This proposal changes the syntax of APIs to now use ``::`` rather than ``.`` to make this more seamless.
