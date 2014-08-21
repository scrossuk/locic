Dynamic Dispatch
================

As described in :doc:`Vtable Generation <VtableGeneration>` Class vtables are only generated when a class reference is cast to an interface type reference, and compilers must generate a vtable for each such cast, meaning that there may be multiple vtables per class, and a vtable will often not contain pointers to all the methods of the class. Vtable pointers are not stored within an object, but are carried around as part of an interface type reference, and this means an interface type reference (pointers do not support polymorphism, and an interface type pointer is illegal, hence this only affects references) is actually two pointers: one to the object, and one to the vtable (as well as a :doc:`Template Generator <TemplateGenerators>`).

A class vtable contains around (to be decided!) 20 function pointers to its methods, in addition to pointers to special methods such as the class destructor. The compiler will create an MD5 hash for each mangled method name from the target interface type, and truncate that hash to the first 8 hex digits (i.e. first 8 4-bit sequences), giving a 32 bit value. It's important that these values do not clash, and compilers must check this for each interface type.

These hashes are then calculated modulo the vtable size (number of function pointers), to give offsets into the vtable. Unlike the original 32-bit hashes, these offsets can clash and are resolved by conflict resolution stubs.

Calling a method
----------------

Code generated to perform virtual method calls then simply needs to do the following:

#. Obtain the vtable pointer.
#. Load the method's function pointer from its offset in the vtable.
#. Set the hidden parameter (platform dependent, but usually a 32 bit register, such as *eax* on x86, should be used) to the 32-bit hash value.
#. Call the function pointer with its arguments.

Conflict Resolution
-------------------

Clearly there may be conflicts in the vtable, in which multiple methods share the same offset. In this case, the compiler must generate a conflict resolution stub, which is an assembly routine that uses the hidden parameter to determine which method is intended to be called. A stub may look like the following:

::

	conflict_resolution_stub:
		cmp eax, <METHOD 1 HASH>
		jne .method_2
	.method_1:
		jmp method_1
	.method_2:
		jmp method_2

A conflict resolution stub must not affect the stack or any of the callee-save registers. It should pass control to the method, which will then return directly to the original caller.

Code example
------------

The virtual method call process described roughly corresponds to the following C code:

.. code-block:: c

	struct VTable {
		void* methodPointers[20];
	};
	
	struct ObjectType;
	
	struct InterfaceRef {
		VTable* vtable;
		ObjectType* object;
	};
	
	// E.g. method takes one 'int' parameter and returns 'void'.
	typedef void (*ExampleMethod)(ObjectType*, int);
	
	void callFunc(InterfaceRef interfaceRef, int arg) {
		VTable* vtable = interfaceRef.vtable;	
		
		// Get method pointer.
		// E.g. '2' is the offset in this case.
		ExampleMethod methodPtr = (ExampleMethod) vtable->methodPointers[2];
		
		// E.g. '42424242' is the hash in this case.
		asm volatile ("movl $42424242, %%eax"
			:
			:
			: "%eax");
		
		methodPtr(interfaceRef.object, arg);
	}

Performance
-----------

The implementation described has many advantages versus competing approaches, such as provided by C++'s virtual tables. It provides the semantic behaviour described previously in this document, but also minimises the overhead of a virtual method call to be negligible compared to other approaches and languages.

As mentioned in :doc:`Vtable Generation <VtableGeneration>`, the vtable is generated for a class when it is cast to an interface type, and that the vtable only includes the methods of the interface. Since the implementation uses a hash table, this is likely to significantly reduce the number of collisions in most cases, which will usually mean eliminating conflict resolution stubs, or in some cases simply shortening them. This therefore reduces the indirection, comparisons and branching required so that in the vast majority of cases setting the hidden parameter is the only overhead.

This design also differs from C++ in that vtable pointers are not stored in objects, but are part of an interface type reference. This decision is particularly appropriate to the language, since Loci doesn't require classes to explicitly implement interfaces (see :doc:`Structural Typing <StructuralTyping>`), and would otherwise therefore need vtable pointers in every object in case the object is used polymorphically.

There's also the potentially much greater benefit that a memory access can be avoided, assuming the vtable pointer can be held in a register, or that at least a heap memory access to the object to retrieve the vtable pointer is likely to be replaced with a stack memory access, which may reduce the chances of a cache miss. The drawback is that an interface type reference is at least two pointers in size (also contains a :doc:`Template Generator <TemplateGenerator>`), but this is a very minor overhead and only affects polymorphic references.

On modern machines, the call/jump instruction is the most expensive operation, generally due to the need to flush the instruction pipeline. This is of course unavoidable in any dynamic dispatch mechanism (i.e. where optimisations fail to convert calls to static dispatch), so applies to typical dynamic dispatch mechanisms in other languages.

Birthday Problem
~~~~~~~~~~~~~~~~

Assessing the value of this dynamic dispatch scheme is best achieved by relating it to the birthday problem.

If there is a set of :math:`n` individuals, with birthdays that are distributed with an independent and uniform probabability through a year of 365 days, the probability :math:`P_{u}(n)` that they all have unique birthdays is, approximately:

.. math::

	P_{u}(n) = (\frac{364}{365})^{\frac{n(n - 1)}{2}}

So, the probability :math:`P_{c}(n)` that there is at least one birthday clash is :math:`1 - P_{u}(n)`:

.. math::

	P_{c}(n) = 1 - \frac{364}{365}^{\frac{n(n - 1)}{2}}

The essence of this `problem' is that relatively small values of :math:`n` can have a surprisingly high probability :math:`P_{c}(n)` of clashes. Relating this to the hash table dynamic dispatch, performance is improved by minimising clashes between the indexes of the vtable function pointers in the tables.

Hash Table Collisions
~~~~~~~~~~~~~~~~~~~~~

Using the above, the probability of collision :math:`p_{c}(n)` for an interface containing :math:`n` methods (with hash table size of 20) is:

.. math::

	p_{c}(n) = 1 - (\frac{19}{20})^\frac{n(n - 1)}{2}

This gives the following:

* 0: 0%
* 1: 0%
* 2: 5%
* 3: 14.3%
* 4: 26.5%
* 5: 40.1%
* 6: 53.7%
* 7: 65.9%
* 8: 76.2%

Estimates of likely interface sizes depend on the code and the way it's written. However, a reasonable guideline (and one that suggests good design) is to generally have at most 4 methods per interface, so collisions are expected to occur for only about a quarter of hash tables generated. A conservative estimate would therefore suggest that in around 75% of cases this dispatch mechanism is almost equivalent to C++ virtual dispatch, the only difference being the need for the caller to set the hidden parameter.

Hash Value Collisions
---------------------

As explained above, the compiler computes a 32-bit hash value for each method in an interface. If methods end up in the same hash table slot that's OK, since they can be resolved by a conflict resolution stub.

However if there is a collision between the 32-bit hash values themselves, the compiler **cannot** generate a conflict resolution stub to disambiguate the methods and therefore the compiler must inform the user via an error that the interface is invalid. This must occur extremely rarely, since this means an implementation detail affecting a high level design decision.

Again, the calculation of probability is:

.. math::

	p_{c}(n) = 1 - (\frac{2^{32} - 1}{2^{32}})^\frac{n(n - 1)}{2}

Which gives:

* 0: 0%.
* 1: 0%.
* 2: 0.00000002%.
* 3: 0.00000007%.
* 4: 0.00000014%.
* 5: 0.00000023%.
* 6: 0.00000035%.
* 7: 0.00000049%.
* 8: 0.00000065%.

Given a presumed average of 4 methods per interface (again, a conservative estimate), we can calculate :math:`m`, the expected number of interfaces that have no hash value collisions before one does have a collision.

As above, let :math:`p_{u}(n)`, the probability of no collisions for an interface containing :math:`n` methods, be defined as:

.. math::

	p_{u}(n) = 1 - p_{c}(n)

Now evaluate :math:`m` such that there are sufficient interfaces for a 50% probability of a collision.

.. math::

	p_{u}(4)^m      = 0.5          \\
	ln (p_{u}(4)^m) = ln 0.5       \\
	m ln p_{u}(4)   = ln 0.5       \\
	m               = \frac{ln 0.5}{ln {p_{u}(4)}}

This evaluates to:

.. math::

	m = 4.95 x 10^8

In other words, the probability of at least one hash value collision is 50% for around half a billion interfaces generated.

