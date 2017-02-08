Proposal: Virtual Template Variables
====================================

.. Note::
	Feature awaiting further design consideration.

This is a proposal to add virtual template variables as a way of expressing properties of a function signature without having to pass the actual values.

Rationale
---------

Currently the following must be expressed with templates:

.. code-block:: c++

	template <bool IsConst>
	const<IsConst>(int)& function(const<IsConst>(int)* ptr) {
		return *ptr;
	}

This is problematic, particularly since interface methods cannot be templated. It also suggests that ``function()`` may or may not modify ``ptr``, which is incorrect; ``function()`` will never modify ``ptr``.

Changes
-------

Template variables would be marked ``virtual`` if they are only needed for the function signature:

.. code-block:: c++

	template <virtual bool IsConst>
	const<IsConst>(int)& function(const<IsConst>(int)* ptr) {
		return *ptr;
	}

``virtual`` template variables would be usable in interface methods and they would be omitted from code generation.

Determining whether function modifies const values
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The compiler can determine that a function does **not** modify a ``const`` argument by unifying the virtual template variable with ``true``:

.. code-block:: c++

	const<true>(int)& function(const<true>(int)* ptr) {
		return *ptr;
	}

This is a valid function signature (e.g. it doesn't ``require()`` that ``IsConst`` is ``false``). The template variable is ``virtual`` and therefore does **not** affect the logic inside the function. We can therefore be sure that function does **not** modify ``ptr``.

Relationship to selfconst
~~~~~~~~~~~~~~~~~~~~~~~~~

``selfconst`` is more convenient than virtual template variables because it is considerably less verbose, and is particularly usable with object properties:

.. code-block:: c++

	class Test {
		selfconst(int)& member;
	}

In future it may make sense to give ``selfconst`` the behaviour of adding an unnamed virtual template to the function:

.. code-block:: c++

	// This:
	selfconst(T)& method() const;

	// ...is treated as being this:
	template <virtual bool IsConst>
	const<IsConst>(T)& method() const<IsConst>;

This would work with template argument deduction. It's unclear, however, if this would actually simplify the compiler.
