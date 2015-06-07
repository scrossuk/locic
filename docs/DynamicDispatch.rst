Dynamic Dispatch
================

As described in :doc:`Vtable Generation <VtableGeneration>` Class vtables are only generated when a class reference is cast to an interface type reference, and compilers must generate a vtable for each such cast, meaning that there may be multiple vtables per class, and a vtable will often not contain pointers to all the methods of the class. Vtable pointers are not stored within an object, but are carried around as part of an interface type reference, and this means an interface type reference [1]_ is actually two pointers: one to the object, and one to the vtable (as well as a :doc:`Template Generator <TemplateGenerators>`).

A class vtable contains around (to be decided!) 20 function pointers to its methods, in addition to pointers to special methods such as the class destructor. The compiler will create an MD5 hash for each mangled method name from the target interface type, and truncate that hash to the first 16 hex digits (i.e. first 16 4-bit sequences), giving a 64 bit value. It's important that these values do not clash, and compilers must check this for each interface type.

These hashes are then calculated modulo the vtable size (number of function pointers), to give offsets into the vtable. Unlike the original 64-bit hashes, these offsets can clash and are resolved by conflict resolution stubs.

Calling a method
----------------

Code generated to perform virtual method calls then simply needs to do the following:

#. Obtain the vtable pointer.
#. Load the method's function pointer from its offset in the vtable.
#. Set the hidden parameter register (platform dependent; see below) to the 64-bit hash value.
#. Call the function pointer with its arguments.

The hidden parameter register selected for a platform will typically be the same as the static chain register used. Hence the registers used are:

* x86: ecx
* x86_64: r10
* ARM (32-bit): r12
* AArch64 (Linux): x18
* (others TBD)

On 32-bit platforms a pointer is passed to global data storing the 64-bit hash value. On 64-bit platforms the hash value is passed directly in the register.

Conflict Resolution
-------------------

Clearly there may be conflicts in the hash table, in which multiple methods share the same slot. In this case, the compiler must generate a conflict resolution stub, which is an assembly routine that uses the hidden parameter to determine which method is intended to be called. A stub may look like the following:

::

	conflict_resolution_stub:
		cmp r10, <METHOD 1 HASH>
		jne .method_2
	.method_1:
		jmp method_1
	.method_2:
		jmp method_2

A conflict resolution stub must not affect the stack or any of the argument registers or callee-save registers (it can push data onto the stack, as long as it pops it off before the jump). It should pass control to the method, which will then return directly to the original caller.

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
		
		__builtin_call_with_static_chain(methodPtr(interfaceRef.object, arg), HASH_VALUE);
	}

Performance
-----------

The implementation described has many advantages versus competing approaches, such as provided by C++'s virtual tables. It provides the semantic behaviour of :doc:`Structural Typing <StructuralTyping>`, but also minimises the overhead of a virtual method call to be negligible compared to other approaches and languages.

As mentioned in :doc:`Vtable Generation <VtableGeneration>`, the vtable is generated for a class when it is cast to an interface type, and that the vtable only includes the methods of the interface. Since the implementation uses a hash table, this is likely to significantly reduce the number of collisions in most cases, which will usually mean eliminating conflict resolution stubs, or in some cases simply shortening them. This therefore reduces the indirection, comparisons and branching required so that in the vast majority of cases setting the hidden parameter is the only overhead.

If a collision does occur, the conflict resolution stub can be structured efficiently to keep the overhead very minimal. For example, conflict resolution stubs could be generated to perform a binary search on the hash value; another option is to shortcut cases for methods that are expected to be called more often.

This design also differs from C++ in that vtable pointers are not stored in objects, but are part of an interface type reference. This decision is particularly appropriate to the language, since Loci doesn't require classes to explicitly implement interfaces (see :doc:`Structural Typing <StructuralTyping>`), and would otherwise therefore need vtable pointers in every object in case the object is used polymorphically.

There's also the potentially much greater benefit that a memory access can be avoided, assuming the vtable pointer can be held in a register, or that at least a heap memory access to the object to retrieve the vtable pointer is likely to be replaced with a stack memory access, which may reduce the chances of a cache miss. The drawback is that an interface type reference is at least two pointers in size (also contains a :doc:`Template Generator <TemplateGenerators>`), but this is a very minor overhead and only affects polymorphic references.

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

As explained above, the compiler computes a 64-bit hash value for each method in an interface. If methods end up in the same hash table slot that's OK, since they can be resolved by a conflict resolution stub.

However if there is a collision between the 64-bit hash values themselves, the compiler **cannot** generate a conflict resolution stub to disambiguate the methods and therefore the compiler must inform the user via an error that the interface is invalid. This must occur extremely rarely, since this means an implementation detail affecting a high level design decision.

Again, the calculation of the probability of a collision between method hashes within an interface is:

.. math::

	p_{c}(n) = 1 - (\frac{2^{64} - 1}{2^{64}})^\frac{n(n - 1)}{2}

Which gives:

.. math::

	p_{c}(0) &= 0  \\
	p_{c}(1) &= 0  \\
	p_{c}(2) &= 5.42 * 10^{-20}  \\
	p_{c}(4) &= 3.25 * 10^{-19}  \\
	p_{c}(8) &= 1.52 * 10^{-18}  \\
	p_{c}(16) &= 6.51 * 10^{-18} \\
	p_{c}(32) &= 2.69 * 10^{-17} \\
	p_{c}(64) &= 1.09 * 10^{-16} \\

Let's go for an extremely conservative estimate of 64 methods per interface. We can then calculate :math:`m`, the expected number of **unique** interfaces until the probability of collision exceeds 0.01%.

As above, let :math:`p_{u}(n)`, the probability of no collisions for an interface containing :math:`n` methods, be defined as:

.. math::

	p_{u}(n) = 1 - p_{c}(n)

Hence:

.. math::

	p_{u}(64)^m      &= 1 - 0.0001  \\
	p_{u}(64)^m      &= 0.9999      \\
	ln (p_{u}(64)^m) &= ln 0.9999   \\
	m ln p_{u}(64)   &= ln 0.9999   \\
	m                &= \frac{ln 0.9999}{ln {p_{u}(64)}}

This evaluates to:

.. math::

	m = 9.15 * 10^{11}

In other words, the probability of at least one hash value collision is 0.01% for around half a trillion **unique** interfaces generated.

Or to put it another way, assuming there are about 500,000 developers in the world, and for some reason they all start writing unique Loci interfaces, each developer would need to write 1.83 million interfaces to reach the 0.01% probability point; this amounts to approximately 60 unique interfaces per day over a typical human lifetime.

From a practical standpoint this means that a hash collision is extremely improbable. If it did ever occur then the compiler would be able to detect this and issue an error, and the user would be able to modify the interface.

Compiler Implementation
-----------------------

(Implementation is in progress.)

The mechanism described above is implemented in :doc:`LLVM <LLVMIntro>` using a combination of:

* The 'nest' attribute - for passing the hidden parameter.
* 'mustcall' - to enforce tail calls in the conflict resolution stub.
* varargs - for representing that there are an unknown number of arguments that must not be affected by the stub.

For example, here's some typical LLVM IR:

.. code-block:: llvm

	define i32 @conflict_resolution_stub(i8* nest %id_ptr, ...) {
		%method_id = ptrtoint i8* %id_ptr to i64
		%is_f = icmp eq i64 %method_id, 42
		br i1 %is_f, label %CallFirstMethod, label %CallSecondMethod
	CallFirstMethod:
		%f0_cast = bitcast i32 (i8*, i32, i32)* @firstMethod to i32 (i8*, ...)*
		%f0_result = musttail call i32 (i8*, ...) %f0_cast(i8* nest %id_ptr, ...)
		ret i32 %f0_result
	CallSecondMethod:
		%f1_cast = bitcast i32 (i8*, i8, i8*)* @secondMethod to i32 (i8*, ...)*
		%f1_result = musttail call i32 (i8*, ...) %f1_cast(i8* nest %id_ptr, ...)
		ret i32 %f1_result
	}

External Links
--------------

This idea is also described in a paper titled `Efficient Implementation of Java Interfaces: Invokeinterface Considered Harmless`_.

* `LLVM mailing list discussion on using nest attribute`_
* `GCC mailing list discussion on ARM static chain registers`_

.. _`Efficient Implementation of Java Interfaces: Invokeinterface Considered Harmless`: https://www.research.ibm.com/people/d/dgrove/papers/oopsla01.pdf
.. _`LLVM mailing list discussion on using nest attribute`: http://comments.gmane.org/gmane.comp.compilers.llvm.devel/86370
.. _`GCC mailing list discussion on ARM static chain registers`: http://www.mail-archive.com/gcc@gcc.gnu.org/msg76927.html

.. [1] Pointers do not support polymorphism, and an interface type pointer is illegal, hence dynamic dispatch is only relevant to references.