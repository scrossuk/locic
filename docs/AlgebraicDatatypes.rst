Algebraic Datatypes
===================

Functional languages such as ML and Haskell provide algebraic data types as a mechanism for holding information, which may itself be of a number of different types. The following is a clear example of such a case, implemented in Loci:

.. code-block:: c++

	datatype BinaryTree =
		Leaf(int v) |
		Node(BinaryTree& left, BinaryTree& right);

In this case, a binary tree element is either a Leaf which contains only an integer value, or a Node that contains two trees as its left and right children.

It is common to need to describe such types when handling lots of data (for example, building ASTs in a compiler), and also in general programming. Expressing similar structures in C or C++ requires the combination of enums, unions and structs, and furthermore unions and class types interact poorly in C++, as they do in Loci:

.. code-block:: c++

	// This is C++ code.
	
	// Invalid - compiler can't know whether
	// to call destructor for std::string.
	// In Loci the problem is exactly the
	// same, so objects cannot be in unions.
	union UnionType{
		int i;
		std::string s;
	};

Defining the structure isn't useful alone, and Loci expands on the existing 'switch' statement to allow accessing and handling the contents of a data type.

.. code-block:: c++

	int getLeftMostValue(const BinaryTree& tree) {
		switch (tree) {
			case Leaf (int v) {
				return v;
			}
			case Node (BinaryTree& left, _) {
				return getLeftMostValue(left);
			}
		}
	}

Note that the underscore character indicates an unused field of the data type.

This statement has some specific semantics:

* The sub-types (in this case, 'Leaf' and 'Node') in the switch statement must be the complete set of sub-types within the parent type ('BinaryTree'); if any types are missed, compilation fails. However unspecified sub-types can be handled using a 'default' entry.
* Case statements can capture L-values within the data types. In the example the value in the leaf is captured so it can be printed to the command line.
* No 'break' statement is required to exit the switch, since this is almost always the desired behaviour and therefore requiring it is tedious for the programmer (also, the behaviour is unintuitive for developers who are new to C-style languages). However, 'continue' can be used to provide the behaviour of moving to the next case if desired.

Pattern Matching
----------------

Algebraic datatypes are clearly a concept that has been adopted from functional languages such as ML and Haskell. Additionally, Loci supports extracting the members of these data types by binding them through pattern matching:

.. code-block:: c++

	datatype Pair(int l, int r);
	
	Pair f() {
		return Pair(1, 2);
	}
	
	void g() {
		Pair(a, b) = f();
		//...
	}

In this example, the types of binding variables 'a' and 'b' are not required, and are internally set to 'auto' by the compiler, so it will then infer them.

