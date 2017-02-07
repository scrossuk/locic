Proposal: Style Rules
=====================

.. Note::
	Feature awaiting further design consideration.

This is a proposal to add style rules into Loci.

Rationale
---------

The language is easier to use if there is maximum consistency. Fixed naming rules also eliminate the need for method name canonicalisation.

Warnings
--------

Violations of the style rules would produce warnings, so that developers can quickly prototype code (e.g. when debugging) without having to fix the violations.

Naming
------

* Variable names should use ``[a-z][a-zA-z0-9]+``: ``thisIsAVariable0``
* Type names should use ``[A-Z][a-zA-z0-9]+``: ``ThisIsAType1``
* Method names and properties should use ``[a-z][a-zA-z0-9]+``: ``thisIsAMethodOrProperty2``
* API names should use ``[a-z][a-z0-9]+``: ``apiname3::apiname4``

Indentation
-----------

The style rules would require a consistent indentation throughout each file. Valid indentation would include:

* 2 spaces
* 4 spaces
* 8 spaces
* A tab

Spaces would always be used for alignment.

Line length
-----------

Lines would be capped at 80 characters. Specific rules would apply for wrapping.

Wrapping arguments
~~~~~~~~~~~~~~~~~~

Functions without wrapped arguments would have the curly brace on the end of the line:

.. code-block:: c++

	void function(int a, int b, int c) {
		...
	}

Wrapping would put the curly brace on the line after the arguments:

.. code-block:: c++

	void function(int a, int b, int c, int d, int e, int f, int g, int h,
	              int i, int j)
	{
		...
	}

This prevents confusion between the aligned arguments and the indentation of the code in the function.

Similar rules would apply for classes:

.. code-block:: c++

	class Example(int a, int b, int c, int d, int e, int f, int g, int h,
	              int i, int j)
	{
		...
	}

Whitespace
----------

Unary operators
~~~~~~~~~~~~~~~

Unnamed unary operators would have no space between them and their operand:

.. code-block:: c++

	+value
	&value
	*value

Named unary operators would have one space between them and their operand:

.. code-block:: c++

	move value
	new value
	alignof Type
	sizeof Type

Binary operators
~~~~~~~~~~~~~~~~

Binary operators would have one space between them and each of their operands:

.. code-block:: c++

	value + value
	value * value
	value & value
	value < value
	value > value
	

Call operators
~~~~~~~~~~~~~~

Call operators would have no spaces:

.. code-block:: c++

	value(...)
	value[...]

Lists
~~~~~

Comma-separated lists would have one space after each comma, except for tuples of one element, which would have no space after the comma:

.. code-block:: c++

	()
	(a,)
	(a, b)
	(a, b, c)

Types
~~~~~

Types would have no spaces, except for the spaces between template arguments:

.. code-block:: c++

	Type&
	Type*
	Type[]
	Type[10]
	Type<Arg0>
	Type<Arg0, Arg1>

Variables
~~~~~~~~~

Variables would have a single space between their type and the variable name:

.. code-block:: c++

	Type name

Initialisation/Assignment
~~~~~~~~~~~~~~~~~~~~~~~~~

Initialisation and assignment would both have a space each side of the ``=``:

.. code-block:: c++

	Type name = value
	lvalue = value

If conditionals
~~~~~~~~~~~~~~~

``if`` conditionals would have:

* One space between ``if`` and the opening parenthesis.
* No spaces between the expression and the parentheses.
* One space between the closing parenthesis and the opening curly brace.

.. code-block:: c++

	if (expr) {
		...
	}

For loops
~~~~~~~~~

For loops would have:

* One space between ``for`` and the opening parenthesis.
* No spaces between the variable and the colon.
* One space between the colon and the range expression.
* One space between the closing parenthesis and the opening curly brace.

.. code-block:: c++

	for (auto x: ...) {
		...
	}

While loops
~~~~~~~~~~~

While loops would have:

* One space between ``while`` and the opening parenthesis.
* No spaces between the expression and the parentheses.
* One space between the closing parenthesis and the opening curly brace.

.. code-block:: c++

	while (expr) {
		...
	}
