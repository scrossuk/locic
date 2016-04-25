Operator Overloading
====================

Operator overloading allows a type to specify functionality for particular language operators (e.g. to implement ``a + b``) and is a useful way to provide a clean syntax for manipulating standard library or user defined types such as strings, vectors, matrices and arrays. This is used by Loci's primitive types (e.g. ``int``) since :doc:`they are objects <PrimitiveObjects>` and developers can implement operators for their own custom types.

Each operator has its own method (e.g. ``a + b`` is implemented by the ``add`` method) which implements the behaviour of the operator. For example:

.. code-block:: c++

	class CustomType {
		compare_result_t compare(const T& other) const;
		CustomType add(const CustomType& other) const;
		CustomType substract(const CustomType& other) const;
	}
	
	void function(const CustomType& a, const CustomType& b) {
		// All following pairs are equivalent...
		bool compare_result = a.compare(b).is_less_than();
		bool compare_result = a < b;
		
		T add_result = a.add(b);
		T add_result = a + b;
		
		T subtract_result = a.subtract(b);
		T subtract_result = a - b;
	}

Operator Overloading is purely syntactic sugar for method calls, so there are no implications for code generation.

Methods
-------

The following is a list of the operator methods.

Unary Operators
~~~~~~~~~~~~~~~

* ``*p``: ``deref``
* ``+a``: ``plus``
* ``-a``: ``minus``
* ``!a``: ``not``
* ``++a`` or ``a++``: ``increment``
* ``--a`` or ``a--``: ``decrement``

Binary Operators
~~~~~~~~~~~~~~~~

* ``a + b``: ``add``
* ``a - b``: ``subtract``
* ``a * b``: ``multiply``
* ``a / b``: ``divide``
* ``a % b``: ``modulo``
* ``a & b``: ``bitwiseand``
* ``a | b``: ``bitwiseor``
* ``a ^ b``: ``bitwisexor``
* ``a == b``: ``equal`` (or ``compare``)
* ``a != b``: ``notequal`` (or ``compare``)
* ``a < b``: ``lessthan`` (or ``compare``)
* ``a <= b``: ``lessthanorequal`` (or ``compare``)
* ``a > b``: ``greaterthanorequal`` (or ``compare``)
* ``a >= b``: ``greaterthanorequal`` (or ``compare``)

Argument List Operators
~~~~~~~~~~~~~~~~~~~~~~~

* ``f(a, b, ...)``: ``call``
* ``m[i]``: ``index``

Lvalue Operators
~~~~~~~~~~~~~~~~

For types marked `lval <LvaluesAndRvalues>`:

* ``&m``: ``address``
* ``m = v``: ``assign``
* ``move m``: ``move``

.. _compare_methods:

Compare Method
~~~~~~~~~~~~~~

Of particular interest is the *compare* method, which is an alternative to defining each of the comparison operators individually. Instead, developers just implement this single method which uses a *compare_result_t* primitive value to indicate whether the left value is less than, equal to, or greater than the right value.

This functionality can avoid double comparisons (i.e. two applications of the less-than operator with left and right values swapped) inside container types (and other relevant functions/types). This is particularly beneficial for objects that may be expensive to compare, such as strings and memory blocks, but for which a single comparison can identify whether a value is less than, equal to, or greater than another value.

Operator Combinations
---------------------

Particular combinations of operators can often lead to confusion. For example, consider the expression ``a << b & c``; in this case the shift has higher precedence so the equivalent bracketed expression is ``(a << b) & c``, but many developers might think it could be ``a << (b & c)``. There are many cases that appear in practice in code and can confuse developers.

Loci follows the same operator precedence rules as C and C++ but the compiler issues warnings for potentially confusing operator combinations. This means that the expression ``a << b & c`` results in a warning from the compiler suggesting that the developer should specify either ``(a << b) & c`` or ``a << (b & c)`` and hence avoid writing an expression which is confusing, or worse yet not what they had intended. Some combinations *are* well understood, such as ``a * b + c``, so the compiler does not issue warnings for these.
