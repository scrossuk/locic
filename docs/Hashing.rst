Hashing
=======

.. Note::
	The following functionality has not yet been implemented in the compiler.

Loci supports hashing types, which is primarily useful for placing those types in hash tables.

Hashing functionality has been carefully implemented to split apart:

* Hashable types.
* Hashing algorithms ('hashers').
* Hash functions.

Hashable Type
-------------

A type ``T`` is hashable if it satisfies the requirement ``hashable<T, Hasher>``, where ``Hasher`` is a hashing algorithm. This simply means it needs to provide a ``hash()`` method with the following signature:

::

	void hash(Hasher& hasher) const noexcept;

The ``hasher`` argument here is a reference to a hashing algorithm, to which you should provide all the relevant state of your object. It is the job of the hashing algorithm (``Hasher``) to turn this state into a hash value.

Typically when a type implements ``hashable`` it will specify its own interface. For example:

::

	interface CustomTypeHasher {
		void hash_custom_type(const CustomType& value) noexcept;
	}
	
	class CustomType(float f, int i) {
		void hash(CustomTypeHasher& hasher) const noexcept {
			hasher.hash_custom_type(self);
		}
	}

This implementation requires the ``Hasher`` to have a special method ``hash_custom_type()`` for this type. If every type does this then hashing algorithms would need to have large numbers of ``hash_*()`` methods. Hence it's generally better to call the hasher on your member variables and to only require new ``hash_*`` methods for primitive types:

::

	interface CustomTypeHasher {
		void hash_float(float value) noexcept;
		void hash_int(int value) noexcept;
	}
	
	class CustomType(float f, int i) {
		void hash(CustomTypeHasher& hasher) const noexcept {
			@f.hash(hasher);
			@i.hash(hasher);
		}
	}

Your type will then be hashable for any hashing algorithm that supports ``float`` and ``int``.

Hashing algorithm
-----------------

Hashing algorithms (or 'hashers') implement the logic for generating a hash value from some given object state. Hashing algorithms need to implement ``hash_*()`` methods for all the relevant basic types they intend to support (usually this is just Loci's primitive types).

The standard hashing algorithm is called ``std::hasher`` and exists in the ``std.container`` module. You can also define custom hashing algorithms:

::

	class CustomHasher {
		void hash_float(float value) noexcept;
		void hash_int(int value) noexcept;
	}

This hasher supports both ``float`` and ``int`` because the ``float::hash()`` method will call ``hash_float()``, whereas the ``int::hash()`` method will call ``hash_int()``.

Typically the hasher will also need to have a method to return the hash value to the calling hash function:

::

	class CustomHasher {
		void hash_float(float value) noexcept;
		void hash_int(int value) noexcept;
		size_t get() const noexcept;
	}

Hash function
-------------

A hash function is essentially a convenient wrapper around a hashing algorithm, with a single ``call`` method that takes in a hashable type and returns the hash value.

The standard hash function is called ``std::hash<T>`` and exists in the ``std.container`` module. You can also define custom hash functions:

::

	template <typename T>
	require(hashable<T, CustomHasher>)
	class HashFunction() {
		static create = default;
		
		size_t call(const T& object) const noexcept {
			auto h = CustomHasher();
			object.hash(h);
			return h.get();
		}
	}

The hash function can then be used as follows:

::

	auto hashFn = HashFunction();
	auto hashOf42 = hashFn(42);
	auto hashOf42_0f = hashFn(42.0f);

The ``hashable<T, CustomHasher>`` requirement will expand out to checking that ``T`` has a ``hash`` method that can take a reference to ``CustomHasher``. So the hash function will work for all types supporting by the ``CustomHasher`` hashing algorithm.


