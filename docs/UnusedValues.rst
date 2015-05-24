Unused Values
=============

Loci provides simple support for determining whether variables/values are used and marking the cases where they aren't used. For example:

.. code-block:: c++

	int function(int a, unused int b) {
		return a;
	}

The compiler will check whether the parameter variables are used; it can see that *a* is used but *b* is not used. Given that *a* is not marked unused but *b* is marked unused this is correct code.

By marking variables/values as unused and taking advantage of static analysis of the compiler, programmers can reduce the chances of introducing bugs into a code base; this is essentially a simple tool that the programmer can deploy to their benefit.

Detecting unused variables not marked
-------------------------------------

Consider the following code:

.. code-block:: c++

	int function(int a, int b) {
		return a + a;
	}

Here the compiler determines that *b* is not used, but the programmer has not marked this and so the compiler issues an error. This means the programmer is made aware about a potential problem and can choose to either:

* Add a use for the variable.
* Mark the variable as unused.

In this case the programmer may have intended to compute ``a + b``, hence the compiler error directs the programmer to fix the problem:

.. code-block:: c++

	int function(int a, int b) {
		return a + b;
	}

Detecting used variables marked unused
--------------------------------------

Now it's possible for a programmer to start out with code such as:

.. code-block:: c++

	int function(int a, unused int b) {
		return a;
	}

In this case the programmer has clearly indicated that not using variable *b* was intentional and so the compiler accepts this code. However a later programmer may modify the code:

.. code-block:: c++

	int function(int a, unused int b) {
		return a + b;
	}

The compiler will issue an error because *b* is marked as being unused but is actually used in the function code. Presumably the code is correct in this case (but in general new variable uses can be erroneous) and so the programmer can remove the *unused* marker.
