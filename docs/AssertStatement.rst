Assert Statement
================

Rather than providing a macro or function for *assert*, Loci adds it straight into the language as a statement:

.. code-block:: c++

	void function() {
		int value = 2;
		assert value > 1;
		assert value < 3;
		assert (value * 2) == 4;
	}

The reasons for this choice include:

* Compiler options can determine whether asserts are enabled/disabled.
* No warnings for 'unused' variable in Release builds, since the compiler is aware of them.
* While it's bad practice, any side effects in *assert* statements will be performed correctly in all builds.

Developers are encouraged to write assertions to express program invariants/expectations, both to ensure bugs are caught early and to make the code easier to understand.

So in a build with asserts enabled (which should be most of the time), the compiler adds checks for these conditions and traps if they fail. If the assert expression can be proved to always be true by optimisations, they would be expected to eliminate the check/trap entirely.

If assertions have been disabled, the expressions will still be evaluated (further options are likely to be available in future to disable this in extreme cases) but they are used as hints to optimisers that the condition must be true. Unless the expressions do have side effects, they are likely to be eliminated entirely by optimisations.

Unreachable Statement
---------------------

A similar statement is *unreachable*, which expresses that a code path must never be executed. Here's an example:

.. code-block:: c++

	void function() {
		int value = 2;
		if (value == 2) {
			return;
		} else {
			unreachable;
		}
	}

(Developers may note the inspiration from :doc:`LLVM <LLVMIntro>`.)

Again, builds with asserts enabled will generate code that checks the *unreachable* is observed, while builds with asserts disabled would use it as an optimiser hint.

