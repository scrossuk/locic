Heap Memory Management
======================

Unlike C++, Loci does not provide *new* or *delete* operators. Instead standard library functions are provided to allocate objects on the heap that follow various strategies (e.g. unique ownership versus reference counting).

.. code-block:: c++

	class Type(int a) {
		static create = default;
	}
	
	Type f() {
		return Type(5);
	}
	
	void function() {
		// Stack allocation.
		Type stackAllocated = Type(0);
		
		// 'Raw' heap allocation.
		Type * heapAllocated = std::new_raw<Type>(Type(0));
		
		// Allocates space on heap, and moves return
		// value of function 'f' from stack to heap.
		Type * heapMoved = std::new_raw<Type>(f());
		
		// Raw allocations must be deallocated.
		std::delete_raw<Type>(heapAllocated);
		std::delete_raw<Type>(heapMoved);
	}

In particular, C++ forces the new operator to call a type constructor, so that stack objects are moved to the heap by a copy constructor. Loci, however, makes it easy to move objects without running any code (other than the class' *__move* method, which by default is just a memcpy), and therefore heap allocation functions have been designed to be applied to any R-value (the details of which are described later) of the desired type, and will move that value onto the heap.

In the example code the function *std::new_raw* is called, meaning that the developer is responsible for calling *std::delete_raw* on the pointers when finished with them (i.e. as with *new* and *delete* in C++). Also note that due to template type inference, the template parameter to heap allocation functions can be omitted:

.. code-block:: c++

	void function() {
		Type * heapAllocated = std::new_raw(Type(0));
		// etc..
	}

Ownership Strategies
--------------------

The previous sample code showed *raw* allocations, in which allocation and construction of objects is performed as in C++. However, Loci supports many other ownership strategies (and it is recommended for developers to **avoid** *raw* allocations as much as possible!):

.. code-block:: c++

	void function() {
		// Automatically freed when leaving scope,
		// but only one instance can exist.
		std::unique_ptr<Type> uniquePtr = std::new_unique<Type>(Type(0));
		
		// Above is essentially equivalent to...
		std::unique_ptr<Type> uniquePtr1 = std::unique_ptr<Type>(std::new_raw<Type>(Type(0)));
		
		// Invalid - cannot copy unique pointers.
		std::unique_ptr<Type> uniquePtr2 = uniquePtr;
		
		// Allocated via garbage collection.
		Type* gcPtr = std::new_gc<Type>(Type(0));
		
		// Reference counted.
		std::rc_ptr<Type> refCountedPtr = std::new_rc<Type>(Type(0));
		
		// Can be copied, increasing ref count.
		std::rc_ptr<Type> refCountedPtr1 = refCountedPtr;
		
		// Shared; could be GC or referencing counting.
		std::shared_ptr<Type> sharedPtr = std::new_shared<Type>(Type(0));
		
		// Can be copied.
		std::shared_ptr<Type> sharedPtr1 = sharedPtr;
	}

Design decisions for new memory management methods were made in consideration of the fundamental aim to minimise the syntax required to instantiate an object on the heap, while maximising the available options for how to manage the memory. Note that there is **not** a default allocation method and hence programmers must carefully consider how objects are to be heap allocated.

That said, here's some general advice:

* Use the stack whenever possible (for objects smaller than 4KB).
* If heap allocation is required, and unique ownership is acceptable, use *std::unique_ptr*.
* *RARE*: If unique ownership isn't viable, use *std::rc_ptr* or *std::shared_ptr* for shared ownership semantics.
* *VERY RARE*: If garbage collection is required, use *std::new_gc*.
* **ALMOST NEVER**: If the object is a low level container directly managing some memory (e.g. you're making a new smart pointer), use raw allocation but **always remember exception safety!**

C++ chooses to use a raw allocation as the basis of the *new* operator, but this is about the worst possible option because exceptions can cause a *delete* statement to be missed and therefore memory is leaked; the other memory management methods are much safer.

Unique Ownership
~~~~~~~~~~~~~~~~

While the other ownership strategies are relatively complex, the 'unique' method is trivial, and returns an object of type *std::unique_ptr*, that is very similar to C++11's std::unique_ptr.

Unique allocation should generally be used as much as possible as long as it doesn't restrict the developer, since it offers no overhead against a *raw* allocation, and yet will reliably recollect the memory and call the destructor.

Reference Counting
~~~~~~~~~~~~~~~~~~

Reference counting provides a simple and deterministic way to release memory and call destructors when there are no remaining references to an object on the heap. Unlike garbage collection, which cannot ensure destructors are called, reference counting means destructors are called as soon as the last referring *std::rc_ptr* instance is destroyed.

It does however restrict the developer to passing around *std::rc_ptr* instances that are responsible for incrementing and decrementing the count. Another key limitation of reference counting is the failure to collect cycles; *std::rc_weak_ptr* is provided with the usual 'weak' semantics.

Shared Ownership
~~~~~~~~~~~~~~~~

*std::shared_ptr* is provided to allow code (the imagined use case here is libraries, that are often dependent on the choices of the program that links them) to take advantage of GC where it is available, but falls back to reference counting otherwise. This is typically best for immutable structures such as trees, and performance is usually better when using garbage collection.

This method of allocation is slightly more limited than using 'gc' and does not expose a raw pointer that can be freely copied around the program, since this obviously wouldn't work with reference counting. Another restriction is that types must not have destructors, since GC cannot provide any sort of deterministic destruction. Finally, developers must be careful to avoid cycles.

In many ways, therefore, shared allocation suffers from the combined weaknesses of both GC and reference counting. It does, however, allow code that would hope to use garbage collection fall back to reference counting when necessary.

Garbage Collection
~~~~~~~~~~~~~~~~~~

Garbage collection may also be available, using a conservative collector (meaning that it doesn't need to move memory, which would be difficult to do at the systems level), and this is generally a good choice for allocating POD ('Plain Old Data') types such as arrays, strings, trees etc. The availability of garbage collection depends on the standard library build type, so that some applications can choose to not use garbage collection; in such a case the developer will experience linker errors that will make the problem clear.

As with shared allocation, Loci will disallow the allocation of an object via garbage collection if it has a destructor. Finalisers could've been used to trigger destructor execution, but they are *not deterministic*, and therefore any actions within the destructor cannot be guaranteed to be executed at a reasonable time (unless other methods such as reference counting are used as well, however this defeats the purpose of garbage collection).

Typically types which might be garbage collected should not require a destructor, since most destructors are required for managing memory of the type's member variables, something that usually becomes unnecessary in the the presence of garbage collection (note that if a type's member variables have destructors, then the type itself will also have an auto-generated destructor which will call them, prohibiting its use with garbage collection).

However, as with all rules, there may be reasonable exceptions, so a *std::__new_force_gc* function is provided:

.. code-block:: c++

	class Type(int v) {
		static create = default;
		
		~ {
			//...
		}
	}
	
	void function(){
		// Fails to compile - 'Type' has destructor.
		Type * gcPtr = std::new_gc<Type>(Type(0));
		
		// Succeeds - but probably isn't wise, since 
		// the destructor will never be called.
		Type * gcPtr1 = std::__new_force_gc(Type(0));
	}


