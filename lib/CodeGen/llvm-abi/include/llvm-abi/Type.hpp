#ifndef LLVMABI_ABITYPE_HPP
#define LLVMABI_ABITYPE_HPP

#include <memory>
#include <vector>

namespace llvm_abi {

	/**
	 * \brief Integer Kind
	 * 
	 * A set of possible integer types.
	 */
	enum IntegerKind {
		Bool,
		Char,
		Short,
		Int,
		Long,
		LongLong,
		Int8,
		Int16,
		Int32,
		Int64,
		Int128,
		SizeT
	};
	
	/**
	 * \brief Floating Point Kind
	 * 
	 * A set of possible floating point types.
	 */
	enum FloatingPointKind {
		Float,
		Double,
		LongDouble,
		Float128
	};
	
	/**
	 * \brief Type Kind
	 * 
	 * A set of possible type kinds.
	 */
	enum TypeKind {
		PointerType,
		IntegerType,
		FloatingPointType,
		ComplexType,
		StructType,
		ArrayType
	};
	
	/**
	 * \brief ABI Type
	 * 
	 * A representation of a C type for the purpose
	 * of ABI-compliant code generation.
	 */
	class Type {
		public:
			/**
			 * \brief Integer Type
			 */
			static Type Pointer();
			
			/**
			 * \brief Integer Type
			 */
			static Type Integer(IntegerKind kind);
			
			/**
			 * \brief Floating Point Type
			 */
			static Type FloatingPoint(FloatingPointKind kind);
			
			/**
			 * \brief Complex Type
			 */
			static Type Complex(FloatingPointKind kind);
			
			/**
			 * \brief Packed Struct Type
			 * 
			 * The byte offet of each member can be specified
			 * for each member.
			 */
			static Type PackedStruct(std::vector<struct StructMember> members);
			
			/**
			 * \brief Padded Struct Type
			 * 
			 * The byte offsets of each member are automatically
			 * calculated by the ABI based on its alignment
			 * rules/preferences.
			 */
			static Type PaddedStruct(std::vector<Type> members);
			
			/**
			 * \brief Array Type
			 */
			static Type Array(size_t elementCount, Type elementType);
			
			Type();
			
			Type(Type&&);
			
			Type& operator=(Type);
			
			~Type();
			
			TypeKind kind() const;
			
			bool isPointer() const;
			
			bool isInteger() const;
			
			IntegerKind integerKind() const;
			
			bool isFloatingPoint() const;
			
			FloatingPointKind floatingPointKind() const;
			
			bool isComplex() const;
			
			FloatingPointKind complexKind() const;
						
			bool isPackedStruct() const;
			
			const std::vector<StructMember>& packedStructMembers() const;
			
			bool isPaddedStruct() const;
			
			const std::vector<Type>& paddedStructMembers() const;
			
			bool isArray() const;
			
			size_t arrayElementCount() const;
			
			const Type& arrayElementType() const;
			
		private:
			Type(std::unique_ptr<struct TypeImpl>&&);
			
			std::unique_ptr<struct TypeImpl> impl_;
		
	};
	
	struct StructMember {
		Type type;
		size_t offset;
		
		inline StructMember(Type pType, size_t pOffset)
			: type(std::move(pType)), offset(pOffset) { }
	};
	
}

#endif
