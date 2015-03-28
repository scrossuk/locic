Strings
=======

Strings ('std::string') are used in Loci as the default way to handle character sequences. They have the following properties:

* Immutable
* Unicode support
* May use non-contiguous storage
* Concatenation is O(log N)
* Substring is O(log N)
* Indexing is O(log N)
* Iteration is O(N)

Hence the likely implementation of strings is using a 'rope' data structure. The internal representation is likely to use UTF-8. Here's some code using 'std::string':

.. code-block:: c++

	void function() {
		auto a = "Hello";
		auto b = "Hello" + " ";
		auto c = " " + "world!";
		auto d = b.substring(0, 5) + c;
		auto e = "Hello %s!".format("world");
		auto f = "%0 is written as '%1'.".format({ 42, "fourty two" });
	}

String Builder
--------------

If developers are seeking O(1) append, they can use 'std::string_builder', which has the following properties:

* Mutable
* Unicode support
* Appending is O(1) amortized
* No concatenation or substring
* Can produce Loci strings ('std::string')

Here's some code using 'std::string_builder':

.. code-block:: c++

	void function() {
		{
			auto builder = std::string_builder();
			builder.append("Hello");
			builder.append(" ");
			builder.append("world");
			builder.append("!");
			
			const std::string lociString = builder.str();
		}
		
		{
			auto builder = std::string_builder();
			builder.append(42);
			builder.append(" is written as '");
			builder.append("fourty two");
			builder.append("'.");
		}
	}

UTF-8 Buffer
------------

A UTF-8 buffer is a sequence of unicode characters represented in UTF-8 form.

* Mutable
* Unicode support
* Appending is O(1) amortized
* No concatenation or substring
* Can produce C strings ('const ubyte*')
* Can produce Loci strings ('std::string')

UTF-8 Encoder
-------------

A UTF-8 encoder is a transform range that consumes unicode characters and produces UTF-8 encoded bytes.

For example:

.. code-block:: c++

	void function(const std::varray<std::unichar>& unicodeCharacters) {
		for (const auto utf8Byte: std::utf8_encoder(unicodeCharacters.all())) {
			// Etc.
		}
	}

Literals
--------

Loci breaks away from C by making string literals (without specifiers) be of type 'std::string', rather than type 'const ubyte*':

.. code-block:: c++

	std::string f() {
		return "Hello " + "world!";
	}

However, as discussed on :doc:`Compatibility with C <CompatibilityWithC>`, C string literals can be created using the string :doc:`literal specifier <Literals>` 'C':

.. code-block:: c++

	void f(const(ubyte*) cString) {
		printf(C"String: %s\n", cString);
	}
	
	void function() {
		// Invalid - no implicit cast from std::string to const ubyte*.
		f("Hello world!");
		
		// Valid - literal prefixed with 'C' gives C string.
		f(C"Hello world!");
		
		// Also valid - literal specifiers can be suffixes.
		f("Hello world!"C);
	}

Flyweight Strings
-----------------

It is planned to soon add support for 'flyweight strings', which are strings 'uniqued' by a flyweight factory object (this pattern is used in the Loci compiler itself). In this design the factory stores a collection of the actual objects, merging any identical objects and then pointers to those objects called 'flyweights' are manipulated in the code. The most significant gain is that copying and comparison operators become extremely cheap:

* **Copying** - Copy the pointer.
* **Comparison** - Compare the pointers.
