Named Parameters
================

**NOTE**: Feature not currently implemented; awaiting further design consideration.

There are often cases where it is difficult to ascertain the purpose of parameters passed to a function, or where it is difficult to remember the order of parameters to a function. For example:

.. code-block:: c++

	void function1(Renderer& renderer, Circle& circle) {
		circle.draw(renderer, true);
	}

In this case, the meaning of the second argument to the method is unclear. Similarly in the following case it is difficult to identify the order of parameters:

.. code-block:: c++

	void function2(){
		final auto haystack = "Hello world!";
		final auto needle = "world";
		
		// Is it...
		bool found = canFindString(needle, haystack);
		
		// Or...
		bool found = canFindString(haystack, needle);
	}

Named parameters are a feature that aims to assist code documentation, as well as calling functions by the names of their parameters rather than the order, which in some cases is more meaningful. The two examples above can therefore be expressed clearly with this feature:

.. code-block:: c++

	void function1(Renderer& renderer, Circle& circle){
		circle.draw(renderer: renderer, drawOutline: true);
	}
	
	void function2(){
		bool found = canFindString(needle: "world", haystack: "Hello world!");
	}

This is facilitated by the ability to type named function pointers, which can be implicitly cast to ordered function pointers:

.. code-block:: c++

	void f(int first, int second);
	
	void function(){
		*(int)(int first, int second) namedFunctionPtr = f;
		*(int)(int, int) functionPtr = namedFunctionPtr;
		
		namedFunctionPtr(second: 1, first: 0);
		functionPtr(0, 1);
	}

