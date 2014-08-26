Const
=====

The 'const' keyword is used in both C and C++ (as well as some other languages) to mark that data should not be modified by a specific section, or to indicate that a function guarantees to not modify its argument, or a method guarantees to not modify its owning object. Loci also provides 'const', but with some syntactic and semantic changes.

Const and Final
---------------

One of the most notable changes for programming languages, which ties in with the concept of 'lvals', is a modification in the application of 'const' so it (generally) only applies to 'r-values' and not to 'l-values'. Consider the following example in C:

.. code-block:: c++

	void function() {
		const int i = 0;
		i = 1;
	}

This is invalid C code, since the assignment is attempting to assign to a variable declared 'const'. In constrast, this code is valid in Loci, and the 'const' is essentially meaningless, since it applies to the 'int' type rather than the 'lval' that holds the 'int'. To make this clearer, here is an equivalent example with explicit lval types:

.. code-block:: c++

	void function() {
		lval<const int> value_lval<const int> i = lval<const int>(value_lval<const int>(0));
		i = 1;
	}

It's then clearer in this case that while 'int' is declared as const, the implicitly generated 'value_lval' is not (and so its non-const 'assign' method can be called). The correct solution to this problem in Loci is to use 'final':

.. code-block:: c++

	void function() {
		final int i = 0;
		i = 1;
	}

This has the desired result of generating a suitable compile time error for the assignment. Again, for clarity, here's the explicit form:

.. code-block:: c++

	void function() {
		lval<int> const value_lval<int> i = lval<int>(value_lval<int>(0));
		i = 1;
	}

So 'final' provides a way to specify that the implicitly generated lval is const. To understand the rationale for this decision, consider:

.. code-block:: c++

	class ExampleClass {
		static ExampleClass create();
		void modify();
	}
	
	void function(final const ExampleClass * const examplePtr) {
		examplePtr = null;
		*examplePtr = ExampleClass();
		examplePtr->modify();
	}

Each statement in 'function' tests a different 'node' in the type tree, and are allowable or not dependent on whether that node is const. Each statement in the function generates a compile-time error, since:

* The application of 'final' means the lval is const, hence the variable can't be re-assigned.
* The application of 'const' after the asterisk means the pointer is const, hence its 'deref' method generates a const lval (in this case, a 'ptr_lval') and so the value it points-to cannot be re-assigned.
* The application of 'const' before 'ExampleClass' means the object being pointed to is also const, and therefore also cannot be modified. This prevents any calls to non-const methods (such as 'modify').

Const Chains
------------

Const implicit casting rules follow a concept of 'const chains', meaning that a type node may only become const when all its parent nodes have become const (i.e. there is a chain from the 'root' to the type node of const types). Here's an example:

..

	Valid:   int ** -> int ** const
	
	Invalid: int ** -> int * const *
	
	Valid:   int ** const -> int * const * const
	
	Invalid: int ** const -> int const ** const
	
	Valid:   int ** const -> int const * const * const
	
	Valid:   int ** -> int const * const * const
	
	Invalid: int * const * -> int const * const *
	
	Valid:   int * const * -> int const * const * const

Note that, in the last case, a type such as 'int\* const\*' can be achieved by code such as following:

.. code-block:: c++

	void function() {
		int i = 0;
		int* j = &i;
		int* const k = j;
		int* const * l = &k;
		// etc.
	}

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

	// A type alias. Note that 'const CString' is actually 'char* const'.
	using CString = char *;
	
	void unknownStringOperation(const CString string);
	void printSize(size_t size);
	void printStringLength(const CString string) {
		size_t length = 0;
		CString ptr = string;
			while(*ptr != 0x00){
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

Mutable
-------

As part of 'logical const', Loci provides the 'mutable' keyword, which allows developers to explicitly ignore const markers if needed:

.. code-block:: c++

	struct Struct{
		int normalField;
		mutable int mutableField;
	};
	
	void function(const Struct& ref){
		// Invalid - 'normalField' is now const.
		ref.normalField = 1;
		
		// Valid - mutable keyword overrides const.
		ref.mutableField = 1;
	}

Following the rules of logical const, 'mutable' should only ever be used when it has no effect on the external behaviour of an object. Again, this means the above transformation should apply. And since optimisations occur based on const, it is important that developers only use 'mutable' when absolutely necessary and ensure correctness when it is used.

Of course this means the example given for 'mutable' (involving the mutable field of a struct) is an incorrect use, since the field marked as 'mutable' is directly accessible to functions 'f' and 'g'footnote{Consider the case where 'f' modifies the mutable field, and 'g' reads it.}.

A good example of its correct use would be in a reference counting smart pointer class, in which the reference count field can (and should) be marked as 'mutable'. Considering the transformation above once again, it doesn't matter whether 'f' modifies the reference countfootnote{It could, for example, create a copy of the smart pointer and store it somewhere, increasing the reference count.}, because 'g' only depends on a count greater than 0 (and the reference counting invariant is intended to ensure that is always true until the last smart pointer object is destroyed).

Marking class member variable mutexes as 'mutable' is another example of a good use of the keyword, since 'lock' and 'unlock' methods modify the external behaviour of the mutexfootnote{Consider calling 'lock' twice in a row, without calling 'unlock'.} and therefore require it to be non-const, but any object that contains a mutex to handle races uses it in a way that does not affect its external behaviour (i.e. the above transformation is valid):

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
	
	class CustomType(mutable Mutex mutex, Type value){
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

	class ClassObject{
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

Casting Const Away
------------------

Const can be cast away if needed with 'const_cast', but doing so could be very dangerous, since the compiler may be performing transformations as above. This means that the only valid use for const_cast is to modify the type of a pointer to support an API that fails to use 'const', but it is guaranteed that the API does not modify the object:

.. code-block:: c++

	void doSomething(int i);
	
	void oldAPI(int * i){
		doSomething(*i);
	}
	
	void f(const int * const i){
		oldAPI(const_cast<int *>(i));
	}


