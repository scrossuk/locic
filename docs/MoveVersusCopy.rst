Move vs Copy
============

C++ copies objects by default, through the use of copy constructors. These can be implicitly generated, so copying can only be prevented by marking the copy constructor as private or (in C++11 only) removing it entirely by the delete keyword. C++11 also adds support for *move* operations, through a complex theory which extends lvalues and rvalues with prvalues, xvalues and glvalues, but which must be enabled for each type by the programmer.

	C++ can therefore be considered to use *copy* by default, *move* by choice.

	Loci reverses this: it uses *move* by default, *copy* by choice.

Move by default
---------------

Here’s an example of "*move* by default":

.. code-block:: c++

	class Resource(size_t size, void* ptr) {
		static create(size_t size) {
			return @(size, malloc(size));
		}
		
		~ {
			free(@ptr);
		}
		
		size_t size() const {
			return @size;
		}
	}
	
	void f() {
		auto resource = Resource(100);
		g(move resource);
	}
	
	void g(Resource resource) {
		std::println("Resource size: %0".format(resource.size()));
	}

As well as demonstrating some interesting features of Loci, this example shows a use of the *move* operator. This operator is actually a method call on an lvalue type, and extracts the contents of an lvalue and returns it as an rvalue. Note that the programmer doesn’t need to do any work to enable *move* operations for the class type.

The clear practical benefit in the example is that the Resource instance can be trivially passed between the functions f() and g() without any copying; both the constructor and destructor will be called exactly once.

Copy by choice
--------------

Here’s an example of "*copy* by choice":

.. code-block:: c++

	class Resource(size_t size, void* ptr) {
		static create(size_t size) {
			return @(size, malloc(size));
		}
		
		~ {
			free(@ptr);
		}
	
		Resource copy() const {
			return @(size, malloc(@size));
		}
		
		size_t size() const {
			return @size;
		}
	}
	
	void f() {
		auto resource = Resource(100);
		g(resource.copy());
	}
	
	void g(Resource resource) {
		std::println("Resource size: %0".format(resource.size()));
	}

Here the programmer of the class has decided to include a *copy* method, which makes it possible to create a *copy* of the Resource instance. This example therefore shows the creation of two instances.

Implicit copy
-------------

The above example shows the creation of an explicit *copy* method. However, there are many types (e.g. a 2D vector of ints) that are cheap to *copy* and therefore justify an implicit *copy* mechanism.

Loci supports this through a method named *implicitCopy*; here’s an example:

.. code-block:: c++

	class Vector2D(int x, int y) {
		static create = default;
		
		Vector2D implicitCopy() const {
			return @(@x, @y);
		}
	}
	
	void f(Vector2D v) {
		Vector2D w = v;
		Vector2D x = w;
		// etc..
	}

This seems like a lot of work for such a simple type, and it is! A 2D Vector is better suited to :doc:`Algebraic Datatypes <AlgebraicDatatypes>`:

.. code-block:: c++

	datatype Vector2D(int x, int y);
	
	void f(Vector2D v) {
		Vector2D w = v;
		Vector2D x = w;
		// etc..
	}

Algebraic datatypes are also just class types, but in this case an implicitCopy method is automatically generated if all the datatype's children support implicit *copy*.

Why move by default?
--------------------

Move operations are almost always cheaper than *copy* operations, often to a great extent (consider a variable sized heap allocated array). Having *move* operations enabled for all types gives a great deal of flexibility to programmers for a very small (generally negligible) cost.

In comparison, C++03 (and earlier) programs that disabled *copy* operations were then forced to pin objects into a single position in memory. To get around this restriction some types tried to implement *move* operations in terms of *copy* operations, a notable example being std::auto_ptr (which is now deprecated in favour of the *move*-operation-based std::unique_ptr).

Why not copy by default?
------------------------

For many types copying is expensive. More importantly, the implicitly generated *copy* constructor may be incorrect (e.g. performing a shallow *copy* of a pointer rather than a necessary deep *copy*), leading to subtle program bugs. Even more importantly, some types do not have a useful meaning for a *copy* operation.

Loci requires programmers to specify *copy* methods (for class types) to ensure that *copy* operations are always meaningful; C++ programmers can easily forget to delete the *copy* constructor. Note that programmers must also handle the assignment operator in C++ on a per-type basis; in Loci this is handled by lvalue types.

Customising move operations per type
------------------------------------

(If you're looking to create a new lvalue type then see :doc:`LvaluesAndRvalues <LvaluesAndRvalues>` for an explanation of this.)

In some (rare) cases a type has custom behaviour during a move operation, involving more logic than simply a *memcpy* from the source address to the destination address. Loci allows this to be customised by manually implementing a method called *__move_to*, which by default just performs a *memcpy*.

Here's an example:

.. code-block:: c++

	class TestClass(int value) {
		static Create() {
			return @(0);
		}
		
		int value() const noexcept {
			return @value;
		}
		
		void __move_to(void* ptr, size_t position) noexcept {
			@value += 1;
			@value.__move_to(ptr, position);
		}
	}

Here the class is essentially counting the number of times it is moved. Ultimately the *__move_to* method is what lvalue types call to transfer an object from one area in memory to another and customising this property therefore provides the developer additional flexibility for their classes.

There are a few things to note about this:

* The *__move_to* method must be *noexcept*, since move operations cannot throw.
* The arguments are the pointer to the base object (which could contain our object) and the offset of our object within the base object.
* The compiler is allowed to increase/reduce (typically the latter!) the number of move operations, affecting the behaviour of this program.

It's generally advisable to **NOT** customise *__move_to* methods unless there is a clear need and the resulting behaviour is well understood.
