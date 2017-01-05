Ranges
======

.. Note::
	See :doc:`ValueGenerators` for built-in range implementations, such as ``range(start, end)``.

Ranges are a core part of Loci, making it possible to perform algorithms with sequences of values. There are multiple different kinds of ranges, representing the varying requirements of algorithms and capabilities of containers.

Every range can be described by the containers that **support** it and the algorithms that **use** it. The key point is that ranges are just interfaces for an algorithm to use a container.

Custom ranges can be defined at any time, and via :doc:`StructuralTyping` these can be **compatible with existing containers and algorithms** simply by having the correct methods.

Input Range
-----------

.. code-block:: c++

	template <typename T>
	interface input_range {
		bool empty() const;
		
		void skip_front();
		
		T front();
	}

Input ranges are the most common kind of range, due to their use in :ref:`for-each loops <foreachloop>`. Here's an example:

.. code-block:: c++

	for (const auto& value: range) {
		[...]
	}

An input range could be used to read an array, a list, an open file, network data etc.

This kind of range is generally useful for any kind of iteration. For example:

.. code-block:: c++

	template <typename T>
	require(has_zero<T> and addable<T>)
	T sum(input_range<T>& sum_range) {
		T total = T.zero();
		for (const auto& value: sum_range) {
			total += value;
		}
		return move total;
	}

The ``skip_front`` method is deliberately named to distinguish it from a ``pop_front`` operation; the latter is expected to always be a destructive operation whereas the former simply means we should move to the next element.

Bidirectional Range
-------------------

.. code-block:: c++

	template <typename T>
	interface bidirectional_range {
		bool empty() const;
		
		void skip_front();
		
		T front();
		
		void skip_back();
		
		T back();
	}

A bidirectional range is simply an input range that supports both accessing the first and last operations. This is useful for operations such as:

.. code-block:: c++

	template <swappable T>
	void reverse_inplace(bidirectional_range<T&>& reverse_range) {
		while (!reverse_range.empty()) {
			swap(reverse_range.front(), reverse_range.back());
			reverse_range.skip_front();
			reverse_range.skip_back();
		}
	}

Arrays, lists, files, etc. are all bidirectional ranges.

Output Range
------------

.. code-block:: c++

	template <typename T>
	interface output_range {
		void push_back(T value);
	}

Output ranges are append-only, so they can provided by an array, a list, a circular buffer etc.

A typical algorithm using output ranges would be a transformation (e.g. UTF-8 encoding).
