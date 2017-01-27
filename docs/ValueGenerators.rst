Value Generators
================

Value Generators are :doc:`ranges <Ranges>` that produce a sequence of values. These are usually integers incrementing by 1 for use in :doc:`Control Flow <ControlFlow>`, however they could produce almost any sequence.

Counters
--------

range()
~~~~~~~

This can be used to construct a range that counts from a lower bound *inclusive* to an upper bound *exclusive*. For example:

.. code-block:: c++

	void example() {
		for (auto i: range<int>(0, 5)) {
			printf(C"i = %d\n", i);
		}
	}

This starts from 0 and counts up by 1 until it reaches 5. Hence it will print:

::

	i = 0
	i = 1
	i = 2
	i = 3
	i = 4

range_incl()
~~~~~~~~~~~~

This is essentially equivalent to ``range()`` except that the upper bound is also treated inclusively. For example:

.. code-block:: c++

	void example() {
		for (auto i: range_incl<int>(0, 5)) {
			printf(C"i = %d\n", i);
		}
	}

This will print:

::

	i = 0
	i = 1
	i = 2
	i = 3
	i = 4
	i = 5

reversed()
~~~~~~~~~~

Any range can be reversed using this function.

.. Note::
	Currently the template parameters for ``reversed()`` must be explicitly specified, so ``reversed(range(1, 2))`` is currently ``reversed<int, range_t<int>>(range<int>(1, 2))``; once template argument deduction is implemented this will no longer be necessary.

.. code-block:: c++

	void example() {
		for (int i: reversed(range(0, 5))) {
			printf(C"i = %d\n", i);
		}
	}

This will print:

::

	i = 4
	i = 3
	i = 2
	i = 1
	i = 0

Custom Value Generators
-----------------------

Developers need not restrict themselves to the generators available in the standard library. They can simply implement one of the :doc:`standard ranges <Ranges>` or their own custom range. Here's a custom generator for Fibonacci values:

.. code-block:: c++

	class fibonacci_counter (int currentValue, int nextValue) {
		static create() noexcept {
			return @(1, 1);
		}
		
		const int& front() const noexcept {
			return @currentValue;
		}
		
		void skip_front() noexcept {
			int previousValue = @currentValue;
			@currentValue = @nextValue;
			@nextValue += previousValue;
		}
		
		bool empty() const noexcept {
			// Never ends!
			return false;
		}
	}

