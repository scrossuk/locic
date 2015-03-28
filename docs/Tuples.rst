Tuples
======

**NOTE**: Feature not currently implemented; awaiting implementation.

Tuples are a useful way of passing around multiple values of different types together, particularly in the case of returning multiple values.

.. code-block:: c++

	(bool, int, float) f(){
		return (true, 1, 1.5);
	}
	
	void function(){
		(a, b, c) = f();
		//...
	}

This example shows the creation of a 3-tuple, returning it from a function, and then retrieving the values within it by pattern matching on the type, so that variables 'a', 'b' and 'c' are bound to the values in the tuple.


