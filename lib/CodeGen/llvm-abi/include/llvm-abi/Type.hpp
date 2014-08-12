#ifndef LLVMABI_ABITYPE_HPP
#define LLVMABI_ABITYPE_HPP

#include <string>
#include <vector>

#include <llvm/ADT/ArrayRef.h>

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
		SizeT,
		PtrDiffT
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
	class Context;
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
			static Type* Pointer(Context& context);
			
			/**
			 * \brief Integer Type
			 */
			static Type* Integer(Context& context, IntegerKind kind);
			
			/**
			 * \brief Floating Point Type
			 */
			static Type* FloatingPoint(Context& context, FloatingPointKind kind);
			
			/**
			 * \brief Complex Type
			 */
			static Type* Complex(Context& context, FloatingPointKind kind);
			
			/**
			 * \brief Struct Type
			 */
			static Type* Struct(Context& context, std::vector<StructMember> members);
			
			/**
			 * \brief Auto-aligned Struct Type
			 */
			static Type* AutoStruct(Context& context, llvm::ArrayRef<Type*> memberTypes);
			
			/**
			 * \brief Array Type
			 */
			static Type* Array(Context& context, size_t elementCount, Type* elementType);
			
			bool operator<(const Type& type) const;
			
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
			
			Type* arrayElementType() const;
			
			std::string toString() const;
			
		private:
			Type(TypeKind kind);
			
			TypeKind kind_;
			
			union {
				IntegerKind integerKind;
				FloatingPointKind floatingPointKind;
				FloatingPointKind complexKind;
			} subKind_;
			
			struct {
				std::vector<StructMember> members;
			} structType_;
			
			struct {
				size_t elementCount;
				Type* elementType;
			} arrayType_;
			
	};
	
	class StructMember {
		public:
			inline static StructMember AutoOffset(Type* type) {
				return StructMember(type, 0);
			}
			
			inline static StructMember ForceOffset(Type* type, size_t offset) {
				return StructMember(type, offset);
			}
			
			inline Type* type() const {
				return type_;
			}
			
			inline size_t offset() const {
				return offset_;
			}
			
		private:
			inline StructMember(Type* pType, size_t pOffset)
				: type_(pType), offset_(pOffset) { }
			
			Type* type_;
			size_t offset_;
			
	};
	
}

#endif
