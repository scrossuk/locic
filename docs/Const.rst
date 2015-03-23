Const
=====

The 'const' keyword is used in both C and C++ (as well as some other languages) to mark that data should not be modified by a specific section, or to indicate that a function guarantees to not modify its argument, or a method guarantees to not modify its owning object. Loci also provides 'const', but with some simple syntactic and semantic changes.

Const Values
------------

As in C and C++ 'const' prevents modifications of values. For example, the following code is invalid:

.. code-block:: c++

	void function() {
		const int i = 0;
		i = 1;
	}

In this case the compiler will complain that the underlying :doc:`lvalue <LvaluesAndRvalues>` has a :doc:`require predicate <Predicates>` for its 'assign' method that specifies the templated type must be movable; const types are NOT movable and therefore the assignment cannot occur.

Here's another example of invalid code in both C++ and Loci:

.. code-block:: c++

	void function() {
		int i = 0;
		const int* p = &i;
		*p = 1;
	}

Again the template type of the dereferenced pointer lvalue is const and hence not movable.

Transitivity
------------

Consider the following C++ code:

.. code-block:: c++

	void function() {
		int i = 0;
		int* const p = &i;
		*p = 1;
	}

Here the pointer is declared const but the value it points to is **not** const, which means the code is accepted. Note that while const is transitive within values in C++, it is **not** transitive across pointers.

In Loci const is transitive in all cases, so the code might look like:

.. code-block:: c++

	void function() {
		int i = 0;
		const(int*) p = &i;
		*p = 1;
	}

This program will be rejected by the compiler, because 'p' is a const pointer to a const int, due to the transitivity of const.

Not only is the syntax much simpler here but the semantics of transitive const are much more obvious. For example, consider:

.. code-block:: c++

	struct Example {
		int* value;
	};
	
	void function(const Example exampleInstance) {
		*(exampleInstance.value) = 42;
	}

This code would be valid in C++ but is *invalid* in Loci. The intention behind this approach is to provide behaviour that is clearer and more closely matches the intuition of developers.

Final
-----

Consider the following code:

.. code-block:: c++

	void function() {
		int i = 0;
		const(int*) p = &i;
		*p = 1;
	}

In this case we may have intended to use const to prevent accidental assignments to p, but in this case due to the transitivity of const we've also disabled assignments to the value it points-to.

Fortunately the 'final' keyword provides a way to prevent assignments to an lvalue without having to mark it const. So the above code would become:

.. code-block:: c++

	void function() {
		int i = 0;
		final int* p = &i;
		*p = 1;
	}

Now the code will compile, but any assignments to 'p' itself fails. The implementation of this keyword is to use a 'final_lval' as the underlying lvalue type, which doesn't support assignment in any case, rather than 'value_lval' (which does support assignment for non-const types).

Note that 'final' is an lvalue qualifier (or 'variable qualifier') rather than a type qualifier, so doesn't affect Loci's type system in any way.

Logical Const
-------------

Loci provides 'const' to mark data as logically constant, which means that the fundamental memory contents of 'const' objects may vary, as long as there is no change to the external behaviour of the object. ''No change to the external behaviour" means the following two functions 'function' should be equivalent, and any transformation between them is valid:

.. code-block:: c++

	void f(const Type& value);
	void g(const Type& value);
	
	void function(){
		const Type var = SOME_EXPR;
		f(var);
		g(var);
	}
	
	void function(){
		const Type var = SOME_EXPR;
		const Type tmpVar = _copy_of_var_;
		f(var);
		g(tmpVar);
	}

Here '_copy_of_var_' means a simple byte-for-byte copy of variable 'var' that does not involve invoking a 'copy' method, and furthermore that the destructor for 'tmpVar' is not run. Therefore, this effectively means that the second function does not have to reload the value of the variable from memory, since it can assume that it has not changed.

Compilers are allowed to optimise (note also that optimisations can only be performed where the compiler can prove that it has the only (const) reference to an object, otherwise other parts of the program may have non-const references to the object and thereby modify it in parallel, or as part of, the execution of the function with the const reference) based on the validity of this transformation. This optimisation requires that 'f' and 'g' operate within type rules and don't use 'const_cast', and this is a requirement that the developer must follow. Consider, for example:

.. code-block:: c++

	// A type alias.
	using CString = const uchar *;
	
	void unknownStringOperation(const CString string);
	void printSize(size_t size);
	void printStringLength(const CString string) {
		size_t length = 0;
		CString ptr = string;
		while(*ptr != 0x00u){
			length++;
			ptr++;
		}
		printSize(length);
	}
	
	void function() {
		// Prefix 'C' means 'C string'; this is explained later.
		const CString string = C"This is a string";
		unknownStringOperation(string);
		printStringLength(string);
	}

By the above equivalence, the compiler can assume this is equivalent to:

.. code-block:: c++

	// ... as above ...
	
	void function() {
		unknownStringOperation(C"This is a string");
		printStringLength(C"This is a string");
	}

Which, in combination with other transformations (such as inlining), leads to the optimised code:

.. code-block:: c++

	// A type alias. Note that 'const CString' is actually 'char* const'.
	using CString = char *;
	
	void unknownStringOperation(const CString string);
	void printSize(size_t size);
	
	void function() {
		unknownStringOperation(C"This is a string");
		printSize(cast<size_t>(16));
	}

Overriding Const
----------------

As part of 'logical const', Loci provides the '__override_const' keyword, which allows developers to explicitly ignore const markers if needed:

.. code-block:: c++

	struct Struct {
		int normalField;
		__override_const int mutableField;
	};
	
	void function(const Struct& ref) {
		// Invalid - 'normalField' is now const.
		ref.normalField = 1;
		
		// Valid - mutable keyword overrides const.
		ref.mutableField = 1;
	}

Following the rules of logical const, '__override_const' should only ever be used when it has no effect on the external behaviour of an object. Again, this means the above transformation should apply. And since optimisations occur based on const, it is important that developers only use '__override_const' when absolutely necessary and ensure correctness when it is used.

A good example of its correct use would be in a reference counting smart pointer class, in which the reference count field can (and should) be marked as '__override_const'. Considering the transformation above once again, it doesn't matter whether 'f' modifies the reference count (it could, for example, create a copy of the smart pointer and store it somewhere, increasing the reference count), because 'g' only depends on a count greater than 0 (and the reference counting invariant is intended to ensure that is always true until the last smart pointer object is destroyed).

Marking class member variable mutexes as '__override_const' is another example of a good use of the keyword, since 'lock' and 'unlock' methods modify the external behaviour of the mutexfootnote{Consider calling 'lock' twice in a row, without calling 'unlock'.} and therefore require it to be non-const, but any object that contains a mutex to handle races uses it in a way that does not affect its external behaviour (i.e. the above transformation is valid):

.. code-block:: c++

	class Mutex {
		void lock();
		void unlock();
	}
	
	class Lock(Mutex& mutex) {
		static create(Mutex& mutex) {
			mutex.lock();
			return @(mutex);
		}
		
		~ {
			@mutex.unlock();
		}
	}
	
	class CustomType(__override_const Mutex mutex, Type value){
		// ...
		
		void setValue(Type value) {
			auto lock = Lock(@mutex);
			@value = value;
		}
		
		int getValue() const {
			// 'Lock' object will call 'lock'
			// and 'unlock' on the mutex.
			auto lock = Lock(@mutex);
			return @value;
		}
	}

Methods
-------

Methods can be marked as 'const' to indicate they do not modify their owning object, as used above. Here's another example:

.. code-block:: c++

	class ClassObject {
		void normalMethod();
		void constMethod() const;
	}
	
	void f(const ClassObject& object){
		// Invalid - non-const method cannot
		// be called on const object.
		object.normalMethod();
		
		// Valid.
		object.constMethod();
	}

It's possible to specify a :doc:`predicate <Predicates>` within the const declaration. For example:

.. code-block:: c++

	template <bool IsConst>
	class ClassObject {
		void method() const(IsConst);
	}

Casting Const Away
------------------

**NOTE**: Feature not currently implemented; awaiting further design consideration.

Const can be cast away if needed with 'const_cast', but doing so could be very dangerous, since the compiler may be performing transformations as above. This means that the only valid use for const_cast is to modify the type of a pointer to support an API that fails to use 'const', but it is guaranteed that the API does not modify the object:

.. code-block:: c++

	void doSomething(int i);
	
	void oldAPI(int* i) {
		doSomething(*i);
	}
	
	void f(const(int*) i) {
		oldAPI(const_cast<int *>(i));
	}


