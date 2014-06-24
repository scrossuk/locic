#include <assert.h>

#include <memory>

#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/Type.hpp>

namespace llvm_abi {

	namespace {
	
		template<typename T, typename ...Args>
		std::unique_ptr<T> make_unique(Args&& ...args) {
			return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
		}
		
	}
	
	Type Type::Pointer() {
		Type type;
		type.kind_ = PointerType;
		return type;
	}
	
	Type Type::Integer(IntegerKind kind) {
		Type type;
		type.kind_ = IntegerType;
		type.subKind_.integerKind = kind;
		return type;
	}
	
	Type Type::FloatingPoint(FloatingPointKind kind) {
		Type type;
		type.kind_ = FloatingPointType;
		type.subKind_.floatingPointKind = kind;
		return type;
	}
	
	Type Type::Complex(FloatingPointKind kind) {
		Type type;
		type.kind_ = ComplexType;
		type.subKind_.complexKind = kind;
		return type;
	}
	
	Type Type::Struct(std::vector<StructMember> members) {
		Type type;
		type.kind_ = StructType;
		type.structType_.members = std::move(members);
		return type;
	}
	
	Type Type::AutoStruct(std::vector<Type> memberTypes) {
		Type type;
		type.kind_ = StructType;
		type.structType_.members.reserve(memberTypes.size());
		for (auto& memberType: memberTypes) {
			type.structType_.members.push_back(StructMember::AutoOffset(std::move(memberType)));
		}
		return type;
	}
	
	Type Type::Array(size_t elementCount, Type elementType) {
		Type type;
		type.kind_ = ArrayType;
		type.arrayType_.elementCount = elementCount;
		type.arrayType_.elementType = make_unique<Type>(std::move(elementType));
		return type;
	}
	
	Type::Type()
		: kind_(PointerType) { }
		
	Type::~Type() { }
	
	Type Type::copy() const {
		switch (kind()) {
			case PointerType:
				return Pointer();
			case IntegerType:
				return Integer(integerKind());
			case FloatingPointType:
				return FloatingPoint(floatingPointKind());
			case ComplexType:
				return Complex(complexKind());
			case StructType: {
				std::vector<StructMember> structMembersCopy;
				structMembersCopy.reserve(structMembers().size());
				
				for (const auto& member: structMembers()) {
					structMembersCopy.push_back(member.copy());
				}
				
				return Struct(std::move(structMembersCopy));
			}
			case ArrayType:
				return Array(arrayElementCount(), arrayElementType().copy());
			default:
				llvm_unreachable("Unknown ABI Type kind in copy().");
		}
	}
	
	TypeKind Type::kind() const {
		return kind_;
	}
	
	bool Type::isPointer() const {
		return kind_ == PointerType;
	}
	
	bool Type::isInteger() const {
		return kind_ == IntegerType;
	}
	
	IntegerKind Type::integerKind() const {
		assert(isInteger());
		return subKind_.integerKind;
	}
	
	bool Type::isFloatingPoint() const {
		return kind_ == FloatingPointType;
	}
	
	FloatingPointKind Type::floatingPointKind() const {
		assert(isFloatingPoint());
		return subKind_.floatingPointKind;
	}
	
	bool Type::isComplex() const {
		return kind_ == ComplexType;
	}
	
	FloatingPointKind Type::complexKind() const {
		assert(isComplex());
		return subKind_.complexKind;
	}
	
	bool Type::isStruct() const {
		return kind_ == StructType;
	}
	
	const std::vector<StructMember>& Type::structMembers() const {
		assert(isStruct());
		return structType_.members;
	}
	
	bool Type::isArray() const {
		return kind_ == ArrayType;
	}
	
	size_t Type::arrayElementCount() const {
		assert(isArray());
		return arrayType_.elementCount;
	}
	
	const Type& Type::arrayElementType() const {
		assert(isArray());
		return *(arrayType_.elementType);
	}
	
	static std::string intKindToString(IntegerKind kind) {
		switch (kind) {
			case Bool:
				return "Bool";
			case Char:
				return "Char";
			case Short:
				return "Short";
			case Int:
				return "Int";
			case Long:
				return "Long";
			case LongLong:
				return "LongLong";
			case Int8:
				return "Int8";
			case Int16:
				return "Int16";
			case Int32:
				return "Int32";
			case Int64:
				return "Int64";
			case Int128:
				return "Int128";
			case SizeT:
				return "SizeT";
			default:
				return "[UNKNOWN]";
		}
	}
	
	static std::string floatKindToString(FloatingPointKind kind) {
		switch (kind) {
			case Float:
				return "Float";
			case Double:
				return "Double";
			case LongDouble:
				return "LongDouble";
			case Float128:
				return "Float128";
			default:
				return "[UNKNOWN]";
		}
	}
	
	std::string Type::toString() const {
		switch (kind()) {
			case PointerType:
				return "Pointer";
			case IntegerType:
				return std::string("Integer(") + intKindToString(integerKind()) + ")";
			case FloatingPointType:
				return std::string("FloatingPoint(") + floatKindToString(floatingPointKind()) + ")";
			case ComplexType:
				return std::string("Complex(") + floatKindToString(complexKind()) + ")";
			case StructType: {
				std::string s = "Struct(";
				const auto& members = structMembers();
				for (size_t i = 0; i < members.size(); i++) {
					if (i > 0) {
						s += ", ";
					}
					s += std::string("StructMember(") + members.at(i).type().toString() + ")";
				}
				return s + ")";
			}
			case ArrayType:
				return std::string("Array(") + arrayElementType().toString() + ")";
			default:
				return "[UNKNOWN]";
		}
	}
	
}

