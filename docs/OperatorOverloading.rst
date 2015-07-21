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
		bool compare = a0.compare(a1).is_less_than();
		bool compare = a0 < a1;
		
		T a2 = a0.add(a1);
		T a2 = a0 + a1;
		
		T a3 = a0.subtract(a1);
		T a3 = a0 - a1;
		
		nolval(array.index(0)).assign(5);
		array[0] = 5;
		
		int i = nolval(array.index(0)).dissolve().implicit_copy();
		int i = array[0];
	}

Operator Overloading is purely syntactic sugar for method calls, so there are no relevant implementation considerations.

.. _compare_methods:

Compare Method
--------------

Of particular interest is the *compare* method, which is an alternative to defining each of the comparison operators individually. Instead, developers just implement this single method which uses a *compare_result_t* primitive value to indicate whether the left value is less than, equal to, or greater than the right value.

This functionality can avoid double comparisons (i.e. two applications of the less-than operator with left and right values swapped) inside container types (and other relevant functions/types). This is particularly beneficial for objects that may be expensive to compare, such as strings and memory blocks, but for which a single comparison can identify whether a value is less than, equal to, or greater than another value.

Operator Combinations
---------------------

In other languages particular combinations of operators can often lead to confusion. For example, consider the expression ``a << b & c`` in C++; in this case the shift has higher precedence so the equivalent bracketed expression is ``(a << b) & c``, but many developers might think it could be ``a << (b & c)``. There are many cases that appear in practice in code and can confuse developers.

Loci eliminates this problem by constraining operator combinations, meaning that the expression ``a << b & c`` is invalid. Instead the developer must specify either ``(a << b) & c`` or ``a << (b & c)`` and hence avoid writing an expression which is confusing, or worse yet not what they had intended. A nice side effect of these constraints is the simplification they bring to the language grammar.

Many combinations are well understood, such as ``a * b + c``, so Loci allows these. Essentially the process of developing the grammar of Loci has been to identify operator combinations and consider whether they might lead to confusion (as above) or are well understood.

Operator Grammar
----------------

The following describes the language grammar of values in respect to operator precedence, associativity and allowed combinations. (Note that this is a succinct description that doesn't deal with grammar issues such as conflicts.)

::

	atomic:
		'(' value ')'
		constant
		variable access
		function/method call
	
	unary:
		atomic
		'move' unary
		'+' unary
		'-' unary
		'!' unary
		'&' unary
		'*' unary
	
	// 'multiply' name used for *, / and %
	multiply:
		unary
		multiply '*' unary
		multiply '/' unary
		multiply '%' unary
	
	// 'add' name used for + and -
	add:
		multiply
		add '+' multiply
		add '-' multiply
	
	shift:
		unary '<<' unary
		unary '>>' unary
	
	comparison:
		// Use a unary value as the next level down to avoid conditions
		// like 'a + b' or 'a * b'.
		unary
		add '==' add
		add '!=' add
		add '<' add
		add '>' add
		add '<=' add
		add '>=' add
	
	bitwise_and:
		unary
		bitwise_and '&' unary
	
	bitwise_or:
		unary
		bitwise_or '|' unary
	
	logical_and_short_circuit:
		comparison
		logical_and_short_circuit '&&' comparison
	
	logical_or_short_circuit:
		comparison
		logical_or_short_circuit '||' comparison
	
	logical_and:
		comparison
		logical_and 'and' comparison
	
	logical_or:
		comparison
		logical_or 'or' comparison
	
	ternary_result:
		atomic
		unary
		multiply
		add
		shift
		bitwise_and
		bitwise_or
	
	ternary:
		comparison '?' ternary_result : ternary_result
	
	value:
		atomic
		unary
		multiply
		add
		comparison
		shift
		bitwise_and
		bitwise_or
		logical_and
		logical_or
		logical_and_short_circuit
		logical_or_short_circuit
		ternary
	
