Template Generators
===================

As explained in :doc:`Templates <Templates>`, Loci supports template declarations across module API boundaries, such as:

.. code-block:: c++

	// Module 'A'.
	export A 1.0.0 {
		
		template <typename T>
		class ExampleClass(T value) {
			static create = default;
		}
		
	}

.. code-block:: c++

	// Module 'B'.
	import A 1.0.0 {
		
		template <typename T>
		class ExampleClass {
			static ExampleClass<T> create(T value);
		}
		
	}

This functionality is not provided in C++ due to restrictions about how templates are instantiated, which lead to these consequences:

* Templated functions or types must have their implementation exposed to clients, essentially making them unsuitable for stable APIs.
* Client code must be recompiled when the templated function/type implementation changes.
* A large compile-time overhead for repeatedly processing and instantiating template implementations.
* Different code is generated for each template instantiation, which can lead to code bloat.

C++'s export keyword
--------------------

C++ added an *export* keyword to try to fix this problem and decrease the coupling between a template use and an implementation. The idea is that developers can indicate to the compiler that a templated function or type is provided by another source file via an exported declaration, such as:

.. code-block:: c++

	// This is C++ code!
	export template <typename T>
	void function();
	
	void example() {
		function<int>();
	}

The compiler is therefore responsible for working with the linker to ensure that the templates are instantiated when the source file containing the declaration and the source file containing the implementation are linked together.

This feature was ultimately removed in C++11, since the C++ compiler still needs access to the definition to instantiate the templates and therefore exported templates tend to be as slow or even slower to compile than normal inline templates.

Loci Templates
--------------

Loci takes an entirely different approach to templates, recognising that the only way to allow separate uses and implementations is for the code of each to be compiled separately, such that it can be linked with a standard linker. This means that there cannot be further intervention by the compiler during linking and hence the compilation model is identical to normal non-templated functions and classes.

The implication of this decision is that templates are instantiated at **run-time**, not at **compile-time**. Special functions called 'template generators' are produced by the compiler for each templated construct, and are split into two variants:

* *Root template generators* - Generator functions where all the template arguments are statically known.
* *Intermediate template generators* - Generator functions where one or more template arguments are dependent on other template arguments.

Together these form a **Directed Acyclic Graph**.

Generator Example
~~~~~~~~~~~~~~~~~

Consider this fairly simple example:

.. code-block:: c++

	void f() {
		g<int>();
		h<float>();
	}
	
	template <typename T>
	void g() {
		i<T, byte>();
		j<T, short>();
	}
	
	template <typename T>
	void h() { }
	
	template <typename S, typename T>
	void i() { }
	
	template <typename S, typename T>
	void j() { }

This code generates the following template generator graph:

::

	f: ROOT
		-> g: INTERMEDIATE (int)
			-> i: INTERMEDIATE (T, byte)
			-> j: INTERMEDIATE (T, short)
	f: ROOT
		-> h: INTERMEDIATE (float)

Here's an example for how the code of the first root template generator might look:

.. code-block:: c++

	Type[8] ROOT_0(uint32_t path) {
		assert(path >= 1);
		
		Types[8] types;
		types[0] = { VTABLE_int, NULL, 0 };
		
		if (path == 1) {
			return types;
		} else {
			return TPLGEN_g(types, ROOT_0, path, 31 - ctlz(path));
		}
	}

And here's an example for the intermediate template generator for g():

.. code-block:: c++

	Type[8] TPLGEN_g(Type[8] types, void* rootFn, uint32_t path, uint8_t parentPosition) {
		const auto position = parentPosition - 1;
		const auto subPath = (path >> position);
		const auto mask = 0x3;
		const auto component = (subPath & mask);
		
		Type[8] newTypes;
		
		if (component == 0) {
			newTypes[0] = types[0];
			newTypes[1] = { VTABLE_byte, NULL, 0 };
			if (position == 0) return newTypes;
			return TPLGEN_i(newTypes, rootFn, path, position);
		} else if (component == 1) {
			newTypes[0] = types[0];
			newTypes[1] = { VTABLE_short, NULL, 0 };
			if (position == 0) return newTypes;
			return TPLGEN_j(newTypes, rootFn, path, position);
		} else {
			// Unreachable!
		}
	}

And finally, here's how the template generator for i() might look:

.. code-block:: c++

	Type[8] TPLGEN_i(Type[8] types, void* rootFn, uint32_t path, uint8_t parentPosition) {
		// Unreachable!
	}

So the purpose of the template generator functions is to return an array of template argument values for any templated function or type. They achieve this by starting at the *root template generator*, where all the initial template argument values are known, and then using a 32-bit unsigned integer that specifies the path from the *root template generator* to the relevant *intermediate template generator*, at which point the functions will exit and the correct template argument values are returned.

Top Down Calls
~~~~~~~~~~~~~~

This design is probably the reverse of what most developers expect, since it's natural to think of a function accessing its own template arguments or parameters first, and then performing further operations to access template arguments in outer contexts (i.e. 'bottom up'). However in this case template generators always call down from the root template generator until they reach the relevant intermediate generator (i.e. 'top down').

To understand this design, consider:

.. code-block:: c++

	void f() {
		h<int>();
	}
	
	void g() {
		h<float>();
	}
	
	template <typename T>
	void h() { }

Here's the corresponding graph:

::

	f: ROOT -----
	            |
	g: ROOT --  |
	         |  |
	         |  |
	         -----> h: INTERMEDIATE(int|float)

So there are two separate root template generators that refer to the same intermediate template generator. Hence if we wanted to distinguish which root template generator is being used, to determine whether the template argument is *int* or *float*, we'd need to distinguish between these two cases.

The problem is of course that, unlike the top-down path, this path can't be constructed at compile-time. The compiler can distinguish between the various template uses that are made within a function or type since they're locally visible, however it can't distinguish between uses of that function or type at any point in any dependent source code, since this is clearly not visible.

Optimisation
~~~~~~~~~~~~

Template Generators have been carefully designed to facilitate optimisation, such that a standard optimiser (such as LLVM's *opt*) can eliminate the template generators by inlining and hence automatically instantiate the templates in a very similar way to C++.

For example, in the above example a good optimiser would:

* Resolve the type arrays returned by each call to template generators as a simple constant.
* Use this information to convert dynamic dispatch for methods such as *sizeof()* (special method to determine require storage capacity for an object) into constants.
* Use the now-known size of objects to move them from the stack into registers.

These steps transform the relatively high overhead of dynamic dispatch for template parameters (and to a lesser extent template generators) into a set of native instructions that perform exactly the desired operations. Hence this closely mimics the behaviour of a C++ compiler when both the template use and the templated implementation are visible.

**What if these optimisations aren't possible?**

Well, firstly, make sure you're using link-time optimisation if you can, since you'll be able to link entirely separate modules together with the same benefits as described above (as previously mentioned, the design means the compiler has produced truly-separate assembly code for each module, so normal rules apply).

If you *definitely* can't link two modules together that communicate via a templated API, then you're breaking new ground; C++ doesn't support this use case at all!

As with all cases of API boundaries, you'll likely have to accept a certain amount of overhead due to more generalised ABI compliance and a lack of interprocedural knowledge. It's therefore a trade-off, though in practice this is likely to **not** be a problem (after all, C++ developers have managed without this feature entirely).

