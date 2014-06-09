#ifndef LLVMABI_ABITYPE_HPP
#define LLVMABI_ABITYPE_HPP

#include <memory>
#include <string>
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
	
	// Forward declaration.
	class StructMember;
	
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
			 * \brief Struct Type
			 */
			static Type Struct(std::vector<StructMember> members);
			
			/**
			 * \brief Auto-aligned Struct Type
			 */
			static Type AutoStruct(std::vector<Type> memberTypes);
			
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
			
			bool isStruct() const;
			
			const std::vector<StructMember>& structMembers() const;
			
			bool isArray() const;
			
			size_t arrayElementCount() const;
			
			const Type& arrayElementType() const;
			
			std::string toString() const;
			
		private:
			Type(std::unique_ptr<struct TypeImpl>&&);
			
			std::unique_ptr<struct TypeImpl> impl_;
		
	};
	
	class StructMember {
		public:
			inline static StructMember AutoOffset(Type type) {
				return StructMember(std::move(type), 0);
			}
			
			inline static StructMember ForceOffset(Type type, size_t offset) {
				return StructMember(std::move(type), offset);
			}
			
			inline const Type& type() const {
				return type_;
			}
			
			inline Type& type() {
				return type_;
			}
			
			inline size_t offset() const {
				return offset_;
			}
			
		private:
			inline StructMember(Type&& pType, size_t pOffset)
				: type_(std::move(pType)), offset_(pOffset) { }
			
			Type type_;
			size_t offset_;
			
	};
	
}

#endif
