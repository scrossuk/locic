Lvalues and Rvalues
===================

Developers from many programming languages will be familiar with the concept of lvalues and rvalues. Here’s a rough definition of these terms:

	lvalue – a slot that holds a value

	rvalue – a value in transit between slots

If you're a C or C++ developer, you’ll be familiar with the idea that an lvalue represents a location in memory. For example, consider the following C code:

.. code-block:: c

	void f() {
		int a = 1;
		
		// Valid - 'a' is an lvalue, so can assign.
		a = 2
		
		// Value - 'a' is an lvalue, so can take address.
		int* b = &a;
		
		// Valid - '*b' is an lvalue, so can assign.
		*b = 3;
		
		// INVALID - '1' is an rvalue, so CANNOT assign.
		1 = 2;
		
		// INVALID - '1' is an rvalue, so CANNOT take address.
		int* c = &1;
	}

The rules demonstrated above should feel intuitive to many programmers, and it's simple to understand how this works for primitive types such as int.

Custom Lvalues
--------------

Loci significantly extends the above by allowing custom lvalue types to be created. Here’s an example:

.. code-block:: c++

	class CustomLval(int v) {
		static create = default;
		
		void assign(int v) {
			@v = v;
		}
		
		int* address() {
			return &@v;
		}
		
		int& dissolve() {
			return @v;
		}
		
		int move() {
			int tmp = @v;
			@v = 0;
			return tmp;
		}
	}
	
	void f(lval CustomLval value) {
		// Calls CustomLval::address.
		int* intPointer = &value;
		
		// Calls CustomLval::move.
		int intMovedValue = move value;
		
		// Calls (implicitly) CustomLval::dissolve.
		int& dissolvedReference = value;
		
		// Calls CustomLval::assign.
		value = 1;
	}

Here the CustomLval appears to be an ordinary class type. However the 'lval' keyword allows the CustomLval instance to be used as an lvalue directly, meaning that all lvalue-specific operations (such as assignment and taking the address) are translated into method calls.

Primitive Lvalues
-----------------

The above code raises the question of the behaviour when using a normal type without the lval keyword. For example, consider the following:

.. code-block:: c++

	void f(int value) {
		// Calls value_lval<int>::address.
		int* intPointer = &value;
		
		// Calls value_lval<int>::move.
		int intMovedValue = move value;
		
		// Calls (implicitly) value_lval<int>::dissolve.
		int& dissolvedReference = value;
		
		// Calls value_lval<int>::assign.
		value = 1;
	}

As the comments suggest, there’s a primitive type called 'value_lval' being generated implicitly, that has trivial (and intuitive) implementations of the lvalue methods.

Lvalues and Memory Management Classes
-------------------------------------

Consider this Loci class definition:

.. code-block:: c++

	class SomeType(int * p) {
		static create(int value) {
			return @(std::new_raw<int>(value));
		}
		
		~ {
			std::delete_raw<int>(@p);
		}
	}

This is essentially a complete implementation of a class (of course, additional methods could add functionality e.g. set the integer value pointed to) that manages a pointer to an integer; the destructor will be run exactly once for each call to the constructor.

Ensuring this simple invariant in C++11 (note that doing this in C++03 is substantially more complex, and in fact does not support move operations) is surprisingly difficult:

.. code-block:: c++

	// This is C++ code!
	class SomeType{
		public:
			SomeType(int value)
				: p_(new int(value)) { }
			
			SomeType(SomeType&& t) 
				: p_(nullptr) {
				std::swap(p_, t.p_);
			}
			
			SomeType& operator=(SomeType t) {
				std::swap(p_, t.p_);
				return *this;
			}
			
			~SomeType() {
				delete p_;
			}
		
		private:
			int * p_;
		
	};

While the Loci code is very clear, the C++ code is much less clear and requires the careful attention of the developer to ensure correctness.

It's actually even more complex than it first appears, since C++ essentially requires the creation of separate dead states for objects (note the 'nullptr' initialisation in the move constructor), while in Loci developers only need to consider an object's valid states. Note in particular that the destructor of the Loci class will always be run with a non-null value for '@p', whereas the C++ class will sometimes be run in a 'dead state' (i.e. 'p\_' being NULL).

The key reason for this difference is that lvalues are a separate object to the value they hold in Loci, following the Single Responsibility Principle (SRP says every class should have a single responsibility, and that responsibility should be entirely encapsulated by the class). Hence developers only have to handle behaviour such as address, assignment, move etc. in these lval classes (this also provides substantial freedom to customise lvals), allowing non-lval objects to focus on their own responsibilities.

The consequence of the above is that C++ developers must rewrite the same code multiple times (violating DRP - 'Don't repeat yourself') to ensure classes are used safely, following procedures such as the rule of three (if a class defines one of a copy constructor, destructor or copy assignment operator, it should almost always implement all three; in C++11 this can potentially become the rule of five, including a move constructor and move assignment operator). In Loci this isn't necessary, since the design of the language has eliminated this repetition.

