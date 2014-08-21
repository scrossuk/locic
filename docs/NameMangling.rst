Name Mangling
=============

While Loci was designed without function or method overloading, limited name mangling is still required for some features, and also in general to try to avoid conflicts between normal C function names and Loci generated functions, such as :doc:`Class <Classes>` methods.

C-like Functions
----------------

Here's an example:

.. code-block:: c++

	export void function() { }

In this case, the mangling is essentially a no-op, so function() is mangled to function(). This behaviour applies to all non-templated functions in the global namespace in an unnamed export/import (see :doc:`Modules <Modules>` for more information). These functions are therefore callable to/from C.

Class Method
------------

Here's an example of a :doc:`Class <Classes>` method:

.. code-block:: c++

	class ExampleClass() {
		void method() { }
	}

Mangling will transform this to:

::

	MT1N12ExampleClassF1N6method

This essentially breaks down into:

::

	Method(type: TypeName([ "ExampleClass" ]), function: FunctionName([ "method" ]))

Namespace Class Method
----------------------

Here's a case with a namespace:

.. code-block:: c++

	namespace Namespace {
		class ExampleClass() {
			void method() { }
		}
	}

The method name now mangles to:

::

	MT2N9NamespaceN12ExampleClassF1N6method

This breaks down into:

::

	Method(type: TypeName([ "Namespace", "ExampleClass" ]), function: FunctionName([ "method" ]))

Named Module Function
---------------------

Here's a function inside a :doc:`Named Module <Modules>`:

.. code-block:: c++

	export TestModule 1.2.3 {
		void function() { }
	}

This mangles to:

::

	P1N9TestModuleV1_2_3_F1N8function

This breaks down into:

::

	Module(name: [ "TestModule" ], version: 1.2.3), FunctionName([ "function" ])

