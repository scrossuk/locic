Literals
========

Literals provide a convenient way for developers to specify constants or initialisers for standard or custom types. Consider the following simple example:

.. code-block:: c++

	void function() {
		char c = 127;
		short s = 32767;
		int i = 32767;
		long l = 2147483647;
		// etc.
		
		unsigned char uc = 255u;
		unsigned short us = 65535u;
		unsigned int ui = 65535u;
		unsigned long ul = 4294967295u;
		// etc.
		
		float f = 1.0f;
		double d = 1.0;
	}

This example appears quite trivial, but it's worth noting the automatic types given to these literals. The rules for *signed* integer literals are:

* If less than or equal to maximum for int8_t (127), type is int8_t.
* If less than or equal to maximum for int16_t (32767), type is int16_t.
* If less than or equal to maximum for int32_t (2147483647), type is int32_t.
* etc.

The rules are similar for unsigned types. In the example above each integer variable is assigned its maximum literal; consider this:

.. code-block:: c++

	void function() {
		// ERROR: no cast from 'int16_t' to 'char'.
		char c = 128;
		
		// ERROR: no cast from 'int32_t' to 'short'.
		short s = 32768;
		
		// ERROR: no cast from 'int32_t' to 'int'.
		int i = 32768;
		
		// ERROR: no cast from 'int64_t' to 'long'.
		long l = 2147483648;
		
		// etc.
	}

The errors might appear invalid, since many programmers may expect int to be 32 bits. However, the C standard specifies that:

* 'short' must be an integer type of at least 16 bits.
* 'int' must be an integer type of at least 16 bits which is also at least as large as 'short'.
* 'long' must be an integer type of at least 32 bits which is also at least as large as 'int'.
* etc.

Loci is compatible with these rules, and therefore aims to enhance cross-architecture compatibility by preventing any casts which cannot be guaranteed to be valid across all compliant architectures. This means that semantic analysis of Loci code is completely independent of the target, and so provides a much more robust error checking procedure. It's rare that it's ever justified to use an undersized types; developers can simply use a larger type (or rely on fixed sized types such as 'int32_t').

Literal Specifiers
------------------

In order to make literals more useful Loci supports literal specifiers, which are names that can be used as either a prefix or suffix of a literal to generate a custom value related to that literal. For example:

.. code-block:: c++

	Complex integer_literal_i(int32_t value);
	
	Mass float_literal_kg(double value);
	
	void function() {
		Complex value = 10i;
		// ...
		
		Mass oneKg = 1.0kg;
		// ...
	}


