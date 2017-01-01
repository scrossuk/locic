Proposal: Vector Types
======================

.. Note::
	Feature awaiting further design consideration.

Loci supports SIMD operations directly through the use of vector types, allowing operations such as addition to be applied in parallel to a vector of integer or floating point values.

.. code-block:: c++

	void function(){
		int!4 a = {0, 1, 2, 3};
		int!4 b = {5, 6, 7, 8};
		int!4 c = a + b;
		
		// The resulting values.
		assert c[0] == 5;
		assert c[1] == 7;
		assert c[2] == 9;
		assert c[3] == 11;
	}

The ternary operator also provides some interesting behaviour for vectorised types:

.. code-block:: c++

	void function(){
		int!4 a = {0, 1, 2, 3};
		int!4 b = {4, 5, 6, 7};
		bool!4 c = {true, false, true, false};
		
		int!4 d = c ? a : b;
		
		// The resulting values.
		assert d[0] == 0;
		assert d[1] == 5;
		assert d[2] == 2;
		assert d[3] == 7;
	}

