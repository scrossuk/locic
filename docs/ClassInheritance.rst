Class Inheritance
=================

Loci doesn't support inheritance between classes, in which one class *B* inherits the member variables and methods from another class *A*, and usually instances of *B* can be cast to *A* (generally via a pointer or reference). Note that the latter functionality is polymorphism, and is supported by Loci's interfaces via :doc:`Structural Typing <StructuralTyping>`.

C++ supports both single inheritance and multiple inheritance, and even virtual inheritance for resolving the diamond problem. Most other languages, including Java, C# and D, support only single inheritance, which helps to alleviate the problems with multiple inheritance.

However Loci doesn't support inheritance as a matter of design, instead providing composition as superior method of code reuse. Inheritance has a number of flaws, including the fragile superclass problem, violating encapsulation, increasing coupling between classes and early binding of the implementation of the subclass to that of the superclass.

On the other hand, composition is a very powerful mechanism to allow one class to use the functionality of another without relying on the internal implementation of the superclass.

While inheritance (in particular tall class hierarchies) encourage monolothic design, a combination of composition and interfaces facilitates modular designs consisting of many small single-purpose classes. Furthermore, classes can modify their internal behaviour, while maintaining a stable API, without breaking code that uses them, which clearly goes a long way towards satisfying Loci's key design goals.

The decision not to include inheritance was not made based on whether developers might or might not misuse it (for example, Java decided to throw out :doc:`Operator Overloading <OperatorOverloading>` on this basis, which turned out to be harmful to those developers who understood how to use the feature correctly), but rather the impact it has on the language, both in terms of the semantics and the actual implementation.

Any feature added must be integrated into the language, so that it is defined how it interacts with other parts of the language, and may imply a significant amount of effort on the part of the implementor(s).


