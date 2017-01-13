Variants
========

Variants hold exactly one of a set of fixed types.

.. code-block:: c++

	datatype Constant(int value);
	datatype Add(Value& left, Value& right);
	datatype Multiply(Value& left, Value& right);
	
	variant Value = Constant | Add | Multiply;

At any time an object of type ``Value`` will contain exactly one of ``Constant``, ``Add`` or ``Multiply``.

Datatype Short Syntax
---------------------

A syntactic special case of  ``variant`` is ``datatype <name> = ...``:

.. code-block:: c++

	datatype Value =
		Constant(int value) |
		Add(Value& left, Value& right) |
		Multiply(Value& left, Value& right);

This is identical to the code shown above; this more concise form is desirable unless the ``variant`` should contain non-``datatype`` values (e.g. a ``class``):

.. code-block:: c++

	class Constant { }
	class Add { }
	class Multiply { }
	
	variant Value = Constant | Add | Multiply;

Extracting elements
-------------------

Elements of a ``variant`` can be extracted using ``if`` or ``switch``.

Switch
~~~~~~

.. code-block:: c++
	
	int compute(Value value) {
		switch (value) {
			case Constant(int constant) {
				return constant;
			}
			case Add(Value& left, Value& right) {
				return compute(left) + compute(right);
			}
			case Multiply(Value& left, Value& right) {
				return compute(left) * compute(right);
			}
		}
	}

If
~~

.. Note::
	Not yet implemented.

.. code-block:: c++

	int getConstantOrZero(Value value) {
		if (Constant constant = value) {
			return constant.value;
		} else {
			return 0;
		}
	}

Method Calls
------------

.. Note::
	Not yet implemented.

If there are common methods across all of the ``variant`` sub-types then these can be called via the ``variant``:

.. code-block:: c++

	int Constant::depth() const {
		return 0;
	}
	
	int Add::depth() const {
		return max(@left.depth(), @right.depth());
	}
	
	int Multiply::depth() const {
		return max(@left.depth(), @right.depth());
	}
	
	int depth(const Value& value) {
		return value.depth();
	}

Templates
~~~~~~~~~

A ``variant`` can contain a templated type:

.. code-block:: c++

	template <typename T>
	variant optional = T | None;
	
	import optional<Value> parse();
