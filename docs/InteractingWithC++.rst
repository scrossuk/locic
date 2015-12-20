Interacting with C++
====================

This page describes approaches for using Loci and C++ code together. This is currently 'developer advice' but is intended to become part of a tool so this process becomes automatic.

Functions
---------

Functions are generally quite easy to translate to call from Loci into C++ (and vice versa), since both languages have a C-like subset. For example, here's a C++ function:

.. code-block:: c++

	void function();

The following C++ will be generated:

.. code-block:: c++

	extern "C" {
		
		void function() {
			real_cpp_function();
		}
		
	}

Then in Loci:

.. code-block:: c++

	void function();

So it's just a matter of sorting out the name mangling. (In future Loci could learn how to do a direct call into C++.)

The return types and argument types can be all sorts of primitives, structs, enums, etc. and this all should work.

Function overloading
--------------------

A hard case of Loci calling to C++ is dealing with function overloading, since Loci doesn't support it. For example:

.. code-block:: c++

	void function(int i);
	void function(double d);

One solution is some kind of prefix, such as having:

.. code-block:: c++

	void function_int(int i);
	void function_double(double d);

An alternative (in future) might be templates:

.. code-block:: c++

	template <typename T>
	void function(T value) {
		static if (T == int) {
			function_int(value);
		} else {
			function_double(value);
		}
	}

Of course, this won't work for virtual methods in classes.

Clearly, there's no problem the other way around since Loci function names can always be described in C++.

Functions with default arguments
--------------------------------

This should work nicely (once implemented in Loci):

.. code-block:: c++

	void function(int i, double d = 42.0);

It's the same syntax in Loci. Similarly, it should work vice versa.

A slight caveat is the default value expression may need to be replaced with a function call.

C++ Classes
-----------

Non-virtual classes
~~~~~~~~~~~~~~~~~~~

Loci has classes, C++ has classes, so a normal (non-virtual) C++ class can be wrapped easily with a Loci class. For example, here's the original C++ class definition:

.. code-block:: c++

	class CPPClass {
	public:
		CPPClass();
		
		int method();
		
	private:
		int value_;
		
	};

Here's the Loci class declaration:

.. code-block:: c++

	class CPPClass {
		static CPPClass create();
		
		int method();
	}

The Loci compatibility layer would look something like:

.. code-block:: c++

	extern "C" {
		
		size_t MT1N8CPPClassF1N11__alignmask() {
			return alignof(CPPClass) - 1;
		}
		
		size_t MT1N8CPPClassF1N8__sizeof() {
			return sizeof(CPPClass);
		}
		
		void MT1N8CPPClassF1N6create(void* ptr) {
			new(ptr) CPPClass();
		}
		
		void MT1N8CPPClassF1N9__destroy(void* ptr) {
			static_cast<CPPClass*>(ptr)->~CPPClass();
		}
		
		int MT1N8CPPClassF1N4method(void* ptr) {
			return static_cast<CPPClass*>(ptr)->method();
		}
		
	}

As always the Loci methods are just C functions with mangled names, so it's relatively easy to construct them. A nice aspect of this is that the size of the C++ class is hidden behind the API, which means the C++ class can have members added/removed and the Loci API client doesn't need to be re-compiled (as with Loci classes). Similarly, we can use C++ classes with Loci's interfaces.

If any function takes a non-virtual class, we simply have to translate from the Loci pointer to the C++ pointer, which is a NOOP.

Virtual classes
~~~~~~~~~~~~~~~

This is a considerably harder case because:

* Loci stores its vtable **outside** the object in the reference type; C++ stores its vtable **inside** the object.
* Loci uses hash table based virtual calls; C++ uses a fixed offset table.
* Loci doesn't support polymorphic class inheritance, whereas C++ does.

Here's an example C++ class:

.. code-block:: c++

	class CPPClass {
	public:
		CPPClass();
		
		virtual int method();
		
	private:
		int value_;
		
	};

In Loci this becomes:

.. code-block:: c++

	interface ICPPClass {
		int method();
		
		void* _cppclass(ICPPClass* object);
	}
	
	class CPPClass {
		static CPPClass create();
		
		int method();
		
		void* _cppclass(ICPPClass* object);
	}

This construction splits the class into an interface and a class, with a new method that gets a pointer to the C++ class given the interface pointer.

The new method is required in case a Loci class inherits from the C++ class. The C++ class wrapper will reserve space for the Loci class vtable and this will be updated when ``_cppclass`` is called (it could be set on construction, but this approach is likely to be less mistake prone. So the class wrapper implementation looks like:

.. code-block:: c++

	class CPPClass(RealCPPClass realCPPClass, ICPPClass* classRef) {
		[...]
		
		void* _cppclass(ICPPClass* object) {
			@classRef = object;
			return this;
		}
	}

We've had to store the vtable directly in the object since this is what C++ requires, and Loci would otherwise only store it in the reference; clearly the Loci reference is larger than the C++ reference so the two are not interchangeable.

Here's an example inheritance in Loci (once the ``inherit`` keyword has been implemented):

.. code-block:: c++

	class InheritingLociClass(inherit CPPClass object) {
		[...]
	}

``InheritingLociClass`` will now have the ``_cppclass`` method from ``CPPClass`` (as well as all the other methods) so we can pass it to functions expecting ``CPPClass``. For example, here's a C++ function taking a reference to ``CPPClass``:

.. code-block:: c++

	void function(CPPClass& object);

This is translated into the following Loci:

.. code-block:: c++

	void function(ICPPClass& object) {
		real_cpp_function(object._cpp_class(&object));
	}

Hence all virtual calls to the C++ class wrapper will then call into the Loci interface.

This only covers half the problem, since we can also receive C++ objects by reference:

.. code-block:: c++

	CPPClass& function();

Here's how we translate this:

.. code-block:: c++

	class CPPClass_refwrapper {
		int method() {
			return real_cppclass_method();
		}
		
		void* _cppclass(ICPPClass* object) {
			return this;
		}
	}
	
	ICPPClass& function() {
		CPPClass_refwrapper& real_object = real_cpp_function();
		return real_object;
	}

``CPPClass_refwrapper`` is another generated class that allows us to create a Loci interface reference to call into the C++ class. This is fairly straightforward as (unlike C++) Loci lets us store the vtable pointer inside the reference. There's a bit of duplication here though, since the C++ class also contains its own vtable. However the two are incompatible and the Loci vtable is more powerful since it allows structural typing.

Note that the ``_cppclass`` of ``CPPClass_refwrapper`` doesn't do anything, as it already has all the information it needs to perform virtual calls.

Finally, we can have functions which receive or return the class by value, and we just use the ``CPPClass`` wrapper in this case:

.. code-block:: c++

	// C++
	CPPClass function();
	
	// Loci
	CPPClass function() {
		return CPPClass._from_cppclass(real_cpp_function());
	}
	
	// C++
	void function(CPPClass object);
	
	// Loci
	void function(CPPClass object) {
		real_cpp_function(object.real_cpp_object);
	}

Loci interfaces
---------------

References to interfaces
~~~~~~~~~~~~~~~~~~~~~~~~

Using Loci interfaces in C++ is non-trivial, once again because Loci stores the vtable in the reference type whereas C++ stores it in the object.

For example:

.. code-block:: c++

	interface Test {
		void method();
	}
	
	Test& function();

One way of resolving this would be to have a C++ value class corresponding to the reference:

.. code-block:: c++

	class TestRef {
	public:
		TestRef(loci_ref_t value);
		
		void method();
		
	private:
		loci_ref_t ref_;
		
	}
	
	TestRef function();

This fixes the problem at a cost of being a little unusual.

Implementing interfaces
~~~~~~~~~~~~~~~~~~~~~~~

Implementing Loci interfaces should be reasonably straightforward.

For example:

.. code-block:: c++

	interface Test {
		void method();
	}

We can translate this to:

.. code-block:: c++

	class Test {
	public:
		virtual void method() = 0;
		
	};

We can then simply inherit from the virtual class and cast this to a Loci interface every time we're passing it to Loci code.

Templates
---------

The interaction between C++ templates and Loci templates is interesting, since Loci supports templates across ABI boundaries whereas C++ does not.

Using C++ templates in Loci
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here's a C++ templated function:

.. code-block:: c++

	template <typename T>
	T min(T a, T b) {
		return b < a ? b : a;
	}

This is very different to Loci templates because:

* The template must be evaluated in Semantic Analysis, but Loci defers template evaluation to CodeGen/optimisation/run-time.
* The template doesn't provide any information about type requirements.

This means that usual Loci templates won't work here, and a new form of 'static' templates will be needed. For example:

.. code-block:: c++

	static template <typename T>
	T min(T a, T b);

This would need to trigger the evaluation of the C++ template, which is clearly non-trivial since the Loci compiler would itself need to be outputting C++ code (this seems a likely eventuality); in the meantime it makes more sense to instantiate the C++ templates as needed up-front.

Using Loci templates in C++
~~~~~~~~~~~~~~~~~~~~~~~~~~~

This direction is a lot easier, because we only have to generate code that creates the appropriate template instantiation and we don't have to evaluate the template.

For example, here's a Loci templated function:

.. code-block:: c++

	template <typename T>
	require(is_lessthancomparable<T>)
	T min(T a, T b);

We should just need to translate the C++ type into a Loci type, and then instantiate. The main complexity will be that root template generators would need to be emitted in C++ so the C++ compiler can produce them itself. Something like:

.. code-block:: c++

	template <typename T>
	bool lessthan_fn(void* ptr, void* other) {
		return *(static_cast<T*>(ptr)) < *(static_cast<T*>(other));
	}
	
	extern vtable_t lessthanvtable;
	
	template <typename T>
	void root_generator(type_info_t* args, path_t path) {
		// Possibly use the template generator to hold the type's functions.
		args[0] = { &lessthanvtable, { lessthan_fn<T>, 1 } };
		[..etc..]
	}
	
	void min(void* result, void* a, void* b, template_generator_t tplgen);
	
	template <typename T>
	T cpp_min(T a, T b) {
		template_generator_t tplgen;
		tplgen.root_generator = cpp_min_root_generator<T>;
		tplgen.path = 1;
		T result;
		min(&result, &a, &b, tplgen);
		return result;
	}
