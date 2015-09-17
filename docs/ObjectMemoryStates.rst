Object Memory States
====================

Loci defines the following:

* **Objects** - Values that can pass between multiple memory slots, occupying only one location at any point in time.
* **Memory Slots** - Positions in memory where objects can be 'resident' for some range of time.

Note that these definitions are similar, but not identical, to those of :doc:`Lvalues and Rvalues <LvaluesAndRvalues>`.

Object memory slots in Loci can be in one of the following states:

* Live state
* Dead state
* Invalid state

These states are needed to ensure that:

* Destructors are run exactly once for each object.
* Move methods are not invoked on memory locations without a resident object.

For example, consider:

.. code-block:: c++

	class ExampleClass(int value) {
		static create = default;
		
		~ {
			printf(C"Destroying; value was %d.", @value);
		}
	}
	
	import void bar(ExampleClass value);
	
	void foo() {
		auto exampleVariable = ExampleClass(42);
		bar(move exampleVariable);
	}

The variable ``exampleVariable`` starts in a live state but is placed into a dead state when we ``move`` it as part of the function call to ``bar``. Loci only invokes the user-specified destructor code of the ``ExampleClass`` instance once and therefore must ensure that the destructor is not invoked on ``exampleVariable``. This is achieved by setting it into a dead state during the ``move`` operation and checking for a dead state in the destructor.

Note that this is contrary to C++, which leaves this work to the developer; Loci does the work automatically by default but the behaviour can easily be customised as needed.

Live State
----------

A live state:

* Is a 'normal' state for the object, as might be returned from a constructor.
* Is valid for calling any methods.
* Will have any user-specified destructor invoked.
* Will have any user-specified ``__moveto`` method invoked.

An object memory slot is in a live state when it has a 'resident' object (i.e. an object is stored in it). In these cases the ``__islive`` method will return ``true``.

Dead State
----------

A dead state:

* Is an 'empty' slot value obtained after moving an object out of the slot.
* Is only defined for calling :doc:`Lifetime Methods <LifetimeMethods>`.
* Will **not** have any user-specified destructor invoked.
* Will **not** have any user-specified ``__moveto`` method invoked.

An object memory slot in a live state can be forced into a dead state by invoking ``__setdead``. **Note that this will NOT invoke the destructor.**

Objects are not required to have any dead states, and this means that ``__setdead`` is allowed to leave the object memory unchanged. This is useful for practical purposes because it allow objects that don't need distinct 'dead' states, such as where neither the object nor its member have a user-specified move method or destructor. For example:

.. code-block:: c++

	class ExampleClass(int value) {
		static create = default;
	}

This class simply doesn't need any indicator of liveness because it doesn't have a custom destructor or custom move method which would need to be handled.

Invalid State
-------------

An invalid state:

* Is a value of the memory occupied by the object which can never represent a usable state.
* Is only defined for calling :doc:`Lifetime Methods <LifetimeMethods>`.
* Will **not** have any user-specified destructor invoked.
* Will **not** have any user-specified ``__moveto`` method invoked.

Objects are not required to have any invalid states, however invalid states can be useful to allow an outer parent object (which has the object as a member) to use the object's invalid state to mark its own dead state. For example:

.. code-block:: c++

	class ExampleClass(int& value) {
		static create = default;
		
		~ {
			printf(C"Destroying; value was %d.", @value);
		}
	}

References have an invalid state; they can never be ``null``. This means that the compiler generates an ``__islive`` method for ``ExampleClass`` which simply queries whether the reference is in a valid state and generates ``__setdead`` to set the reference member into its invalid state. Hence this is allows us to avoid allocating extra space in the object (i.e. increasing its size in memory) to hold liveness information.
