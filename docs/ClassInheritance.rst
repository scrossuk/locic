Class Inheritance
=================

.. Note::
	The following functionality has not yet been implemented in the compiler.

Loci has support for class implementation inheritance via the ``inherit`` keyword. Here's an example:

.. code-block:: c++

	class BaseClass {
		void method();
	}
	
	class TestClass(inherit BaseClass base) {
		[...]
	}

The ``inherit`` keyword is used on a class member variable to indicate that the parent class inherits all the methods of the member. This means that methods are added to ``TestClass`` which directly call the methods of ``BaseClass``.

Note that this is **entirely separate** from :doc:`polymorphism <StructuralTyping>` (also known as 'interface inheritance' or 'subtyping'); in this case it means that ``TestClass&`` **cannot** be cast to ``BaseClass&``, and that all methods of ``BaseClass`` remain **non-virtual** (i.e. calls to ``BaseClass`` never call into ``TestClass``). (See :ref:`Rationale <inheritance_rationale>`.)

Overriding Methods
------------------

If a derived class wishes to define a method that exists in a base class, it must use the ``override`` keyword:

.. code-block:: c++

	class BaseClass {
		void method();
		
		void otherMethod();
	}
	
	class TestClass(inherit BaseClass base) {
		void otherMethod() override {
			[...]
		}
	}

.. Note::
	If ``override`` is specified, but the method in the derived class isn't actually overriding anything in the base class, this is a compile-time error.

Deleting Methods
----------------

In some cases the parent class may not want to inherit all the methods from a base class, in which case it can 'delete' the methods it doesn't want:

.. code-block:: c++

	class BaseClass {
		void method();
		
		void otherMethod();
	}
	
	class TestClass(inherit BaseClass base) {
		otherMethod = delete;
		
		[...]
	}

Multiple Inheritance
--------------------

``inherit`` can be used on multiple members:

.. code-block:: c++

	class FirstClass {
		void firstMethod();
	}
	
	class SecondClass {
		void secondMethod();
	}
	
	class TestClass(inherit FirstClass first, inherit SecondClass second) {
		[...]
	}

In this case ``TestClass`` simply inherits the methods from both ``FirstClass`` and ``SecondClass``.

The programmer is responsible for using ``override`` and ``delete`` to resolve any ambiguities. For example:

.. code-block:: c++

	class FirstClass {
		void method();
	}
	
	class SecondClass {
		void method();
	}
	
	class TestClass(inherit FirstClass first, inherit SecondClass second) {
		void method() override {
			@first.method();
			@second.method();
		}
	}

Virtual Inheritance
-------------------

``inherit`` can achieve the same effect as virtual inheritance by simply having derived classes take a reference to the base class:

.. code-block:: c++

	class A {
		static A create();
		
		void method();
	}
	
	class B(inherit A& a) {
		static create(A& a) {
			return @(a);
		}
	}
	
	class C(inherit A& a) {
		static create(A& a) {
			return @(a);
		}
	}
	
	class D(inherit A a, inherit B b, inherit C c) {
		static create() {
			return @(a: A(), B(a), C(a));
		}
	}

This works because ``inherit`` supports calling through :doc:`references <References>`, so (as expected) ``B`` and ``C`` inherit their methods from ``A`` by calling through the reference.

.. _inheritance_rationale:

Rationale
---------

This mechanism of inheritance is considerably different from inheritance in other languages (such as C++), as indicated by the clear syntactic difference, because it only provides implementation inheritance and **not** subtyping. This is, in fact, its key strength.

The intention behind the ``inherit`` keyword is to provide a convenient way to represent that a parent object provides all the capabilities of the member object. However it achieves this without constraining the classes to a fixed hierarchy; ``TestClass`` could in future inherit from another class or directly implement all its methods without changing its externally-visible API. This is because the inheritance relationship is **invisible to external users**.

There are many problems avoided by this approach:

* **Fragile superclass problem** - This is a problem where a seemingly safe change to a base class can break a derived class. This is avoided in Loci because the base class cannot call up to the derived class, the derived class cannot access the member variables of the base class and by allowing subclasses to select which methods they expose in their API.
* **Diamond problem** - This is where a cast is ambiguous, because the inheritance tree forms a diamond. This can also lead to ambiguous method calls. This is avoided in Loci because ``inherit`` does not provide a way to cast from the parent class to the member class, it simply forwards the methods. Polymorphic casts should always use interfaces and such casts are always unambiguous. Ambiguous method calls are avoided by forcing subclasses to resolve these ambiguities when they inherit.
* **Tight coupling** - A common problem with inheritance is that classes are bound together, so they can't be separated later. This isn't a problem with Loci because the derived class only depends on the public API of the base class, and the base class has no knowledge of the derived class.
* **Brittle hierarchies** - Sometimes an inheritance hierarchy can be created and it is later discovered to be flawed/suboptimal, but cannot be changed. In Loci a class can modify its inheritance relationships invisibly to external users so this problem doesn't occur.
* **Complex memory layout** - Multiple inheritance in some languages can lead to complex memory layouts. This is again avoided in Loci because ``inherit`` has nothing to do with layout - it's purely a mechanism for forwarding method calls - so members would be placed in memory as usual. :doc:`Interfaces <StructuralTyping>` also avoid this problem by using a hash table for their vtable.

Developer Advice
----------------

While Loci's form of inheritance is much safer and easy to use than inheritance in other languages, it is still advised to **use it sparingly**.

In particular it is best if, as much as possible, classes carefully and deliberately specify the methods in their API, forwarding manually when required. This means that the API is well understood and methods are not unexpectedly exposed to the users of the class.

If you're looking for polymorphism with support for (implicit) upcasts, then you should be using interfaces.
