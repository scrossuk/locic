Control Flow
============

Loci has two constructs for flow control:

* While loop
* For-each loop

While Loop
----------

This is a very standard construct that exists in many other languages. For example:

.. code-block:: c++

	void example() {
		int i = 0;
		while (i < 5) {
			printf(C"i = %d\n", i);
			i += 1;
		}
	}

This will of course print:

::

	i = 0
	i = 1
	i = 2
	i = 3
	i = 4

.. _foreachloop:

For-each Loop
-------------

Again, this is very similar to constructs offered by many languages. In this case, the syntax and semantics are essentially identical to C++11's *for-each* loop.

Here's an example:

.. code-block:: c++

	void example(std::varray<int> array) {
		for (int element: array.all()) {
			printf(C"element = %d\n", element);
		}
	}

C/C++ For Loop
--------------

Note that Loci doesn't support a *for* loop as has existed in C and C++ for a long time. The primary reason is that such a construct can be easily replicated by either a *while* loop and *for-each* loop; usually the latter is most suitable.

For example, consider this C++ example:

.. code-block:: c++

	void example() {
		for (int i = 0; i < 5; i++) {
			printf(C"i = %d\n", i);
		}
	}

In Loci the equivalent idiomatic code would be:

.. code-block:: c++

	void example() {
		for (int i: range<int>(0, 5)) {
			printf(C"i = %d\n", i);
		}
	}

These examples are very similar, though the latter is arguably better since it doesn't modify the variable *i*. For example, you could make *i* const:

.. code-block:: c++

	void example() {
		for (const int i: range<int>(0, 5)) {
			printf(C"i = %d\n", i);
		}
	}

``range()`` is an example of a :doc:`Value Generator <ValueGenerators>`.

Break/Continue
--------------

Just like C and C++, Loci provides *break* and *continue* and these behave in exactly the same way.

Here's an example:

.. code-block:: c++

	void example() {
		for (const int i: range<int>(0, 5)) {
			if (i == 1) {
				continue;
			} else if (i == 4) {
				break;
			}
			printf(C"i = %d\n", i);
		}
	}




