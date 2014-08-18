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

Char Sequence
-------------

If developers are seeking O(1) append, they can use 'std::char_sequence', which has the following properties:

* Mutable
* Unicode support
* Must use contiguous storage
* Indexing is O(1)
* Appending is O(1) amortized
* No concatenation or substring
* Iteration is O(N)
* Can produce C strings ('const byte* const')
* Can produce Loci strings ('std::string')

Here's some code using 'std::char_sequence':

.. code-block:: c++

	void function() {
		{
			auto sequence = std::char_sequence();
			sequence.append("Hello");
			sequence.append(" ");
			sequence.append("world");
			sequence.append("!");
			
			const byte* const cString = sequence.c_str();
			const std::string lociString = sequence.str();
		}
		
		{
			auto sequence = std::char_sequence();
			sequence.append(42);
			sequence.append(" is written as '");
			sequence.append("fourty two");
			sequence.append("'.");
		}
	}

Literals
--------

Loci breaks away from C by making string literals (without specifiers) be of type 'std::string', rather than type 'const byte * const'. However C string literals can be declared using the string literal specifier 'C':

.. code-block:: c++

	void f(const byte * const cString) {
		printf(C"%s", cString);
	}
	
	void function() {
		// Invalid - no implicit cast from std::string to const byte* const.
		f("Hello world!");
		
		// Valid - literal prefixed with 'C' gives C string.
		f(C"Hello world!");
		
		// Also valid - literal specifiers can be suffixes.
		f("Hello world!"C);
	}


