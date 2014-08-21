Operator Overloading
====================

Loci supports operator overloading as a useful way to provide a clean syntax for manipulating standard library or user defined types such as strings, vectors, matrices and arrays. Unlike C++, Loci uses normal function names to define methods that can be used in operator overloading:

.. code-block:: c++

	template <typename T>
	interface BasicNumber {
		compare_result_t compare(const T& other) const;
		T add(const T& other) const;
		T substract(const T& other) const;
	}
	
	interface IntArray {
		// Arrays should return an lval for array members to
		// allow various lval operations; custom lval types
		// can be defined for special behaviour (or to eliminate
		// some operations).
		lval<int> value_lval<int>& index(size_t index);
	}
	
	template <typename T: BasicNumber<T>>
	void function(const T& a0, const T& a1, IntArray& array) {
		// All following pairs are equivalent...
		bool compare = a0.compare(a1).isLessThan();
		bool compare = a0 < a1;
		
		T a2 = a0.add(a1);
		T a2 = a0 + a1;
		
		T a3 = a0.subtract(a1);
		T a3 = a0 - a1;
		
		array.index(0).assign(5);
		array[0] = 5;
		
		int i = array.index(0).dissolve().implicitCopy();
		int i = array[0];
	}

Of particular interest is the *compare* method, which is an alternative to defining each of the comparison operators individually. Instead, developers just implement this single method which uses a *compare_result_t* primitive value to indicate whether the left value is less than, equal to, or greater than the right value.

This functionality can avoid double comparisons (i.e. two applications of the less-than operator with left and right values swapped) inside container types (and other relevant functions/types). This is particularly beneficial for objects that may be expensive to compare, such as strings and memory blocks, but for which a single comparison can identify whether a value is less than, equal to, or greater than another value.

Operator Overloading is purely syntactic sugar for method calls, so there are no relevant implementation considerations.

