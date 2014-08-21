Function Overloading
====================

**Note:** This feature is **completely different** from :doc:`Operator Overloading <OperatorOverloading>`, which is enthusiastically supported by Loci.

Function/method overloading is a common feature in other languages, such as C++ and Java (though not C). Loci was designed to intentionally avoid this feature and hence the problems that it creates:

* *Ambiguous function calls* - The compiler needs to take special care to call the correct function when multiple variants are available. In many cases this choice is obvious but in some cases there are multiple valid choices and, depending on the rules of the language, the compiler could choose incorrectly (or complain about the ambiguity).
* *Complex name mangling* - As soon as function overloading is added to a language the underlying :doc:`Name Mangling <NameMangling>` must be more complex to prevent symbol name collisions between these functions at link-time. This can affect C integration, which requires an extra compatibility layer (e.g. C++'s 'extern C').
* *Language Compatibility* - Naming multiple functions/methods with the same name is likely to inhibit compatibility with some environments/languages.
* *Name referencing* - A source analyser, a documentation tool or even a developer may wish to refer to a particular function or method. However this is much more difficult when this needs to be further qualified with the type of the parameters.
* *Poor interaction with implicit casts* - Implicit casts tend to be an inevitable feature in programming languages to reduce unnecessary verbosity by the developer. Unfortunately when combined with function/method overloading this exacerbates the issue of ambiguity due to multiple implicit cast paths from the function call leading to the different alternatives.
* *Pre-call function references* - With function overloading a reference to a function leads to an ambiguous value, which would present substantial difficulties with resolving this ambiguity.

For the last point, consider the following code:

.. code-block:: c++

	void function(int i) { }
	
	void function(double d) { }
	
	void example() {
		auto v = function;
		v(1);
		return;
	}

This code might look OK, but it's actually really problematic from the point of view of the language. The issue is that the type of variable *v* is unknown!

There are potential solutions, such as looking at the next line to determine the probable type of *v*, but this is likely to be extremely complex. Another (arguably better) alternative is to complain about this problem, but the process of deducing this and trying to help developers understand this problem would again be difficult and complex.

Ultimately it was decided that the problems above mean that function/method overloading is not a worthwhile addition to Loci. Instead it turns out the problems that function/method overloading is designed to solve can be better solved by a combination of :doc:`Templates <Templates>` and :doc:`Polymorphism <StructuralTyping>`.

