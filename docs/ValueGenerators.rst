Value Generators
================

Value Generators are ranges that produce a sequence of values. These are usually integers incrementing by 1 for use in :doc:`Control Flow <ControlFlow>`, however they could produce almost any sequence.

Counters
--------

std::counter
~~~~~~~~~~~~

This object will count from a lower bound *inclusive* to an upper bound *exclusive*, by a specified increment. For example:

.. code-block:: c++

	void example() {
		for (int i: std::counter<int>(0, 5, 1)) {
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

std::counter_incl
~~~~~~~~~~~~~~~~~

This is essentially equivalent to *std::counter* except that the upper bound is also treated inclusively. For example:

.. code-block:: c++

	void example() {
		for (int i: std::counter_incl<int>(0, 5, 1)) {
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

std::reverse_counter
~~~~~~~~~~~~~~~~~~~~

A reverse counter simply starts at the upper bound (inclusive) and decrements by the specified value until it reaches the lower bound (exclusive). For example:

.. code-block:: c++

	void example() {
		for (int i: std::reverse_counter<int>(5, 0, 1)) {
			printf(C"i = %d\n", i);
		}
	}

This will print:

::

	i = 5
	i = 4
	i = 3
	i = 2
	i = 1

As with *std::counter*, there's also a variant of this called *std::reverse_counter_incl*.

Custom Value Generators
-----------------------

Developers need not restrict themselves to the generators available in the standard library. Here's a custom generator for Fibonacci values:

.. code-block:: c++

	class fibonacci_counter (int currentValue, int nextValue) {
		static create() noexcept {
			return @(1, 1);
		}
		
		const int& front() const noexcept {
			return @currentValue;
		}
		
		void pop_front() noexcept {
			int previousValue = @currentValue;
			@currentValue = @nextValue;
			@nextValue += previousValue;
		}
		
		bool empty() const noexcept {
			// Never ends!
			return false;
		}
	}

