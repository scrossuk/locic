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

	void ROOT_0(Type* types, path_t path) {
		// Every path is terminated by a '1' bit (so that we can
		// calculate the length of the path from a single integer).
		assert(path >= 1);
		
		// Template parameters for 'g<int>()':
		//   * 'int': add vtable and null template generator.
		types[0] = { VTABLE_int, { NULL, 0 } };
		
		// Pass types to generator for 'g()'.
		TPLGEN_g(types, ROOT_0, path, PATH_BITS - 1 - ctlz(path));
		return;
	}

And here's an example for the intermediate template generator for g():

.. code-block:: c++

	void TPLGEN_g(Type* types, void* rootFn, path_t path, size_t parentPosition) {
		if (parentPosition == 0) {
			// End of path => return type array.
			return;
		}
		
		const auto position = parentPosition - 1;
		const auto subPath = (path >> position);
		const auto mask = 0x3;
		const auto component = (subPath & mask);
		
		switch (component) {
		case 0:
			// Template parameters for 'i<T, byte>()':
			//   * 'T': first argument of parent, so just copy it across.
			//   * 'byte': add vtable and null template generator.
			types[1] = { VTABLE_byte, { NULL, 0 } };
			
			// Still going => pass types to generator for 'i()'.
			TPLGEN_i(types, rootFn, path, position);
			return;
		case 1:
			// Template parameters for 'j<T, byte>()':
			//   * 'T': first argument of parent, so just copy it across.
			//   * 'short': add vtable and null template generator.
			types[1] = { VTABLE_short, { NULL, 0 } };
			
			// Still going => pass types to generator for 'j()'.
			TPLGEN_j(types, rootFn, path, position);
			return;
		default:
			// Unreachable!
		}
	}

And finally, here's how the template generator for i() might look:

.. code-block:: c++

	void TPLGEN_i(Type* types, void* rootFn, path_t path, size_t parentPosition) {
		return;
	}

So the purpose of the template generator functions is to return an array of template argument values for any templated function or type. They achieve this by starting at the *root template generator*, where all the initial template argument values are known, and then using a 32-bit unsigned integer that specifies the path from the *root template generator* to the relevant *intermediate template generator*, at which point the functions will exit and the correct template argument values are returned.

Top Down Calls
~~~~~~~~~~~~~~

This design is probably the reverse of what most developers expect, since it's natural to think of a function accessing its own template arguments or parameters first, and then performing further operations to access template arguments in outer contexts (i.e. 'bottom up' access to template arguments). However template generators always call down from the root template generator until they reach the relevant intermediate generator (i.e. 'top down').

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

The problem is of course that, unlike the top-down path, this path can't be constructed at compile-time from the point of view of 'h()'. The compiler can distinguish between the various template uses that are made within a function or type since they're locally visible, however it can't distinguish between uses of that function or type at any point in any dependent source code, since this is clearly not visible.

Hence the solution is to create a path from the root downwards, since we do know this path at compile time (or, rather, each function knows its children in this graph at compile-time). We can then just remember the root function along with the path from the root to the relevant function (an unsigned integer computed as we perform function calls), and then we can compute the template arguments for that function. This strategy avoids needing to construct complex structures at run-time and, as discussed below, is highly amenable to optimisation.

Pass-through
------------

Most recursive template instantiations are relatively trivial and typically involve passing the same template arguments in the same order. For example:

.. code-block:: c++

	template <typename A, typename B>
	require(is_movable<A> and is_movable<B>)
	A f(B value) {
		return g<A, B>(move value);
	}
	
	template <typename A, typename B>
	require(is_movable<A> and is_movable<B>)
	import A g(B value);

This could produce the following template generator for ``f()``:

.. code-block:: c++

	void TPLGEN_f(Type* types, void* rootFn, path_t path, size_t parentPosition) {
		if (parentPosition == 0) {
			// End of path => return type array.
			return;
		}
		
		const auto position = parentPosition - 1;
		const auto subPath = (path >> position);
		const auto mask = 0x3;
		const auto component = (subPath & mask);
		
		switch (component) {
		case 0:
			TPLGEN_g(types, rootFn, path, position);
			return;
		default:
			// Unreachable!
		}
	}

In this case we've allocated one bit on the path so that we know to continue iterating into the intermediate template generator of ``g()``. However, this isn't actually necessary; the arguments we give to ``g()`` are identical to what we are ourselves expecting so we can call straight down into its intermediate template generator and hence effectively merge these two states:

.. code-block:: c++

	void TPLGEN_f(Type* types, void* rootFn, path_t path, size_t position) {
		TPLGEN_g(types, rootFn, path, position);
		return;
	}

Now ``f()`` doesn't require any bits on the path and its template generator can use a considerably simplified control flow (aiding compiler optimisations and avoiding relying on hardware branch prediction). This works because we can rely on ``g()``'s template generator to return if we've reached the end of the path (i.e. the ``position`` argument is 0), with the same template arguments that we gave it and that were in fact provided to us.

This improvement is called the "Pass-through Optimisation". Any template generator can choose to apply or not apply this transformation without affecting ABI compatibility, however it always makes sense to do so.

Prefix pass-through
~~~~~~~~~~~~~~~~~~~

A variant of pass-through is 'prefix' pass-through, which is the same concept applied when a recursive instantiation is a prefix of the parent instantiation, or vice versa. For example:

.. code-block:: c++

	template <typename A, typename B>
	require(is_movable<A> and is_movable<B>)
	A f(B value) {
		return g<A, B, float>(move value, 10.0f);
	}
	
	template <typename A, typename B, typename C>
	require(is_movable<A> and is_movable<B> and is_movable<C>)
	import A g(B value, C value);

Without pass-through we'd produce a template generator like the following:

.. code-block:: c++

	void TPLGEN_f(Type* types, void* rootFn, path_t path, size_t parentPosition) {
		if (parentPosition == 0) {
			// End of path => return type array.
			return;
		}
		
		const auto position = parentPosition - 1;
		const auto subPath = (path >> position);
		const auto mask = 0x3;
		const auto component = (subPath & mask);
		
		switch (component) {
		case 0:
			types[2] = { VTABLE_float, { NULL, 0 } };
			TPLGEN_g(types, rootFn, path, position);
			return;
		default:
			// Unreachable!
		}
	}

In this case ``g()`` takes slightly different template arguments to ``f()``. However the arguments received by ``f()`` are strictly a prefix of the arguments it gives to ``g()``. Hence the template generator can be reduced to:

.. code-block:: c++

	void TPLGEN_f(Type* types, void* rootFn, path_t path, size_t position) {
		types[2] = { VTABLE_float, { NULL, 0 } };
		TPLGEN_g(types, rootFn, path, position);
		return;
	}

Now when ``f()`` queries its template arguments it gets the unexpected third argument ``float``, but this isn't a problem because ``f()`` will ignore this argument (since it doesn't have a third argument).

The same concept can be applied the opposite way around:

.. code-block:: c++

	template <typename A, typename B, typename C>
	require(is_movable<A> and is_movable<B> and is_movable<C>)
	A f(B value, unused C value) {
		return g<A, B>(move value);
	}
	
	template <typename A, typename B>
	require(is_movable<A> and is_movable<B>)
	import A g(B value);

This will produce the following optimised template generator:

.. code-block:: c++

	void TPLGEN_f(Type* types, void* rootFn, path_t path, size_t position) {
		TPLGEN_g(types, rootFn, path, position);
		return;
	}

``g()`` receives the third argument even though it has no third argument, but again this is fine because ``g()`` will ignore the third argument.

Mutually-Recursive functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Pass-through appears to have an obvious pathological case:

.. code-block:: c++

	template <typename A, typename B>
	require(is_movable<A> and is_movable<B>)
	void f(A a, B b, int i) {
		if (i == 0) {
			return;
		} else {
			return g<A, B>(move a, move b, i - 1);
		}
	}
	
	template <typename A, typename B>
	require(is_movable<A> and is_movable<B>)
	void g(A a, B b, int i) {
		if (i == 0) {
			return;
		} else {
			return f<A, B>(move a, move b, i - 1);
		}
	}

In this case we'd expect to see the following template generators:

.. code-block:: c++

	void TPLGEN_f(Type* types, void* rootFn, path_t path, size_t position) {
		TPLGEN_g(types, rootFn, path, position);
		return;
	}
	
	void TPLGEN_g(Type* types, void* rootFn, path_t path, size_t position) {
		TPLGEN_f(types, rootFn, path, position);
		return;
	}

Obviously this would appear to mean the intermediate template generators infinite loop.

Fortunately, the dependencies of modules must form a **directed acyclic graph**. This means that if one module 'A' performs a **statically determined** call to another module 'B', it is impossible for 'B' to also perform a statically determined call to 'A'. A statically determined call is something like ``function()``, where ``function`` is known as a particular callable function by the compiler; this contrasts with function pointers or interface method calls, the destination of which is determined at run-time.

So if modules can't statically call into each other, then we can't have **inter-module** mutual recursion. It is however possible to have **intra-module** mutual recursions, where a module has two functions within it that each call the other, as with ``f()`` and ``g()`` above.

This case can be handled at compile-time since the compiler has all the relevant knowledge; it can identify mutually-recursive functions with pass-through applied and fix their intermediate template generators to terminate. For example, for the case given above it can create:

.. code-block:: c++

	void TPLGEN_f(Type* types, void* rootFn, path_t path, size_t position) {
		return;
	}
	
	void TPLGEN_g(Type* types, void* rootFn, path_t path, size_t position) {
		return;
	}

This is fairly easy to detect; the compiler produces a graph of the template instantiations and performs cycle detection on the graph. If it finds that pass-through has led to an infinite loop then it simply modifies them to terminate.

Callbacks
~~~~~~~~~

The above analysis seems to miss that modules can be mutually-recursive through the use of callbacks. For example module 'A' calls module 'B' and gives it a function pointer; module 'B' can then call module 'A' later via that function pointer.

However, none of this is relevant to templates because template arguments cannot be provided to a function pointer or interface method call (as in C++). Templates are a static mechanism and are unrelated to run-time recursion. Here's an example of using a callback (note that some of the syntax for templated function pointers is still in development):

.. code-block:: c++

	// ---- In module 'A'.
	template <typename T>
	void f() {
		g(h<T>);
	}
	
	template <typename T>
	void h() {
		f<T>();
	}
	
	// ---- In module 'B'.
	void g((*<>)(void)() function_ptr) {
		function_ptr();
	}

While at run-time we have a cycle of ``f()`` -> ``g()`` -> ``h()`` -> ``f()`` -> etc., the chain of template instantiations is ``f<T>()`` -> ``h<T>()`` -> ``f<T>()``. This is a cycle, but it's a cycle within module 'A' and hence amenable to the approach described previously.

In case you're wondering, a templated function pointer is the following struct:

.. code-block:: c++

	struct templated_function_ptr {
		void* function_ptr;
		struct template_generator {
			void* root_fn;
			path_t path;
		};
	};

So we've created a template generator for ``h()`` inside ``f()`` and we then pass this to ``g()``. This means that normal function pointers can't call templated functions, where the template arguments are **not** fully specified; functions like ``h<int>`` **are** compatible with normal function pointers because the compiler can generate a stub around them.

Optimisation
------------

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

