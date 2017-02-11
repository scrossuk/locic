Proposal: Casting Rules
=======================

.. Note::
	Feature awaiting further design consideration.

This is a proposal outlining potential rules for implicit casts.

Cast Operations
---------------

A cast operation is performed from one value type to another value type. A value type:

* Cannot be ``const``.
* Cannot be an interface.

Cast operations are implemented using ``cast`` methods:

.. code-block:: c++

	class Example {
		static int cast(Example value) noexcept;
	}

Satisfies
~~~~~~~~~

``A : B`` is a satisfies check, where ``A`` and ``B`` are two types. ``A : B`` can only be true if ``A`` contains all the methods required by ``B``.

However if ``B`` is **not** an interface then ``A`` and ``B`` must have the same type (ignoring template arguments), which prevents two different but API-equivalent types from being cast to one another.

Many casts will depend on the result of a satisfies check:

.. code-block:: c++

	template <typename T>
	class ref_t {
		template <typename S>
		static ref_t<S> cast(ref_t<T> value) noexcept
			require(T : S);
	}

Hence to cast from ``T&`` to ``S&`` it is necessary to check that ``T : S``.

Noop
~~~~

The ``noop`` tag indicates that a method doesn't require any run-time code. It would be used on ``cast`` methods to indicate that they are equivalent to omitting the call:

.. code-block:: c++

	template <typename T>
	class RefWrap(T& value) {
		template <typename S>
		static RefWrap<S> cast(RefWrap<T> value) noexcept noop = default;
	}

The ``noop`` property can only be achieved using ``= default`` and has method-specific meaning (e.g. it is different for destructors).

Cast Sequence
-------------

* Copy reference targets (``T&`` -> ``T``) until reference depths match.
* If ``Source = A&`` and ``Dest = B&``:
	* If ``A : B``: success!
	* Otherwise, copy source and continue.
* If ``Source = A`` and ``Dest = B&``:
	* If ``A : B``: success!
	* If ``B`` is an interface: failure!
	* Otherwise, ``Dest = B.stripConst``.
* If ``Source = A`` and ``Dest = B``:
	* If ``A`` has user implicit to ``B``: success!
	* If ``B`` is a variant and ``A`` is in the variant: success!
* Failure!

This pseudocode gives some more detail:

.. code-block:: c++
	
	bool implicitCast(Type source, Type dest, bool canBind) {
		if ((source.refDepth + canBind ? 1 : 0) < dest.refDepth) {
			// We can only bind zero or one times.
			return false;
		}
		
		// Keep removing references from source type until we reach
		// depth of destination type.
		while (source.refDepth > dest.refDepth) {
			if (!source.target.implicitCopyable) { return false; }
			source = source.target.implicitCopyType;
		}
		
		if (source.refDepth == dest.refDepth) {
			if (source.isRef) {
				return implicitCastRefToRef(source, dest, canBind);
			} else {
				return implicitCastValueToValue(source, dest);
			}
		} else {
			assert (source.refDepth + 1) == dest.refDepth;
			return implicitCastValueToRef(source, dest);
		}
	}
	
	bool implicitCastRefToRef(Type source, Type dest, bool canBind) {
		assert source.isRef && dest.isRef;
		assert source.refDepth == dest.refDepth;
		
		// Try a polymorphic reference cast.
		if (satisfies(source.target, dest.target)) {
			return true;
		}
		
		// Reference types aren't compatible so we can try to copy, cast
		// and then bind.
		if (!source.target.implicitCopyable || !canBind) {
			return false;
		}
		
		source = source.target.implicitCopyType;
		
		return implicitCastValueToRef(source, dest);
	}
	
	bool implicitCastValueToRef(Type source, Type dest) {
		assert dest.isRef;
		
		// Try to perform a bind operation; this will be needed if the
		// destination target type is an interface.
		if (implicitCastNoop(source, dest.target)) {
			return true;
		}
		
		// Try to cast the source type to the destination type without
		// the const tag; if this is successful we can then bind.
		return implicitCastValueToValue(source, dest.target.stripConst);
	}
	
	bool implicitCastValueToValue(Type source, Type dest) {
		assert !source.isConst && !dest.isConst && !source.isInterface;
		if (dest.isInterface) { return false; }
		
		if (implicitCastNoop(source, dest)) {
			return true;
		}
		
		// Either we perform a user implicitCast() call or we cast from
		// a variant type to its parent variant object.
		return source.hasUserImplicitCast(dest) ||
		       dest.hasVariantType(source);
	}
	
	bool implicitCastNoop(Type source, Type dest) {
		assert !source.isConst && !dest.isConst && !source.isInterface;
		return satisfies(source, dest);
	}
	
	bool satisfies(Type source, Type dest) {
		if (dest.isAuto) {
			// References can't match auto.
			if (source.isRef) { return false; }
			
			// Everything else does match auto.
			return true;
		}
		
		if (dest.isInterface) {
			return methodSet(source) >= methodSet(dest);
		}
		
		if (source.isObject != dest.isObject) {
			// Cannot cast template var -> object type or
			// vice versa.
			return false;
		}
		
		if (source.isObject) {
			if (!dest.isObject) {
				// Cannot cast object -> template var.
				return false;
			}
			
			if (source.objectType != dest.objectType) {
				// Cannot cast between different object
				// types.
				return false;
			}
		} else {
			assert source.isTemplate;
			
			if (source.templateVar != dest.templateVar) {
				// Cannot cast between different template
				// types.
				return false;
			}
		}
		
		if (source.isRef) {
			assert dest.isRef;
			return methodSet(source.target) >= methodSet(dest.target);
		}
		
		return methodSet(source) >= methodSet(dest);
	}

Noop Casts
----------

Noop casts are the most basic kind of cast, performed without any code executed at run-time:

.. code-block:: c++

	const<A> Type -> auto
	    where Type != OtherType&

	const<A> ObjectType<...> -> const<B> ObjectType<...>
	    where const<A> ObjectType<...> : const<B> ObjectType<...>

	const<A> T -> const<B> T
	    where const<A> T : const<B> T

Variant Parent Casts
--------------------

A type can be cast to a variant that contains it:

.. code-block:: c++

	Type -> VariantType
	    where Type in VariantType

Deref Reference Casts
---------------------

A reference-to-reference type can be cast to remove the outer reference:

.. code-block:: c++

	Type&& -> Type&

Polymorphic Reference Casts
---------------------------

.. code-block:: c++

	Type& -> Interface&

Polymorphic Static Reference Casts
----------------------------------

.. code-block:: c++

	staticref<Type> -> staticref<Interface>

Implicit Copy Ref
-----------------

.. code-block:: c++

	T& -> T

Implicit Copy Const
-------------------

.. code-block:: c++

	const T -> T

Bind
----

.. code-block:: c++

	T -> T&
